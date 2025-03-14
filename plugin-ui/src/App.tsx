import { useEffect } from 'react';

import { MessageTag } from './ipc/message-tags';
import { useLocalFrontendUrl } from './hooks/use-local-frontend-url';
import { Knob } from './components/Knob';
import HorizontalSlider from './components/HorizontalSlider';
import Screen from './components/Screen';

export default function App() {
  const frontendUrl = useLocalFrontendUrl();

  useEffect(() => {
    IPlugSendMsg({ tag: MessageTag.URL_REQUEST });
  }, []);

  return (
    <div id="ui-container">
      <Screen frontendUrl={frontendUrl} />
      <div id="monitor-knob-container">
        <Knob />
      </div>
      <div id="transmit-knob-container">
        <Knob />
      </div>
      <div id="latency-slider-container">
        <HorizontalSlider />
      </div>
    </div>
  );
}
