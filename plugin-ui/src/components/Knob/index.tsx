import { useCallback, useState, MouseEvent as ReactMouseEvent } from 'react';
import KnobSrc from '../../assets/knob.png';
import './Knob.css';

const convertRange = (
  oldMin: number,
  oldMax: number,
  newMin: number,
  newMax: number,
  v: number
) => {
  return ((v - oldMin) * (newMax - newMin)) / (oldMax - oldMin) + newMin;
};

// the last value that was propagated
let lastValue = -1;

export function Knob() {
  const degrees = 270;
  const min = 0;
  const max = 127;

  const lowerAngleBound = (360 - degrees) / 2;
  const upperAngleBound = lowerAngleBound + degrees;

  const [currentDeg, setCurrentDeg] = useState(315);

  const startDrag = useCallback(
    (e: ReactMouseEvent) => {
      e.preventDefault(); // prevent ondrag from firing
      const knob = (e.target as HTMLElement).getBoundingClientRect();
      document.body.classList.add('dragging');

      // get center of the knob
      const center = {
        x: knob.left + knob.width / 2,
        y: knob.top + knob.height / 2,
      };

      // set move handler
      const moveHandler = (innerEvent: MouseEvent) => {
        const point = {
          x: innerEvent.clientX,
          y: innerEvent.clientY,
        };

        const deg = getDeg(point, center, lowerAngleBound, upperAngleBound);
        setCurrentDeg(deg);

        const newValue = Math.floor(
          convertRange(lowerAngleBound, upperAngleBound, min, max, deg)
        );

        // don't both sending the same value twice
        if (newValue !== lastValue) {
          lastValue = newValue;
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
    [lowerAngleBound, upperAngleBound, min, max]
  );

  return (
    <div className="knob" role="button" onMouseDown={startDrag}>
      <div
        className="knob-image-container"
        style={{ transform: `rotate(${currentDeg}deg)` }}
      >
        <img src={KnobSrc} />
      </div>
    </div>
  );
}

/**
 * Calculate radians based on point position, bound appropriately
 */
const getDeg = (
  point: { x: number; y: number },
  center: { x: number; y: number },
  lowerBoundAngle: number,
  upperBoundAngle: number
) => {
  const x = point.x - center.x; // dx
  const y = point.y - center.y; // dy

  let deg = (Math.atan(y / x) * 180) / Math.PI;

  // set (0, -1) to 0 radians
  if ((x < 0 && y >= 0) || (x < 0 && y < 0)) deg += 90;
  else deg += 270;

  // bound return value to startAngle < deg < upperBoundAngle, return
  return Math.min(Math.max(lowerBoundAngle, deg), upperBoundAngle);
};
