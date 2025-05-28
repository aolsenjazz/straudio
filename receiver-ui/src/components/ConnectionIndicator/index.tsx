import { useAppSelector, useAppDispatch } from '../../app/hooks';
import { selectLastConnectionEvent } from '../../features/signal-socket/signal-socket-slice';
import './ConnectionIndicator.css';
import { signalSocketConnect } from '../../features/signal-socket/signal-socket-listener';

export function ConnectionIndicator() {
  const dispatch = useAppDispatch();

  const signalStatus = useAppSelector((state) => state.signalSocket.status);
  const lastSignalMsg = useAppSelector(selectLastConnectionEvent);

  const signalUrl = useAppSelector((state) => state.signalSocket.url);
  const peerConnections = useAppSelector(
    (state) => state.peerConnection.connections
  );
  const peerStatus = Object.values(peerConnections)[0]?.status;

  const isSignalConnecting = signalStatus !== 'connected';
  const isPeerConnecting = peerStatus !== 'connected';

  const isLoading = isSignalConnecting || isPeerConnecting;
  const isConnected = !isSignalConnecting && !isPeerConnecting;
  const isDisconnected = lastSignalMsg?.type === 'disconnect';

  const handleReconnect = () => {
    if (signalUrl) {
      dispatch(signalSocketConnect({ url: signalUrl }));
    }
  };

  return (
    <div className="connection-indicator">
      {isDisconnected ? (
        <>
          <span className="disconnected">Disconnected</span>
          <button className="reconnect-button" onClick={handleReconnect}>
            Reconnect
          </button>
        </>
      ) : isLoading ? (
        <>
          <div className="spinner" />
          <span>Connecting...</span>
        </>
      ) : isConnected ? (
        <span className="connected">Connected</span>
      ) : null}
    </div>
  );
}
