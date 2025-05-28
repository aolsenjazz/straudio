import type { KnownIncomingMessage } from '../../service/signal-service/signal-messages';

export type MessageFromRemote = {
  [K in KnownIncomingMessage['type']]: {
    type: K;
    payload: Extract<KnownIncomingMessage, { type: K }>['payload'];
    timeSent: number;
    timeReceived: number;
  };
}[KnownIncomingMessage['type']];
