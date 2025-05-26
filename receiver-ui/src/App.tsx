import { useEffect } from 'react';
import { createLogger } from './logger';
import { useAppDispatch, useAppSelector } from './app/hooks';
import {
  signalSocketConnect,
  signalSocketInitialize,
  signalSocketSend,
} from './features/signal-socket/signal-socket-listener';
import { peerConnectionInitialize } from './features/peer-connection/peer-connection-listener';
import { ConnectionIndicator } from './components/ConnectionIndicator';

const logger = createLogger({ moduleName: 'App' });

export default function App() {
  const urlParams = new URLSearchParams(window.location.search);
  const endpoint = urlParams.get('u'); // e.g. ?u=ws://localhost:8080/ws
  const sessionId = urlParams.get('s'); // e.g. ?s=12345

  const dispatch = useAppDispatch();
  const socketStatus = useAppSelector((state) => state.signalSocket.status);

  useEffect(() => {
    dispatch(signalSocketInitialize());
    dispatch(peerConnectionInitialize());
  }, []);

  useEffect(() => {
    if (!endpoint) {
      logger.error('No "u" param found in URL, cannot connect');
      return;
    }

    dispatch(
      signalSocketConnect({
        url: endpoint,
      })
    );
  }, [dispatch, endpoint]);

  useEffect(() => {
    if (!sessionId) {
      logger.error('No "s" (sessionId) param found in URL');
      return;
    }

    if (socketStatus === 'connected') {
      dispatch(
        signalSocketSend({
          type: 'joinSession',
          payload: {
            sessionId,
          },
        })
      );
    }
  }, [socketStatus, dispatch]);

  return (
    <div style={{ color: 'blue' }}>
      <h1>WebRTC Guest Client</h1>
      <ConnectionIndicator />
    </div>
  );
}
