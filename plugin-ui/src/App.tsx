import { useEffect } from 'react';
import QRCode from 'react-qr-code';

import { MessageTag } from './ipc/message-tags';
import { useLocalFrontendUrl } from './hooks/use-local-frontend-url';

export default function App() {
  const frontendUrl = useLocalFrontendUrl();

  useEffect(() => {
    IPlugSendMsg({ tag: MessageTag.URL_REQUEST });
  }, []);

  return (
    <div>
      Hello world!
      {frontendUrl && <QRCode value={frontendUrl} />}
    </div>
  );
}
