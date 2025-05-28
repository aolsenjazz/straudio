import type { PeerConnectionEvent } from './peer-connection-event';

type PeerConnectionStatus = 'idle' | 'connecting' | 'connected';

export type PeerConnectionInfo = {
  clientId: string;
  status: PeerConnectionStatus;
  connectionEvents: PeerConnectionEvent[];
};
