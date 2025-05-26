import { LogLevel } from '../log-level';

export const Severity = {
  [LogLevel.FATAL]: 4,
  [LogLevel.ERROR]: 3,
  [LogLevel.WARN]: 2,
  [LogLevel.INFO]: 1,
  [LogLevel.VERBOSE]: 0,
} as const;
