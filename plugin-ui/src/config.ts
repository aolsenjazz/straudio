type Listener<T> = (value: T | undefined) => void;

class ConfigClazz {
  private static instance: ConfigClazz;

  private _localFrontendUrl: string | undefined = undefined;

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  private _listeners = new Map<string, Set<Listener<any>>>();

  private constructor() {
    // no-op
  }

  static getInstance() {
    if (!ConfigClazz.instance) {
      ConfigClazz.instance = new ConfigClazz();
    }

    return ConfigClazz.instance;
  }

  public get localFrontendUrl(): string | undefined {
    return this._localFrontendUrl;
  }

  public set localFrontendUrl(localFrontendUrl: string | undefined) {
    this._localFrontendUrl = localFrontendUrl;
    this._notifyListeners('localFrontendUrl', localFrontendUrl);
  }

  public addListener<T>(field: string, listener: Listener<T>): void {
    if (!this._listeners.has(field)) {
      this._listeners.set(field, new Set());
    }
    this._listeners.get(field)!.add(listener);
  }

  public removeListener<T>(field: string, listener: Listener<T>): void {
    if (this._listeners.has(field)) {
      this._listeners.get(field)!.delete(listener);
    }
  }

  private _notifyListeners<T>(field: string, value: T | undefined): void {
    if (this._listeners.has(field)) {
      this._listeners.get(field)!.forEach((listener) => listener(value));
    }
  }
}

export const Config = ConfigClazz.getInstance();
