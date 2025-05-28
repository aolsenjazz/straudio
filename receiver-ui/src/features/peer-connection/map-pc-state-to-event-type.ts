export function mapPcStateToEventType(
  state: RTCPeerConnectionState
): 'connect' | 'disconnect' | 'connecting' {
  switch (state) {
    case 'new':
    case 'connecting':
      return 'connecting';
    case 'connected':
      return 'connect';
    case 'disconnected':
    case 'failed':
    case 'closed':
      return 'disconnect';
    default:
      return 'disconnect';
  }
}
