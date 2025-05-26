import { createLogger } from '../../logger';
import {
  KnownIncomingMessage,
  JoinedSessionMessage,
  IncomingOfferMessage,
  IncomingAnswerMessage,
  IncomingCandidateMessage,
  PingMessage,
  PongMessage,
} from './signal-messages';
import { PublicSignalEvents, SignalEventMap } from './signal-events';
import { isKnownIncomingMessage } from './is-known-incoming-message';

const logger = createLogger({ moduleName: 'SignalService' });

type Listener<T> = (payload: T) => void;

export class SignalService {
  private static instance: SignalService;
  private ws: WebSocket | null = null;
  private connectionTimeoutId: number | null = null;
  private pingIntervalId: number | null = null;
  private pingTimeoutId: number | null = null; // timeout for pong response

  private defaultConnectTimeoutMs = 5000;
  private pingTimeoutMs = 3000; // 3 seconds to wait for pong

  private listeners: {
    [K in PublicSignalEvents]?: Set<Listener<SignalEventMap[K]>>;
  } = {};

  private constructor() {
    // no-op
  }

  public static getInstance() {
    if (!SignalService.instance) {
      SignalService.instance = new SignalService();
    }
    return SignalService.instance;
  }

  public connect(url: string) {
    if (
      this.ws &&
      (this.ws.readyState === WebSocket.OPEN ||
        this.ws.readyState === WebSocket.CONNECTING)
    ) {
      logger.warn('WebSocket is already open/connecting; ignoring connect()');
      return;
    }

    logger.info(`Connecting to WebSocket at: ${url}`);
    this.ws = new WebSocket(url);

    this.startConnectionTimer();

    this.ws.onopen = () => {
      logger.info('WebSocket opened');
      this.clearConnectionTimeout();
      this.emit('open', undefined);

      // Start the client ping loop:
      this.startClientPingLoop();
    };

    this.ws.onerror = (event) => {
      logger.error(`WebSocket error: ${JSON.stringify(event)}`);
      this.clearConnectionTimeout();
      this.emit('error', event);
    };

    this.ws.onclose = (event) => {
      logger.info(
        `WebSocket closed: code=${event.code}, reason=${event.reason}`
      );
      this.stopClientPingLoop();
      this.emit('close', event);
      this.ws = null;
    };

    this.ws.onmessage = (msgEvent) => {
      logger.verbose(`WebSocket message: ${msgEvent.data}`);
      this.handleIncomingMessage(msgEvent.data);
    };
  }

  public send(
    type: string,
    data: Record<string, unknown> = {},
    hasPayload = true
  ) {
    if (!this.isOpen()) {
      logger.error('Cannot send message: WebSocket is not open');
      return;
    }

    const msg: Record<string, unknown> = {
      type,
      timeSent: Date.now(),
    };

    if (hasPayload) {
      msg.payload = data;
    } else {
      Object.assign(msg, data);
    }

    const msgStr = JSON.stringify(msg);
    this.ws!.send(msgStr);
    logger.verbose(`Sent: ${msgStr}`);
  }

  public disconnect(code = 1000, reason?: string) {
    if (this.ws) {
      logger.info(
        `Disconnecting WebSocket with code=${code}, reason=${reason}`
      );
      this.stopClientPingLoop();
      this.ws.close(code, reason);
    }
  }

  private isOpen() {
    return this.ws && this.ws.readyState === WebSocket.OPEN;
  }

  private startConnectionTimer() {
    this.connectionTimeoutId = window.setTimeout(() => {
      if (this.ws && this.ws.readyState === WebSocket.CONNECTING) {
        logger.warn(
          `WebSocket connection timed out after ${this.defaultConnectTimeoutMs}ms`
        );
        this.ws.close(4001, 'Connection timeout');
        this.emit('error', new Event('timeout'));
      }
      this.connectionTimeoutId = null;
    }, this.defaultConnectTimeoutMs);
  }

  private clearConnectionTimeout() {
    if (this.connectionTimeoutId !== null) {
      clearTimeout(this.connectionTimeoutId);
      this.connectionTimeoutId = null;
    }
  }

