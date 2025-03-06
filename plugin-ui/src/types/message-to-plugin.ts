import { MessageTag } from 'ipc/message-tags';

export type MessageToPlugin<T> = {
  tag: MessageTag;
  data?: T;
};
