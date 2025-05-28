export type ConnectionEvent = {
  type: 'connect' | 'disconnect' | 'connecting' | 'timeout';
  time: number;
};
