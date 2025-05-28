import { createSelector, createSlice } from '@reduxjs/toolkit';

import type { PayloadAction } from '@reduxjs/toolkit';
import type { ConnectionEvent } from './connection-event';
import type { MessageFromRemote } from './message-from-remote';
import { MessageFromUs } from './message-from-us';

import './signal-socket-listener';

type SignalSocketState = {
  status: 'idle' | 'connecting' | 'connected';
  connectionEvents: ConnectionEvent[];
  receivedMessages: MessageFromRemote[];
  sentMessages: MessageFromUs[];
  url?: string;
};

const initialState: SignalSocketState = {
  status: 'idle',
  receivedMessages: [],
  sentMessages: [],
  connectionEvents: [],
};

const signalSocketSlice = createSlice({
  name: 'signalSocket',
  initialState,
  reducers: {
    urlUpdated: (state, action: PayloadAction<string>) => {
      state.url = action.payload;
    },
    connectionStatusChanged: (
      state,
      action: PayloadAction<ConnectionEvent>
    ) => {
      state.connectionEvents.push(action.payload);

      if (state.connectionEvents.length > 100) {
        state.connectionEvents.shift();
      }

      const status =
        action.payload.type === 'connect'
          ? 'connected'
          : action.payload.type === 'disconnect'
            ? 'idle'
            : action.payload.type === 'connecting'
              ? 'connecting'
              : 'idle';
      state.status = status;
    },
    messageReceived: (state, action: PayloadAction<MessageFromRemote>) => {
      state.receivedMessages.push(action.payload);

      if (state.receivedMessages.length > 100) {
        state.receivedMessages.shift();
      }
    },
    messageSent: (state, action: PayloadAction<MessageFromUs>) => {
      state.sentMessages.push(action.payload);

      if (state.sentMessages.length > 100) {
        state.sentMessages.shift();
      }
    },
  },
  selectors: {
    selectConnectionEvents: (state) => state.connectionEvents,
    selectReceivedMessages: (state) => state.receivedMessages,
    selectSentMessages: (state) => state.sentMessages,
  },
});

export const {
  urlUpdated,
  connectionStatusChanged,
  messageReceived,
  messageSent,
} = signalSocketSlice.actions;

export const selectLastConnectionEvent = createSelector(
  [signalSocketSlice.selectors.selectConnectionEvents],
  (events) => {
    return events[events.length - 1];
  }
);

export const selectLastReceivedMessage = createSelector(
  [signalSocketSlice.selectors.selectReceivedMessages],
  (messages) => messages[messages.length - 1]
);

export const selectLastSentMessage = createSelector(
  [signalSocketSlice.selectors.selectSentMessages],
  (messages) => messages[messages.length - 1]
);

export default signalSocketSlice.reducer;
