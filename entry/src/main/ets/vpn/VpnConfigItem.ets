export interface VpnConfigItem {
  id: string;
  name: string;
  server: string;
  token: string;
  password: string;
  deviceId: string;
  deviceName: string;
  virtualIp: string;
  mtu: string;
  useChannel: string;
  stunServers: string;
  nameServers: string;
  ports: string;
  cipherModel: string;
  compressor: string;
  serverEncrypt: boolean;
  finger: boolean;
  enableTraffic: boolean;
}
