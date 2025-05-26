export type MessageFromUs = {
  type: 'joinSession';
  payload: {
    sessionId: string;
  };
};
