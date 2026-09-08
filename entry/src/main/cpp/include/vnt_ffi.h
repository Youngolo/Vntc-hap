/*
 * vnt_ffi —— vnt 动态库 C 接口
 *
 * 使用方式：调用方（App / VpnExtensionAbility）负责创建虚拟网卡，
 * 把 tun 的文件描述符通过 VntConfig.tun_fd 传入，本库负责收发 IP 报文。
 *
 * 所有由本库返回的 char*（除 vnt_version 外）均需使用 vnt_string_free 释放。
 */
#ifndef VNT_FFI_H
#define VNT_FFI_H

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------- 常量 ---------------- */

/* 事件类型（vnt_set_event_callback 的 event 参数） */
#define VNT_EVENT_SUCCESS  0 /* 连接成功 */
#define VNT_EVENT_REGISTER 1 /* 注册成功 */
#define VNT_EVENT_TUN_INFO 2 /* 虚拟网卡信息就绪：{"virtual_ip":..,"netmask":..,"gateway":..,"mtu":..} */
#define VNT_EVENT_PEERS    3 /* 设备列表变化：[{"ip":..,"name":..,"online":true}] */
#define VNT_EVENT_ERROR    4 /* 错误：{"code":..,"msg":..} */
#define VNT_EVENT_STOP     5 /* 已停止 */

/* 日志级别（vnt_set_log_callback 的 level 参数） */
#define VNT_LOG_ERROR 1
#define VNT_LOG_WARN  2
#define VNT_LOG_INFO  3
#define VNT_LOG_DEBUG 4
#define VNT_LOG_TRACE 5

/* 加密算法（VntConfig.cipher_model） */
#define VNT_CIPHER_AES_GCM   0
#define VNT_CIPHER_CHACHA20_POLY1305 1
#define VNT_CIPHER_CHACHA20  2
#define VNT_CIPHER_AES_CBC   3
#define VNT_CIPHER_AES_ECB   4
#define VNT_CIPHER_XOR       5
#define VNT_CIPHER_NONE      6

/* 打洞模型（VntConfig.punch_model） */
#define VNT_PUNCH_ALL      0
#define VNT_PUNCH_IPV4     1
#define VNT_PUNCH_IPV6     2
#define VNT_PUNCH_IPV4_TCP 3
#define VNT_PUNCH_IPV4_UDP 4
#define VNT_PUNCH_IPV6_TCP 5
#define VNT_PUNCH_IPV6_UDP 6

/* 通道类型（VntConfig.use_channel） */
#define VNT_CHANNEL_RELAY 0 /* 仅服务端转发 */
#define VNT_CHANNEL_P2P   1 /* 仅点对点 */
#define VNT_CHANNEL_ALL   2 /* 自动（推荐） */

/* 压缩算法（VntConfig.compressor） */
#define VNT_COMPRESS_NONE 0
#define VNT_COMPRESS_LZ4  1

/* ---------------- 回调 ---------------- */

/* json 仅在回调期间有效，如需保存请自行拷贝 */
typedef void (*vnt_event_cb)(void *ctx, int event, const char *json);
typedef void (*vnt_log_cb)(void *ctx, int level, const char *msg);

/* ---------------- 结构体 ---------------- */

typedef struct VntHandle VntHandle;

typedef struct {
    /* 必填：组网令牌（与服务端一致） */
    const char *token;
    /* 必填：本设备唯一标识，同一网络内不可重复 */
    const char *device_id;
    /* 必填：设备名称（展示用） */
    const char *name;
    /* 必填：服务端地址，支持 udp://host:port、tcp://host:port、
     * ws://host:port、wss://host:port、host:port（等价于 udp://） */
    const char *server;
    /* 可选：组网密码（与服务端一致），无密码填 NULL */
    const char *password;

    /* 可选：DNS，逗号分隔，默认 223.5.5.5,114.114.114.114 */
    const char *name_servers;
    /* 可选：STUN 服务器，逗号分隔，默认 stun.miwifi.com 等 */
    const char *stun_servers;
    /* 可选：指定物理网卡名（多网卡时） */
    const char *local_dev;
    /* 可选：指定本地端口，逗号分隔，如 35535,35536 */
    const char *ports;

    /* 可选：入站路由（本端网段 -> 对端），逗号分隔，格式 x.x.x.x/mask,gateway，
     * 例如 192.168.0.0/24,10.26.0.3 */
    const char *in_ips;
    /* 可选：出站路由，逗号分隔，格式 x.x.x.x/mask，默认 0.0.0.0/0 */
    const char *out_ips;
    /* 可选：期望的本机虚拟 IP，不指定则由服务端分配 */
    const char *virtual_ip;

    /* 必填：虚拟网卡文件描述符（VpnConnection.create 返回值） */
    int tun_fd;
    /* 可选：MTU，0 表示使用默认 1400，需与 VPN 配置一致 */
    unsigned int mtu;

    int cipher_model;     /* 见 VNT_CIPHER_*，默认 AES-GCM */
    int punch_model;      /* 见 VNT_PUNCH_*，默认 ALL */
    int use_channel;      /* 见 VNT_CHANNEL_*，默认 ALL */
    int compressor;       /* 见 VNT_COMPRESS_*，默认 LZ4 */
    int server_encrypt;   /* 非 0 表示启用服务端加密 */
    int finger;           /* 非 0 表示启用数据包指纹校验 */
    int first_latency;    /* 非 0 表示优先低延迟通道 */
    int enable_traffic;   /* 非 0 表示统计流量 */
    int allow_wire_guard; /* 非 0 表示允许转发 wireguard 流量 */
} VntConfig;

/* ---------------- 接口 ---------------- */

/* 库版本号（静态字符串，无需释放） */
const char *vnt_version(void);

/* 初始化日志器，进程内首次调用即可 */
void vnt_init(void);

/* 设置事件回调（cb 传 NULL 取消） */
void vnt_set_event_callback(vnt_event_cb cb, void *ctx);

/* 设置日志回调（cb 传 NULL 取消） */
void vnt_set_log_callback(vnt_log_cb cb, void *ctx);

/* 启动 vnt；成功返回句柄，失败返回 NULL，错误见 vnt_last_error */
VntHandle *vnt_start(const VntConfig *cfg);

/* 停止并回收读线程；返回 0 成功，-1 句柄无效 */
int vnt_stop(VntHandle *handle);

/* 释放句柄（内部先停止） */
void vnt_free(VntHandle *handle);

/* 是否运行：1 运行、0 已停止、-1 句柄无效 */
int vnt_is_running(VntHandle *handle);

/* 运行状态 JSON：{"virtual_ip":..,"netmask":..,"gateway":..,"server":..,
 *                "online":true,"running":true,"up_stream":0,"down_stream":0} */
char *vnt_status_json(VntHandle *handle);

/* 设备列表 JSON：[{"ip":..,"name":..,"online":true,"wireguard":false}] */
char *vnt_device_list_json(VntHandle *handle);

/* 已发送字节数 */
unsigned long long vnt_up_stream(VntHandle *handle);

/* 已接收字节数 */
unsigned long long vnt_down_stream(VntHandle *handle);

/* 收集需要保护的 UDP socket fd（避免 VPN 流量回环），返回写入数量，-1 表示参数错误。
 * out 需由调用方提供，建议容量 64 */
int vnt_collect_udp_fds(VntHandle *handle, int *out, int capacity);

/* 上一次错误信息（不清空） */
char *vnt_last_error(void);

/* 释放本库返回的字符串 */
void vnt_string_free(char *s);

#ifdef __cplusplus
}
#endif

#endif /* VNT_FFI_H */
