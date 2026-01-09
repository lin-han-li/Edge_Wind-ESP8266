#ifndef __ESP8266_H
#define __ESP8266_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* ================= 用户配置区 ================= */
#define WIFI_SSID       "IQOO11"
#define WIFI_PASSWORD   "12345688"
#define SERVER_IP       "192.168.118.147"
#define SERVER_PORT     5000

#define NODE_ID         "STM32_H7_Node"
#define NODE_LOCATION   "Lab_Test"

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

