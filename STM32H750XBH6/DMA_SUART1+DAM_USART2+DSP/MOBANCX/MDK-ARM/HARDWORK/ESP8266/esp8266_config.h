#ifndef __ESP8266_CONFIG_H
#define __ESP8266_CONFIG_H

/* ================= 用户配置区（本地私有，不要提交） ================= */
/* WiFi（注意：2.4GHz 才能连 ESP8266；很多 “xxx-5G” SSID 实际也可能支持 2.4GHz，取决于路由器） */
#define WIFI_SSID       "CMCC"
#define WIFI_PASSWORD   "13978532661"

/* EdgeWind 服务器（你的电脑局域网 IP） */
#define SERVER_IP       "192.168.10.43"
#define SERVER_PORT     5000

/* 节点信息 */
#define NODE_ID         "STM32_H7_智能测试节点"
#define NODE_LOCATION   "Lab_Test"

#endif /* __ESP8266_CONFIG_H */

