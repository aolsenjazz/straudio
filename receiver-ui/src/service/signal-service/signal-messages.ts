export interface JoinedSessionMessage {
  type: 'joinedSession';
  timeSent: number;
  payload: {
    sessionId: string;
    hostId: string;
    role: 'guest';
  };
}

export interface OutgoingOfferMessage {
  type: 'offer';
  timeSent: number;
  payload: {
    targetClientId: string;
    description: string;
  };
}

export interface IncomingOfferMessage {
  type: 'offer';
  timeSent: number;
  payload: {
    sourceClientId: string;
    description: string;
  };
}

export interface OutgoingAnswerMessage {
  type: 'answer';
  timeSent: number;
  payload: {
    targetClientId: string;
    description: string;
  };
}

export interface IncomingAnswerMessage {
  type: 'answer';
  timeSent: number;
  payload: {
    sourceClientId: string;
    description: string;
  };
}

export interface OutgoingCandidateMessage {
  type: 'candidate';
  timeSent: number;
  payload: {
    targetClientId: string;
    candidate: string;
    mid: string;
  };
}

export interface IncomingCandidateMessage {
  type: 'candidate';
  timeSent: number;
  payload: {
    sourceClientId: string;
    candidate: string;
    mid: string;
  };
}

export interface PingMessage {
  type: 'ping';
  payload: undefined;
  timeSent: number;
}

export interface PongMessage {
  type: 'pong';
  payload: undefined;
  timeSent: number;
}

export type KnownIncomingMessage =
  | JoinedSessionMessage
  | IncomingOfferMessage
  | IncomingAnswerMessage
  | IncomingCandidateMessage
  | PingMessage
  | PongMessage;
