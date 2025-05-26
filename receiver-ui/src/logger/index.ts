import { GlobalConfig } from './global-config';
import { Logger } from './logger';
import { LoggerConfig } from './types';

export { setGlobalConfig } from './global-config';

export function createLogger(config: Partial<LoggerConfig> = {}) {
  const mergedConfig = { ...GlobalConfig, ...config };
  return new Logger(mergedConfig);
}
