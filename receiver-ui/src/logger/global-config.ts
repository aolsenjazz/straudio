import { LogTransport } from './transports/log-transport';

export interface GlobalLoggerConfig {
  transports: LogTransport[];
}

export const GlobalConfig: Readonly<GlobalLoggerConfig> = {
  transports: [],
};

export function setGlobalConfig(config: Partial<GlobalLoggerConfig>) {
  Object.assign(GlobalConfig, config);
}
