import { createAction } from '@reduxjs/toolkit';
import { addAppListener } from '../../app/listener-middleware';
import { signalService } from '../../service/signal-service/signal-service';
import {
  connectionStatusChanged,
  messageReceived,
  messageSent,
  urlUpdated,
} from './signal-socket-slice';

type ConnectPayload = {
  url: string;
};
type SendPayload = {
  type: 'joinSession';
  payload: {
    sessionId: string;
  };
};

export const signalSocketInitialize = createAction('signalSocket/initialize');
export const signalSocketDisconnect = createAction('signalSocket/disconnect');
export const signalSocketSend = createAction<SendPayload>('signalSocket/send');
export const signalSocketConnect = createAction<ConnectPayload>(
  'signalSocket/connect'
);

/**
 * React <StrictMode /> is a charm and so we have to use a flag to prevent
 * double-initialization in localhost
 */
let isInitialized = false;

addAppListener({
  actionCreator: signalSocketInitialize,
  effect: (_action, listenerApi) => {
    if (isInitialized) return;
    isInitialized = true;

    signalService.addListener('open', () => {
      listenerApi.dispatch(
        connectionStatusChanged({ type: 'connect', time: Date.now() })
      );
    });

    signalService.addListener('close', () => {
      listenerApi.dispatch(
        connectionStatusChanged({ type: 'disconnect', time: Date.now() })
      );
    });

    signalService.addListener('error', () => {
      // TODO
    });

    signalService.addListener('iceMessage', (msg) => {
      const receivedMsg = {
        ...msg,
        timeReceived: Date.now(),
      };
      listenerApi.dispatch(messageReceived(receivedMsg));
    });
  },
});

addAppListener({
  actionCreator: signalSocketConnect,
  effect: (action, listenerApi) => {
    const { status } = listenerApi.getState().signalSocket;
    if (status === 'idle') {
      listenerApi.dispatch(urlUpdated(action.payload.url));
      listenerApi.dispatch(
        connectionStatusChanged({ type: 'connecting', time: Date.now() })
      );
      signalService.connect(action.payload.url);
    }
  },
});

addAppListener({
  actionCreator: signalSocketSend,
  effect: (action, listenerApi) => {
    signalService.send(action.payload.type, action.payload.payload);

    const sentMsg = {
      type: action.payload.type,
      payload: action.payload.payload,
      timeSent: Date.now(),
    };

    listenerApi.dispatch(messageSent(sentMsg));
  },
});

addAppListener({
  actionCreator: signalSocketDisconnect,
  effect: (_action, listenerApi) => {
    const { status } = listenerApi.getState().signalSocket;
    if (status === 'connected') {
      signalService.disconnect();
      listenerApi.dispatch(
        connectionStatusChanged({ type: 'disconnect', time: Date.now() })
      );
    }
  },
});
