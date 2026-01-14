#ifndef __ESP8266_H
#define __ESP8266_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* ================= 用户配置区 ================= */
// 推荐：使用本地私有配置文件（已在 .gitignore 忽略，避免提交 WiFi/服务器敏感信息）
// 同目录下提供模板：esp8266_config.h.example
#include "esp8266_config.h"

// 兼容兜底：如果没有提供 esp8266_config.h，则使用默认值（请按需修改）
#ifndef WIFI_SSID
#define WIFI_SSID       "YOUR_WIFI_SSID"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#endif
#ifndef SERVER_IP
#define SERVER_IP       "192.168.10.43"
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

/* ================= 调试输出配置 =================
 * 说明：ESP8266 使用 USART2 通信；本工程 printf 默认重定向到 USART1（见 Core/Src/usart.c 的 fputc）。
 * - ESP_DEBUG: 1=开启调试日志（推荐联调时开启）
 * - ESP_LOG_UART_PORT: 1=USART1（推荐接到 PC 串口助手）
 *   如果你板子上 VCP/串口接的不是 USART1，可按需改成 2/3/... 并在 esp8266.c 中补充分支映射。
 */
#ifndef ESP_DEBUG
#define ESP_DEBUG 1
#endif

#ifndef ESP_LOG_UART_PORT
#define ESP_LOG_UART_PORT 1
#endif

/* ================= 串口命令（故障注入）=================
 * 连接到调试串口（默认 USART1）后，直接输入：E00 / E01 / E02 ... 回车即可切换上报故障码
 * - 目的：联调时方便通过串口快速模拟故障
 */
#ifndef ESP_CONSOLE_ENABLE
#define ESP_CONSOLE_ENABLE 1
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
void ESP_Console_Init(void);
void ESP_Console_Poll(void);

#ifdef __cplusplus
}
#endif

#endif /* __ESP8266_H */

