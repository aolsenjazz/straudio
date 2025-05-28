import { createSlice } from '@reduxjs/toolkit';

import { connectionStatusChanged } from '../../features/signal-socket/signal-socket-slice';
import { PeerConnectionInfo } from './peer-connection-info';
import { PeerConnectionEvent } from './peer-connection-event';

import type { PayloadAction } from '@reduxjs/toolkit';

import './peer-connection-listener';

type PeerConnectionState = {
  connections: Record<string, PeerConnectionInfo>;
};

const initialState: PeerConnectionState = {
  connections: {},
};

const peerConnectionSlice = createSlice({
  name: 'peerConnections',
  initialState,
  reducers: {
    peerConnectionAdded: (
      state,
      action: PayloadAction<
        Omit<PeerConnectionInfo, 'status' | 'connectionEvents'>
      >
    ) => {
      const { clientId } = action.payload;
      const connectionState = {
        clientId,
        status: 'idle' as const,
        connectionEvents: [] as PeerConnectionEvent[],
      };

      state.connections[clientId] = connectionState;
    },
    peerConnectionStatusChanged: (
      state,
      action: PayloadAction<{ clientId: string; event: PeerConnectionEvent }>
    ) => {
      const { clientId, event } = action.payload;
      const connection = state.connections[clientId];

      connection.connectionEvents.push(event);
      if (connection.connectionEvents.length > 100) {
        connection.connectionEvents.shift();
      }

      const status =
        action.payload.event.type === 'connect'
          ? 'connected'
          : action.payload.event.type === 'disconnect'
            ? 'idle'
            : action.payload.event.type === 'connecting'
              ? 'connecting'
              : 'idle';
      connection.status = status;

      if (action.payload.event.type === 'disconnect') {
        delete state.connections[clientId];
      }
    },
  },
  extraReducers: (builder) =>
    builder.addCase(connectionStatusChanged, (state, action) => {
      if (action.payload.type === 'disconnect') {
        state.connections = {};
      }
    }),
});

export const { peerConnectionAdded, peerConnectionStatusChanged } =
  peerConnectionSlice.actions;

export default peerConnectionSlice.reducer;
