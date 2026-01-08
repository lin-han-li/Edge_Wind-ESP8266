#ifndef __ESP8266_H
#define __ESP8266_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* ================= 用户配置区 ================= */
/**
 * ⚠️ 注意（准备上传 GitHub 前必须做）：
 * - WiFi SSID/密码、服务器 IP 等属于“环境敏感信息”，不应硬编码进仓库。
 * - 请复制同目录下的 `esp8266_config.h.example` 为 `esp8266_config.h`（该文件已加入 .gitignore）。
 */
#include "esp8266_config.h"

/* 若未提供本地配置，则使用占位默认值（便于编译通过，但实际无法联网） */
#ifndef WIFI_SSID
#define WIFI_SSID       "YOUR_WIFI_SSID"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#endif
#ifndef SERVER_IP
#define SERVER_IP       "192.168.1.100"
#endif
#ifndef SERVER_PORT
#define SERVER_PORT     5000
#endif

#ifndef NODE_ID
#define NODE_ID         "STM32_H7_Node"
#endif
#ifndef NODE_LOCATION
#define NODE_LOCATION   "Lab_Test"
#endif

// 数据参数
#define WAVEFORM_POINTS 1024       // 采集点数
#define WAVEFORM_SEND_STEP 4       // ⚠️改为4 (发256点)，因为4个通道数据量太大，必须降采样防溢出
#define FFT_POINTS      256        // FFT发送点数

/* ================= 数据结构定义 ================= */

typedef struct {
    uint8_t  id;                // 通道ID
    char     label[32];         // 标签 (关键！后端靠这个识别)
    char     unit[8];           // 单位
    char     type[16];          // 类型
    float    current_value;     // 实时值
    float    waveform[WAVEFORM_POINTS]; // 波形
    float    fft_data[FFT_POINTS];      // 频谱
} Channel_Data_t;

/* ⚠️ 改为 4 个通道 */
extern Channel_Data_t node_channels[4];
extern volatile uint8_t g_esp_ready;

/* ================= 函数声明 ================= */
void ESP_Init(void);
void ESP_Update_Data_And_FFT(void);
void ESP_Post_Data(void);
void ESP_Register(void);

#ifdef __cplusplus
}
#endif

#endif /* __ESP8266_H */

