import {
  useState,
  useCallback,
  useRef,
  MouseEvent as ReactMouseEvent,
} from 'react';

import HandleSrc from '../../assets/handle.png';

/**
 * Scale value n from lowBound1 < n < highBound1 to lowBound2 < n highBound2
 */
const convertRange = (
  oldMin: number,
  oldMax: number,
  newMin: number,
  newMax: number,
  value: number
) => {
  return Math.floor(
    ((value - oldMin) * (newMax - newMin)) / (oldMax - oldMin) + newMin
  );
};

/**
 * Calculates how much the current point has moved from the starting point, also taking into
 * account previous position via startingDelta
 */
function calcDelta(
  startingDelta: number,
  start: ReactMouseEvent,
  current: MouseEvent
) {
  return current.clientX - start.clientX + startingDelta;
}

/**
 * Bounds a number such that bound1|bound2 < delta < bound1|bound2, where one may not
 * know whether or not bound1 < bound2
 */
function boundDelta(delta: number, bound1: number, bound2: number) {
  const lb = bound1 > bound2 ? bound2 : bound1;
  const hb = lb === bound1 ? bound2 : bound1;
  return Math.max(Math.min(delta, hb), lb);
}

let lastValue = 127;

export default function HorizontalSlider() {
  const [delta, setDelta] = useState(0); // handles animation, for efficiency
  const [deltaPercent, setDeltaPercent] = useState(0); // sets the margin of slider handle as a percentage. lil smelly but oh well
  const boundingBox = useRef<HTMLDivElement>(null);

  const startDrag = useCallback(
    (clickEvent: ReactMouseEvent) => {
      if (boundingBox.current === null) return; // this won't happen
      clickEvent.preventDefault(); // prevent ondrag from firing

      document.body.classList.add('dragging');

      const bounds = boundingBox.current.getBoundingClientRect();

      const startDelta = delta;
      const hb = bounds.right - bounds.left;

      const moveHandler = (moveEvent: MouseEvent) => {
        // calculate how far the mouse has moved, bound to range, set
        let d = calcDelta(startDelta, clickEvent, moveEvent);
        d = boundDelta(d, 0, hb);

        setDelta(d);
        setDeltaPercent(d / hb);

        // convert to midi value, send
        const converted = convertRange(0, hb, 0, 127, d);
        if (converted !== lastValue) {
          lastValue = converted;
        }
      };

      const mouseUpHandler = () => {
        document.removeEventListener('mouseup', mouseUpHandler);
        document.removeEventListener('mousemove', moveHandler);
        document.body.classList.remove('dragging');
      };

      document.addEventListener('mousemove', moveHandler);
      document.addEventListener('mouseup', mouseUpHandler);
    },
    [delta, setDelta]
  );

  return (
    <div className="horizontal-slider">
      <div className="bounding-box" ref={boundingBox}>
        <img
          src={HandleSrc}
          style={{
            marginLeft: `${deltaPercent * 100}%`,
            transform: `translateX(-50%)`,
          }}
          onMouseDown={startDrag}
        />
      </div>
    </div>
  );
}
