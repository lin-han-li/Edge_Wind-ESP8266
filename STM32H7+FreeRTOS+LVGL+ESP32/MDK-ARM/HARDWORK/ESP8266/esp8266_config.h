#ifndef __ESP8266_CONFIG_H
#define __ESP8266_CONFIG_H

/* ================= 用户配置区（本地私有，不要提交） ================= */

/* * WiFi 配置
 * 注意事项：
 * 1. ESP8266 仅支持 2.4GHz 频段 WiFi。
 * 2. 如果使用手机热点或双频路由器，请确保开启 2.4G 频段。
 * 3. 这里的账号密码是硬编码的，实际产品中通常通过配网模式(SmartConfig)设置。
 */
#define WIFI_SSID "CMCC"            // WiFi 名称
#define WIFI_PASSWORD "13978532661" // WiFi 密码

/* * EdgeWind 后端服务器配置
 * 这是运行 Python/Flask 后端的电脑 IP 地址。
 * 必须确保 STM32(通过ESP8266) 和电脑在同一个局域网网段内。
 */
#define SERVER_IP "192.168.10.43" // 目标服务器 IP
#define SERVER_PORT 5000          // 目标端口 (对应 app.py 中的 port)

/* * 节点身份信息
 * 用于在后端区分不同的监测设备。
 */
#define NODE_ID "STM32_H7_智能RTOS设备" // 设备唯一标识 ID
#define NODE_LOCATION "Lab_Test"        // 设备部署位置

#endif /* __ESP8266_CONFIG_H */

