export type PeerConnectionEvent = {
  type: 'connect' | 'disconnect' | 'connecting' | 'timeout';
  time: number;
};
