import { LogLevelType } from '../types';
import { LogTransport, LogEntry } from './log-transport';
import { Severity } from './transport-constants';

export class ConsoleTransport implements LogTransport {
  private logLevel: LogLevelType;

  constructor({ logLevel }: { logLevel: LogLevelType }) {
    this.logLevel = logLevel;
  }

  public log(entry: LogEntry) {
    if (Severity[entry.logLevel] < Severity[this.logLevel]) return;

    const output = `[${entry.logLevel.toUpperCase()}] ${entry.timestamp.toLocaleTimeString('en-US', { hour12: false })}${
      entry.moduleName ? ` [${entry.moduleName}]` : ''
    } ${entry.message}`;

    switch (entry.logLevel) {
      case 'verbose':
      case 'info':
        console.info(output);
        break;
      case 'warn':
        console.warn(output);
        break;
      case 'error':
      case 'fatal':
        console.error(output);
        break;
    }
  }
}
