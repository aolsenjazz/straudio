import { LogLevel } from './log-level';

export type LogLevelType = (typeof LogLevel)[keyof typeof LogLevel];

export type LogPriority = Record<LogLevelType, number>;

export type LoggerConfig = {
  moduleName?: string;
};
