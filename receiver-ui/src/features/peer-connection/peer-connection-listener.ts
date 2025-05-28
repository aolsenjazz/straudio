import { createAction } from '@reduxjs/toolkit';
import { addAppListener } from '../../app/listener-middleware';
import {
  connectionStatusChanged,
  messageReceived,
} from '../signal-socket/signal-socket-slice';
import { peerConnectionService } from '../../service/peer-connection-service/peer-connection-service';
import {
  peerConnectionAdded,
  peerConnectionStatusChanged,
} from './peer-connection-slice';
import { mapPcStateToEventType } from './map-pc-state-to-event-type';

export const peerConnectionInitialize = createAction(
  'peerConnection/initialize'
);

let isInitialized = false;

addAppListener({
  actionCreator: connectionStatusChanged,
  effect: (action) => {
    if (action.payload.type === 'disconnect') {
      peerConnectionService.closeAllConnections();
    }
  },
});

addAppListener({
  actionCreator: peerConnectionInitialize,
  effect: (_action, listenerApi) => {
    if (isInitialized) return;
    isInitialized = true;

    peerConnectionService.addConnectionStateListener((clientId, state) => {
      listenerApi.dispatch(
        peerConnectionStatusChanged({
          clientId,
          event: {
            type: mapPcStateToEventType(state),
            time: Date.now(),
          },
        })
      );
    });
  },
});

addAppListener({
  actionCreator: messageReceived,
  effect: (action, listenerApi) => {
    const { payload: message } = action;

    switch (message.type) {
      case 'joinedSession':
        peerConnectionService.initiateOffer(message.payload.hostId);
        listenerApi.dispatch(
          peerConnectionAdded({
            clientId: message.payload.hostId,
          })
        );
        break;

      case 'offer':
        peerConnectionService.handleOffer(message);
        break;

      case 'answer':
        peerConnectionService.handleAnswer(message);
        break;

      case 'candidate':
        peerConnectionService.handleCandidate(message);
        break;
    }
  },
});