  private handleIncomingMessage(rawData: unknown) {
    let parsed: unknown;
    if (typeof rawData === 'string') {
      try {
        parsed = JSON.parse(rawData);
      } catch {
        logger.warn(`Non-JSON message received: ${rawData}`);
        this.emit('unknownMessage', rawData);
        return;
      }
    } else {
      parsed = rawData;
    }

    if (!isKnownIncomingMessage(parsed)) {
      logger.warn(`Unknown or malformed message: ${JSON.stringify(parsed)}`);
      this.emit('unknownMessage', parsed);
      return;
    }

    this.routeParsedMessage(parsed);
  }

  private routeParsedMessage(msg: KnownIncomingMessage) {
    switch (msg.type) {
      case 'joinedSession':
        this.handleJoinedSession(msg);
        break;
      case 'offer':
        this.handleOffer(msg);
        break;
      case 'answer':
        this.handleAnswer(msg);
        break;
      case 'candidate':
        this.handleCandidate(msg);
        break;
      case 'ping':
        this.handlePing(msg);
        break;
      case 'pong':
        this.handlePong(msg);
        break;
      default:
        throw new Error('This should be unreachable!');
    }
  }

  private handleJoinedSession(msg: JoinedSessionMessage) {
    const { sessionId, hostId, role } = msg.payload;
    logger.info(
      `JoinedSession: sessionId=${sessionId}, hostId=${hostId}, role=${role}`
    );
    this.emit('iceMessage', msg);
  }

  private handleOffer(msg: IncomingOfferMessage) {
    const { sourceClientId, description } = msg.payload;
    if (!sourceClientId || !description) {
      logger.error('offer is missing sourceClientId or description');
      return;
    }
    this.emit('iceMessage', msg);
  }

  private handleAnswer(msg: IncomingAnswerMessage) {
    const { sourceClientId, description } = msg.payload;
    if (!sourceClientId || !description) {
      logger.error('answer is missing sourceClientId or description');
      return;
    }
    this.emit('iceMessage', msg);
  }

  private handleCandidate(msg: IncomingCandidateMessage) {
    const { sourceClientId, candidate, mid } = msg.payload;
    if (!sourceClientId || !candidate || !mid) {
      logger.error('candidate is missing sourceClientId, candidate, or mid');
      return;
    }
    this.emit('iceMessage', msg);
  }

  private handlePing(msg: PingMessage) {
    logger.verbose(`Received ping at ${msg.timeSent}; sending pong`);
    // Respond to the ping
    this.send('pong', { timestamp: Date.now() }, false);
    // Clear any ping timeout; receiving a ping implies liveness.
    this.clearPingTimeout();
  }

  private handlePong(msg: PongMessage) {
    logger.verbose(`Received pong at ${msg.timeSent}`);
    // Clear any ping timeout because pong arrived.
    this.clearPingTimeout();
  }

  private startClientPingLoop() {
    if (this.pingIntervalId) return;
    this.pingIntervalId = window.setInterval(() => {
      if (this.isOpen()) {
        // If a previous ping timeout is still active, that means no pong was received.
        if (this.pingTimeoutId !== null) {
          logger.error('Ping timeout: no pong received. Disconnecting.');
          this.disconnect(4000, 'Ping timeout');
          return;
        }
        this.send('ping', { timestamp: Date.now() }, false);

        this.pingTimeoutId = window.setTimeout(() => {
          logger.error(
            'Ping timeout (setTimeout fired): no pong received. Disconnecting.'
          );
          this.disconnect(4000, 'Ping timeout');
        }, this.pingTimeoutMs);
      }
    }, 5000);
  }

  private stopClientPingLoop() {
    if (this.pingIntervalId !== null) {
      clearInterval(this.pingIntervalId);
      this.pingIntervalId = null;
    }
    this.clearPingTimeout();
  }

  private clearPingTimeout() {
    if (this.pingTimeoutId !== null) {
      clearTimeout(this.pingTimeoutId);
      this.pingTimeoutId = null;
    }
  }

  public addListener<K extends PublicSignalEvents>(
    event: K,
    listener: Listener<SignalEventMap[K]>
  ) {
    if (!this.listeners[event]) {
      this.listeners[event] = new Set();
    }
    this.listeners[event]!.add(listener);
  }

  public removeListener<K extends PublicSignalEvents>(
    event: K,
    listener: Listener<SignalEventMap[K]>
  ) {
    this.listeners[event]?.delete(listener);
  }

  private emit<K extends PublicSignalEvents>(
    event: K,
    payload: SignalEventMap[K]
  ) {
    this.listeners[event]?.forEach((listener) => {
      listener(payload);
    });
  }
}

export const signalService = SignalService.getInstance();
