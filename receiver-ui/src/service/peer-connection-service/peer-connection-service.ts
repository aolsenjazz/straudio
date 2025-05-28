import { signalService } from '../signal-service/signal-service';
import { createLogger } from '../../logger';

const logger = createLogger({ moduleName: 'PeerConnectionService' });

import type {
  IncomingOfferMessage,
  IncomingAnswerMessage,
  IncomingCandidateMessage,
} from '../signal-service/signal-messages';

const iceServers = [
  { urls: 'stun:stun.l.google.com:19302' },
  { urls: 'stun:stun.l.google.com:5349' },
  { urls: 'stun:stun1.l.google.com:3478' },
  { urls: 'stun:stun1.l.google.com:5349' },
  { urls: 'stun:stun2.l.google.com:19302' },
  { urls: 'stun:stun2.l.google.com:5349' },
  { urls: 'stun:stun3.l.google.com:3478' },
  { urls: 'stun:stun3.l.google.com:5349' },
  { urls: 'stun:stun4.l.google.com:19302' },
  { urls: 'stun:stun4.l.google.com:5349' },
];

type ConnectionStateListener = (
  clientId: string,
  state: RTCPeerConnectionState
) => void;

class PeerConnectionService {
  private connections = new Map<string, RTCPeerConnection>();
  private connectionStateListeners = new Set<ConnectionStateListener>();

  public addConnectionStateListener(listener: ConnectionStateListener) {
    this.connectionStateListeners.add(listener);
  }

  public removeConnectionStateListener(listener: ConnectionStateListener) {
    this.connectionStateListeners.delete(listener);
  }

  private notifyConnectionStateChange(
    clientId: string,
    state: RTCPeerConnectionState
  ) {
    this.connectionStateListeners.forEach((listener) => {
      try {
        listener(clientId, state);
      } catch (err) {
        console.error('Error in connection state listener:', err);
      }
    });
  }

  public async initiateOffer(clientId: string): Promise<void> {
    if (this.connections.get(clientId)) return;

    const pc = this.createPeerConnection(clientId);
    pc.createDataChannel('testDataChannel');

    try {
      const offer = await pc.createOffer();
      await pc.setLocalDescription(offer);
      signalService.send('offer', {
        targetClientId: clientId,
        description: offer.sdp,
      });
    } catch (err) {
      logger.fatal(
        `Error creating offer for clientId=${clientId}. Error: ${err}`
      );
    }
  }

  public async handleOffer(msg: IncomingOfferMessage) {
    const { description, sourceClientId } = msg.payload;

    if (!description) {
      logger.fatal('Offer missing description');
      return;
    }

    const pc = this.getPeerConnection(sourceClientId);
    await pc.setRemoteDescription({ type: 'offer', sdp: description });

    const answer = await pc.createAnswer();
    await pc.setLocalDescription(answer);

    signalService.send('answer', {
      targetClientId: sourceClientId,
      description: answer.sdp,
    });
  }

  public async handleAnswer(msg: IncomingAnswerMessage) {
    const { sourceClientId, description } = msg.payload;
    if (!sourceClientId || !description) {
      logger.fatal('Answer missing sourceClientId or description');
      return;
    }

    const pc = this.getPeerConnection(sourceClientId);
    await pc.setRemoteDescription({ type: 'answer', sdp: description });
  }

  public async handleCandidate(msg: IncomingCandidateMessage) {
    const { sourceClientId, candidate, mid } = msg.payload;
    if (!sourceClientId || !candidate || !mid) {
      logger.error('Candidate missing sourceClientId, candidate, or mid');
      return;
    }

    const pc = this.getPeerConnection(sourceClientId);
    await pc.addIceCandidate({ candidate, sdpMid: mid });
  }

  public closeAllConnections() {
    for (const [clientId, pc] of this.connections.entries()) {
      logger.info(`Closing peer connection for clientId=${clientId}`);
      pc.close();
    }
    this.connections.clear();
  }

  private createPeerConnection(clientId: string): RTCPeerConnection {
    const pc = new RTCPeerConnection({
      iceServers,
    });

    pc.onicecandidate = (event) => {
      if (event.candidate) {
        signalService.send('candidate', {
          targetClientId: clientId,
          candidate: event.candidate.candidate,
          mid: event.candidate.sdpMid,
        });
      }
    };

    pc.onconnectionstatechange = () => {
      const state = pc.connectionState;
      this.notifyConnectionStateChange(clientId, state);
    };

    this.connections.set(clientId, pc);
    return pc;
  }

  private getPeerConnection(clientId: string) {
    const pc = this.connections.get(clientId);
    if (!pc) {
      throw new Error(`PeerConnection[${clientId}] does not exist!`);
    }
    return pc;
  }
}

export const peerConnectionService = new PeerConnectionService();
