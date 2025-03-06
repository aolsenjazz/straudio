import { MessageTag } from './ipc/message-tags';
import { useEffect } from 'react';

export default function App() {
  useEffect(() => {
    IPlugSendMsg({ tag: MessageTag.URL_REQUEST });
  }, []);

  return <div>Hello world!</div>;
}
