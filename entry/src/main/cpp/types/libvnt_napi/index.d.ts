export const version: () => string;
export const start: (tunFd: number, token: string, deviceId: string, name: string, server: string,
  password: string, virtualIp: string, mtu: number, useChannel: number, stunServers: string,
  nameServers: string, ports: string, cipherModel: number, compressor: number,
  serverEncrypt: number, finger: number, enableTraffic: number) => boolean;
export const stop: () => boolean;
export const isRunning: () => boolean;
export const status: () => string;
export const deviceList: () => string;
export const lastError: () => string;
export const upStream: () => number;
export const downStream: () => number;
export const setEventListener: (listener: ((event: number, json: string) => void) | null) => void;
