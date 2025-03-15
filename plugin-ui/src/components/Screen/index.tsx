import { useState } from 'react';
import LoadingScreen from './LoadingScreen';
import './Screen.css';
import QRCode from 'react-qr-code';

type PropTypes = {
  frontendUrl: string | undefined;
};

function Screen(props: PropTypes) {
  const { frontendUrl = 'Uh oh.' } = props;

  const [loading, setLoading] = useState<boolean>(true);

  const handleFinishLoading = () => {
    setLoading(false);
  };

  return (
    <div id="screen">
      {loading ? (
        <LoadingScreen onFinish={handleFinishLoading} loadingText="STRAUDIO." />
      ) : (
        <QRCode
          value={frontendUrl}
          size={95}
          style={{ borderRadius: 10, marginTop: 5 }}
        />
      )}
    </div>
  );
}

export default Screen;
