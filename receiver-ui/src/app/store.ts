import { configureStore } from '@reduxjs/toolkit';
import { listenerMiddleware } from './listener-middleware';
import SignalSocketReducer from '../features/signal-socket/signal-socket-slice';
import PeerConnectionReducer from '../features/peer-connection/peer-connection-slice';

import type { Action, ThunkAction } from '@reduxjs/toolkit';

export const store = configureStore({
  reducer: {
    signalSocket: SignalSocketReducer,
    peerConnection: PeerConnectionReducer,
  },
  middleware: (getDefaultMiddleware) =>
    getDefaultMiddleware().concat(listenerMiddleware.middleware),
});

// The standard RTK boilerplate
export type AppStore = typeof store;
export type RootState = ReturnType<AppStore['getState']>;
export type AppDispatch = AppStore['dispatch'];
export type AppThunk<ThunkReturnType = void> = ThunkAction<
  ThunkReturnType,
  RootState,
  unknown,
  Action
>;
