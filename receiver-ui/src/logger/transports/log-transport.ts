import { LogLevelType } from '../types';

export interface LogEntry {
  logLevel: LogLevelType;
  message: string;
  moduleName?: string;
  timestamp: Date;
}

export interface LogTransport {
  log: (entry: LogEntry) => void;
}
