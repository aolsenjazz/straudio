import { useEffect, useState } from 'react';
import './LoadingScreen.css';

interface PropTypes {
  onFinish: () => void;
  loadingText: string;
  loadingDurationSeconds?: number; // How long the text stays visible after it appears (default 1 second)
  startDelayMs?: number; // How long to wait before showing the text (default 500ms)
  finishDelayMs?: number; // Additional delay after the text disappears before finishing (default 500ms)
  fadeIn?: boolean; // Whether to apply a fade in
  fadeInDurationMs?: number; // Duration of the fade in animation (default 200ms)
}

function LoadingScreen(props: PropTypes) {
  const {
    onFinish,
    loadingText,
    loadingDurationSeconds = 1.5,
    startDelayMs = 500,
    finishDelayMs = 500,
    fadeIn = true,
    fadeInDurationMs = 200,
  } = props;

  const [showText, setShowText] = useState(false);

  useEffect(() => {
    // Timer to show the text after the start delay.
    const showTimer = setTimeout(() => {
      setShowText(true);
    }, startDelayMs);

    // Timer to hide the text after it has been visible for the loading duration.
    const hideTimer = setTimeout(
      () => {
        setShowText(false);
      },
      startDelayMs + loadingDurationSeconds * 1000
    );

    // Timer to finish the loading screen after an additional finish delay.
    const finishTimer = setTimeout(
      () => {
        onFinish();
      },
      startDelayMs + loadingDurationSeconds * 1000 + finishDelayMs
    );

    return () => {
      clearTimeout(showTimer);
      clearTimeout(hideTimer);
      clearTimeout(finishTimer);
    };
  }, [onFinish, startDelayMs, loadingDurationSeconds, finishDelayMs]);

  return (
    <div className="screen-container">
      {showText && (
        <div
          className={`marquee ${fadeIn ? 'fade-in' : ''}`}
          style={fadeIn ? { animationDuration: `${fadeInDurationMs}ms` } : {}}
        >
          {loadingText}
        </div>
      )}
    </div>
  );
}

export default LoadingScreen;
