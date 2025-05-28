import {
  JoinedSessionMessage,
  IncomingOfferMessage,
  IncomingAnswerMessage,
  IncomingCandidateMessage,
} from './signal-messages';

export type PublicSignalEvents =
  | 'open'
  | 'close'
  | 'error'
  | 'iceMessage'
  | 'unknownMessage';

export interface SignalEventMap {
  open: void;
  close: CloseEvent;
  error: Event;

  iceMessage:
    | JoinedSessionMessage
    | IncomingOfferMessage
    | IncomingAnswerMessage
    | IncomingCandidateMessage;

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  unknownMessage: any;
}
