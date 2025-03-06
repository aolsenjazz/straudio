class ConfigClazz {
  private static instance: ConfigClazz;

  private _localFrontendUrl: string | undefined = undefined;

  private constructor() {
    // no-op
  }

  static getInstance() {
    if (!ConfigClazz.instance) {
      ConfigClazz.instance = new ConfigClazz();
    }

    return ConfigClazz.instance;
  }

  public get localFrontendUrl() {
    if (this._localFrontendUrl === undefined)
      throw new Error('_localFrontendUrl has not been set!');

    return this._localFrontendUrl;
  }

  public set localFrontendUrl(localFrontendUrl: string) {
    this._localFrontendUrl = localFrontendUrl;
  }
}

export const Config = ConfigClazz.getInstance();
