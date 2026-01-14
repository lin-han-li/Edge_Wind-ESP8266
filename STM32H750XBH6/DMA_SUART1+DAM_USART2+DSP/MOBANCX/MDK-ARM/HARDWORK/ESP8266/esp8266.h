#ifndef __ESP8266_H
#define __ESP8266_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* ================= 用户配置区 ================= */
// 引入用户具体的 WiFi 和服务器配置
#include "esp8266_config.h"

// 兜底宏定义：如果用户没有在 config.h 中定义，则使用这里的默认值，防止编译报错
#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif
#ifndef SERVER_IP
#define SERVER_IP "192.168.10.43"
#endif
#ifndef SERVER_PORT
#define SERVER_PORT 5000
#endif
#ifndef NODE_ID
#define NODE_ID "STM32_H7_Node"
#endif
#ifndef NODE_LOCATION
#define NODE_LOCATION "Lab_Test"
#endif

/* ================= 调试输出配置 =================
 * 控制日志打印的行为。
 * ESP8266 连接在 USART2，因此调试日志(printf)不能往 USART2 打，否则会干扰 AT 指令。
 * 通常调试日志输出到 USART1 (连接电脑 USB 转串口)。
 */
#ifndef ESP_DEBUG
#define ESP_DEBUG 1 // 1: 开启调试日志，0: 关闭
#endif

#ifndef ESP_LOG_UART_PORT
#define ESP_LOG_UART_PORT 1 // 指定调试日志使用的串口号 (1 -> huart1)
#endif

/* ================= 串口命令（故障注入）=================
 * 这是一个非常实用的功能：通过串口助手输入指令来模拟故障。
 * 例如输入 "E01" 可以让 STM32 假装发生了故障，从而测试后端的报警功能。
 */
#ifndef ESP_CONSOLE_ENABLE
#define ESP_CONSOLE_ENABLE 1 // 1: 启用串口控制台功能
#endif

/* ================= 链路自恢复配置 =================
 * - 启动强制硬复位：保证每次上电/复位都从干净的 ESP 状态开始
 * - 无服务器响应硬复位阈值：连续 N 秒收不到服务器任何响应（HTTP 回包）则直接硬复位并重连
 */
#ifndef ESP_BOOT_HARDRESET_ONCE
#define ESP_BOOT_HARDRESET_ONCE 1
#endif

#ifndef ESP_NO_SERVER_RX_HARDRESET_SEC
#define ESP_NO_SERVER_RX_HARDRESET_SEC 6
#endif

// 数据采样与发送参数
#define WAVEFORM_POINTS 1024 // 本地模拟生成的波形点数 (用于高精度 FFT 计算)
/* * ⚠️ 关键参数 WAVEFORM_SEND_STEP:
 * 因为 1024 个浮点数转成字符串通过 UART 发送，数据量极大(约几KB)，容易造成阻塞或溢出。
 * 这里设置为 4，表示每隔 4 个点取 1 个点发送给后端 (1024/4 = 256 点)，
 * 既保证了波形大致形状，又显著降低了数据传输量。
 */
// 压力测试：不降采样（用户要求）
#ifndef WAVEFORM_SEND_STEP
#define WAVEFORM_SEND_STEP 1
#endif

#ifndef FFT_POINTS
#define FFT_POINTS 256
#endif

    /* ================= 数据结构定义 ================= */

    /* 单个通道的数据结构 */
    typedef struct
    {
        uint8_t id;                      // 通道编号 (0~3)
        char label[32];                  // 标签 (如 "直流母线(+)")，后端依据此字段识别数据含义
        char unit[8];                    // 单位 (V, A, mA)
        char type[16];                   // 类型 (预留字段)
        float current_value;             // 当前显示的有效值/瞬时值
        float waveform[WAVEFORM_POINTS]; // 时域波形数组 (源数据)
        float fft_data[FFT_POINTS];      // 频域数据数组 (FFT 计算结果)
    } Channel_Data_t;

    /* 全局变量声明 */
    extern Channel_Data_t node_channels[4]; // 4个通道实例：母线+, 母线-, 电流, 漏电
    extern volatile uint8_t g_esp_ready;    // 标志位：1表示WiFi/TCP已连接，可以发送数据

    /* ================= 函数声明 ================= */
    void ESP_Init(void);                // 初始化 ESP8266 (AT指令序列)
    void ESP_Update_Data_And_FFT(void); // 更新模拟数据并计算 FFT
    void ESP_Post_Data(void);           // 打包 JSON 并通过 HTTP POST 发送
    void ESP_Register(void);            // 向服务器注册节点信息
    void ESP_Console_Init(void);        // 初始化调试控制台中断
    void ESP_Console_Poll(void);        // 在主循环中轮询控制台输入

#ifdef __cplusplus
}
#endif

#endif /* __ESP8266_H */
