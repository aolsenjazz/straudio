import { useEffect, useState } from 'react';
import { Config } from '../config';

export function useLocalFrontendUrl() {
  const [frontendUrl, setFrontendUrl] = useState<string | undefined>(
    Config.localFrontendUrl
  );

  useEffect(() => {
    Config.addListener<string>('localFrontendUrl', setFrontendUrl);

    return () => {
      Config.removeListener<string>('localFrontendUrl', setFrontendUrl);
    };
  }, []);

  return frontendUrl;
}
