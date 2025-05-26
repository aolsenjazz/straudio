import { setGlobalConfig } from './logger';
import { LogLevel } from './logger/log-level';
import { ConsoleTransport } from './logger/transports/console-transport';

setGlobalConfig({
  transports: [new ConsoleTransport({ logLevel: LogLevel.INFO })],
});
