import { Config } from '../config';
import { MessageTag } from './message-tags';

export function handleMessage(tag: MessageTag, msg: string) {
  switch (tag) {
    case MessageTag.URL_RESPONSE:
      return handleUrlResponse(msg);
  }
}

function handleUrlResponse(url: string) {
  Config.localFrontendUrl = url;
}
