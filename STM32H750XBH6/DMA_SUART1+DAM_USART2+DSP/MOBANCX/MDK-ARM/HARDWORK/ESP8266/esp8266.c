/**
 ******************************************************************************
 * @file    esp8266.c
 * @author  STM32H7 Optimization Expert
 * @brief   ESP8266 WiFi 模组驱动程序 (STM32H7 专用优化版)
 * @note    核心特性：
 * 1. AXI SRAM + D-Cache 一致性维护 (DMA 必备)
 * 2. 2Mbps 高波特率下的软件容错机制 (抗 ORE/FE 错误)
 * 3. 智能流式接收解析 (滑动窗口)
 * 4. 自动故障恢复与重连逻辑
 ******************************************************************************
 */

#include "esp8266.h"
#include "usart.h"
#include "arm_math.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/* 引用 usart.c 中定义的句柄 */
extern UART_HandleTypeDef huart2;

// =================================================================================
// 1. STM32H7 特有的内存管理与 Cache 维护
// =================================================================================
// [背景知识]
// STM32H7 的架构非常复杂。普通的 DTCM RAM (0x20000000) 通常连接在 CPU 的紧耦合总线上，
// DMA 控制器（尤其是 DMA1/DMA2）往往无法直接访问 DTCM，或者访问效率极低。
// 必须将用于 DMA 传输的缓冲区（Buffer）放到 AXI SRAM (0x24000000) 或 D2 域的 SRAM1/2/3。
// 此外，H7 默认开启 D-Cache（数据缓存）。CPU 写数据进 Cache 后，物理内存里可能还是旧数据；
// 此时如果 DMA 从内存搬运数据，发出去的就是错的。
// 所以：
// - 发送前：必须 Clean Cache (将 Cache 数据刷入物理内存)。
// - 接收后：必须 Invalidate Cache (废弃 Cache 数据，强迫 CPU 下次从物理内存重读)。

// 定义通过链接器脚本定位的内存段，确保变量存放在 AXI SRAM 中
#define AXI_SRAM_SECTION __attribute__((section(".axi_sram")))
// 强制 32 字节对齐，这是 Cortex-M7 Cache Line 的大小。
// 如果不对齐，Cache 操作可能会意外破坏相邻变量的数据（Cache 伪共享问题）。
#define DMA_ALIGN32 __attribute__((aligned(32)))

/* 辅助内联函数：用于计算对齐地址 */
static inline uint32_t _align_down_32(uint32_t x) { return x & ~31u; }
static inline uint32_t _align_up_32(uint32_t x) { return (x + 31u) & ~31u; }

/**
 * @brief  Clean D-Cache (将 Cache 中的脏数据写回物理内存)
 * @note   用于 DMA 发送前：CPU 准备好数据 -> Clean -> 内存更新 -> DMA 搬运
 * @param  addr: 缓冲区首地址
 * @param  len:  长度
 */
static void DCache_CleanByAddr_Any(void *addr, uint32_t len)
{
#if defined(SCB_CleanDCache_by_Addr)
    uint32_t a = _align_down_32((uint32_t)addr);
    uint32_t end = _align_up_32(((uint32_t)addr) + len);
    // 调用 CMSIS 标准库函数执行汇编指令
    SCB_CleanDCache_by_Addr((uint32_t *)a, (int32_t)(end - a));
#else
    (void)addr;
    (void)len;
#endif
}

/**
 * @brief  Invalidate D-Cache (使 Cache 失效，强制 CPU 下次从物理内存读取)
 * @note   用于 DMA 接收后：DMA 搬运数据到内存 -> Invalidate -> CPU 读内存新数据
 */
static void DCache_InvalidateByAddr_Any(void *addr, uint32_t len)
{
#if defined(SCB_InvalidateDCache_by_Addr)
    uint32_t a = _align_down_32((uint32_t)addr);
    uint32_t end = _align_up_32(((uint32_t)addr) + len);
    SCB_InvalidateDCache_by_Addr((uint32_t *)a, (int32_t)(end - a));
#else
    (void)addr;
    (void)len;
#endif
}

/* ================= 内存分配 ================= */

/* ⚠️ 发送缓冲区扩大到 48KB，用于存放巨大的 JSON 字符串（包含波形数组）。
 * 必须放在 AXI SRAM 并对齐，否则 DMA 发送会出错或导致 HardFault。 */
static uint8_t http_packet_buf[49152] AXI_SRAM_SECTION DMA_ALIGN32;

/* 简单的接收缓冲，用于 AT 指令阻塞接收 (AT模式下数据量小，且使用轮询，不需要DMA) */
static uint8_t esp_rx_buf[512];

/* 4 个通道的传感器数据结构体实例，用于存储电压、电流、FFT结果等 */
Channel_Data_t node_channels[4];
volatile uint8_t g_esp_ready = 0; // 全局标志：1 表示 WiFi/TCP 就绪，可以发送

/* DSP 相关变量：用于 FFT 计算 */
static arm_rfft_fast_instance_f32 S;
static uint8_t fft_initialized = 0;
static float32_t fft_output_buf[WAVEFORM_POINTS]; // FFT 输出复数数组
static float32_t fft_mag_buf[WAVEFORM_POINTS];    // FFT 幅值数组

/* 内部函数声明 */
static void ESP_Log(const char *format, ...);
static uint8_t ESP_Send_Cmd(const char *cmd, const char *reply, uint32_t timeout);
static void ESP_Clear_Error_Flags(void);
static void Helper_FloatArray_To_String(char *dest, float *data, int count, int step);
static void ESP_Exit_Transparent_Mode(void);
static uint8_t ESP_Exit_Transparent_Mode_Strict(uint32_t timeout_ms);
static void ESP_Uart2_Drain(uint32_t ms);
static uint8_t ESP_Wait_Keyword(const char *kw, uint32_t timeout_ms);
static void ESP_SoftReconnect(void);
static uint8_t ESP_TryReuseTransparent(void);
static void ESP_HardReset(void);
static void Process_Channel_Data(int ch_id, float base_dc, float ripple_amp, float noise_level);
static UART_HandleTypeDef *ESP_GetLogUart(void);
static void ESP_Log_RxBuf(const char *tag);
static void ESP_SetFaultCode(const char *code);
static void ESP_Console_HandleLine(char *line);
static void StrTrimInPlace(char *s);
static void ESP_StreamRx_Start(void);
static void ESP_StreamRx_Feed(const uint8_t *data, uint16_t len);

// “核武器”：强制停止 USART2 的 RX DMA/中断状态机，切换到 AT(阻塞收发)前必须调用
static void ESP_ForceStop_DMA(void);
static uint8_t ESP_BufContains(const uint8_t *buf, uint16_t len, const char *needle);

// 当前上报的故障码（默认正常 E00），可通过串口控制台动态修改
static char g_fault_code[4] = "E00";

// ---------- 串口控制台（调试串口 RX 中断） ----------
static uint8_t g_console_rx_byte = 0;
static volatile uint8_t g_console_line_ready = 0;
static char g_console_line[32];
static volatile uint8_t g_console_line_len = 0;

// ---------- 服务器下发命令解析 ----------
// 当从 USART2 收到的 HTTP 响应中提取到 "reset" 时置 1
static volatile uint8_t g_server_reset_pending = 0;

/* RX DMA 缓冲：用于接收服务器回包 (HTTP响应 / reset命令)。
 * 位于 AXI SRAM，加大到 4096 以降低 IDLE 中断触发频率，减少高负载下的丢包风险。 */
static uint8_t g_stream_rx_buf[4096] AXI_SRAM_SECTION DMA_ALIGN32;

/* 滑动窗口：用于在流式数据中查找关键字。
 * 解决 DMA 分包导致关键字（如 "HTTP/1.1"）被切断的问题。 */
static char g_stream_window[256];
static uint16_t g_stream_window_len = 0;

/* 调试与统计变量 */
static volatile uint32_t g_usart2_rx_events = 0;  // 接收中断次数
static volatile uint32_t g_usart2_rx_bytes = 0;   // 接收总字节数
static volatile uint8_t g_usart2_rx_started = 0;  // DMA 接收开启标志
static volatile uint32_t g_usart2_rx_restart = 0; // 重启次数
static volatile uint32_t g_uart2_err = 0;         // 总错误数
static volatile uint32_t g_uart2_err_ore = 0;     // 溢出错误 (Overrun) - 2Mbps下需重点关注
static volatile uint32_t g_uart2_err_fe = 0;      // 帧错误
static volatile uint32_t g_uart2_err_ne = 0;      // 噪声错误
static volatile uint32_t g_uart2_err_pe = 0;      // 校验错误

/* 链路健康监测 */
static volatile uint32_t g_last_rx_tick = 0;     // 上次收到数据的时间
static volatile uint8_t g_link_reconnecting = 0; // 是否正在重连中
static uint8_t g_boot_hardreset_done = 0;        // 启动时是否已执行过硬复位

/* HTTP 发送流控门控
 * 作用：发送 HTTP 请求后置为 1，收到回复或超时后置为 0。
 * 防止请求发送过快淹没服务器，导致 TCP 拥塞或解析错误。 */
static volatile uint8_t g_waiting_http_response = 0;
static volatile uint32_t g_waiting_http_tick = 0;

/* 关键标志位：指示当前是否处于 AT 命令模式
 * 当处于 AT 模式（使用阻塞式 HAL_UART_Receive）时，必须禁止 DMA 中断回调逻辑介入。
 * 否则 HAL_UARTEx_RxEventCallback 会打断 HAL_UART_Receive，导致状态机错乱死锁。 */
static volatile uint8_t g_uart2_at_mode = 0;

// =================================================================================
// 工具函数实现
// =================================================================================

/**
 * @brief  在缓冲区中查找子字符串（朴素匹配算法）
 * @note   用于在 DMA 原始 buffer 中直接搜索关键字
 */
static uint8_t ESP_BufContains(const uint8_t *buf, uint16_t len, const char *needle)
{
    if (!buf || len == 0 || !needle || !*needle)
        return 0;
    uint16_t nlen = (uint16_t)strlen(needle);
    if (nlen == 0 || nlen > len)
        return 0;
    // 朴素扫描：len<=4096，needle 很短，效率足够高
    for (uint16_t i = 0; i + nlen <= len; i++)
    {
        if (memcmp(buf + i, needle, nlen) == 0)
            return 1;
    }
    return 0;
}

/**
 * @brief  【核武器级函数】强制停止 DMA 并复位串口状态机
 * @note   这是解决 STM32 HAL 库混合使用 阻塞模式(AT) 和 DMA模式(透传) 导致死锁的关键。
 * HAL 库如果检测到 DMA 正在运行或 RxState 为 BUSY_RX_DMA，
 * 调用阻塞式的 HAL_UART_Receive 会直接返回 HAL_BUSY。
 * 所以在发送 AT 指令前，必须先调用此函数“杀掉”所有后台 DMA 任务。
 */
static void ESP_ForceStop_DMA(void)
{
    // 1) 关闭 IDLE 中断（ReceiveToIdle 依赖 IDLE，AT 阶段不应触发）
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_IDLE);

    // 2) 停止 DMA（包括 DMAR/DMAT），并强制 Abort UART
    // 这会将 HAL 内部状态重置为 READY
    (void)HAL_UART_DMAStop(&huart2);
    (void)HAL_UART_Abort(&huart2);

    // 3) 暴力复位 HAL 句柄状态 (双重保险，防止 Abort 不彻底)
    /* 注意：这是直接操作 HAL 结构体成员，为了应对极端卡死情况 */
    // huart2.gState = HAL_UART_STATE_READY; // TX 状态
    // huart2.RxState = HAL_UART_STATE_READY; // RX 状态

    // 4) 清除可能残留的硬件错误标志 (ORE/FE等)，避免一开启中断就进 ErrorHandler
    ESP_Clear_Error_Flags();

    // 5) 更新软件状态标志
    g_usart2_rx_started = 0;
    g_waiting_http_response = 0;
}

/**
 * @brief  获取用于打印日志的串口句柄
 */
static UART_HandleTypeDef *ESP_GetLogUart(void)
{
#if (ESP_LOG_UART_PORT == 1)
    extern UART_HandleTypeDef huart1;
    return &huart1;
#elif (ESP_LOG_UART_PORT == 2)
    return &huart2; // ⚠️一般不建议：USART2 用于 ESP8266 通信，混用会干扰协议
#else
    extern UART_HandleTypeDef huart1;
    return &huart1; // 默认回退 USART1
#endif
}

static void ESP_Log_RxBuf(const char *tag)
{
#if (ESP_DEBUG)
    if (tag == NULL)
        tag = "RX";
    ESP_Log("[ESP 回显 %s] << %s\r\n", tag, esp_rx_buf);
#else
    (void)tag;
#endif
}

/* ================= 核心代码 ================= */

/**
 * @brief  ESP8266 初始化主流程
 * @note   包含：硬复位 -> 复用探测 -> AT初始化 -> WiFi连接 -> TCP连接 -> 透传模式 -> 开启DMA监听
 */
void ESP_Init(void)
{
    char cmd_buf[128];
    g_esp_ready = 0;
    int retry_count = 0;

    // 步骤 1: 进 AT 模式前，必须清场，确保 UART 处于 READY 状态
    // 否则 HAL_UART_Receive 会直接返回 BUSY，导致初始化失败
    g_uart2_at_mode = 1;
    ESP_ForceStop_DMA();

    // 步骤 2: 启动时强制硬复位一次：保证 ESP 状态干净（用户配置）
#if (ESP_BOOT_HARDRESET_ONCE)
    if (!g_boot_hardreset_done)
    {
        g_boot_hardreset_done = 1;
        ESP_Log("[ESP] 启动硬复位一次...\r\n");
        ESP_HardReset();
    }
#endif

    // 确保 ESP8266 不处于硬件复位状态（RST 低有效，必须拉高）
#ifdef ESP8266_RST_Pin
    HAL_GPIO_WritePin(ESP8266_RST_GPIO_Port, ESP8266_RST_Pin, GPIO_PIN_SET);
#endif

    // 初始化 DSP 库 (FFT)
    if (!fft_initialized)
    {
        arm_rfft_fast_init_f32(&S, WAVEFORM_POINTS);
        fft_initialized = 1;
    }

    ESP_Log("\r\n[ESP] 初始化（4通道模式）...\r\n");
    ESP_Log("[ESP] WiFi 名称(SSID): %s\r\n", WIFI_SSID);
    ESP_Log("[ESP] 服务器地址: %s:%d\r\n", SERVER_IP, SERVER_PORT);
    ESP_Clear_Error_Flags();

    // 步骤 3: 优先尝试复用“透传 + TCP”现有连接
    // 场景：MCU 只是软复位，而 ESP8266 还在透传模式且连接正常，此时无需断电重连，直接发包即可
    if (ESP_TryReuseTransparent())
    {
        g_esp_ready = 1;
        g_uart2_at_mode = 0;
        ESP_Log("[ESP] 复用透传连接成功（无需断电可重连）\r\n");
        return;
    }

    // 步骤 4: 复用失败：必须先确保回到 AT 命令模式
    // 注意：ESP_Send_Cmd 的超时本质不是“等待不够”，而是“没收到任何字节”。
    // 需要先尝试发送 "+++" 退出透传，并确认能收到 "OK"。
    ESP_Uart2_Drain(120);
    uint8_t at_ok = 0;
    for (int k = 0; k < 3; k++)
    {
        if (ESP_Send_Cmd("AT\r\n", "OK", 800))
        {
            at_ok = 1;
            break;
        }
        (void)ESP_Exit_Transparent_Mode_Strict(2000); // 尝试发送 +++ 退出透传
        ESP_Uart2_Drain(120);
    }

    // 如果还是进不去 AT 模式，执行硬复位 (硬件重启模组)
    if (!at_ok)
    {
        ESP_Log("[ESP] AT无回显，执行硬复位ESP8266...\r\n");
        ESP_HardReset();
        // 硬复位后，HAL 库状态可能被中断打断，再次确保处于 AT(阻塞)可用状态
        ESP_ForceStop_DMA();

        // 复位后再探测一次 AT
        if (!ESP_Send_Cmd("AT\r\n", "OK", 1500))
        {
            ESP_Log("[ESP] 致命：硬复位后仍无法进入 AT 模式\r\n");
            return;
        }
    }

    /* 复位流程 */
    ESP_Log("[ESP 指令] >> AT+RST\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t *)"AT+RST\r\n", 8, 100);
    HAL_Delay(3500); // 等待模组启动日志打印完成
    ESP_Clear_Error_Flags();

    // 关闭回显 (ATE0)，方便后续指令解析
    while (!ESP_Send_Cmd("ATE0\r\n", "OK", 500))
    {
        ESP_Log("[ESP] 关闭回显失败，重试 ATE0...\r\n");
        ESP_Clear_Error_Flags();
        HAL_Delay(500);
        retry_count++;
        if (retry_count > 5)
            break;
    }

    // 设置 Station 模式
    ESP_Send_Cmd("AT+CWMODE=1\r\n", "OK", 1000);

    // 连接 WiFi
    sprintf(cmd_buf, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
    if (!ESP_Send_Cmd(cmd_buf, "GOT IP", 20000)) // 给足 20s 超时
    {
        // 重试一次
        if (!ESP_Send_Cmd(cmd_buf, "GOT IP", 20000))
        {
            ESP_Log("[ESP] WiFi 连接失败。\r\n");
            ESP_Log_RxBuf("WIFI_FAIL");
            return;
        }
    }
    ESP_Log("[ESP] WiFi 连接成功（已获取 IP）。\r\n");
    // 打印 STA IP 信息（便于确认是否进到目标网段）
    ESP_Send_Cmd("AT+CIFSR\r\n", "OK", 2000);
    ESP_Log_RxBuf("CIFSR");

    /* TCP 连接 */
    ESP_Send_Cmd("AT+CIPCLOSE\r\n", "OK", 500); // 先关闭可能存在的旧连接
    sprintf(cmd_buf, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", SERVER_IP, SERVER_PORT);
    if (!ESP_Send_Cmd(cmd_buf, "CONNECT", 10000))
    {
        // 如果返回 ALREADY，说明连接还在，也算成功
        if (strstr((char *)esp_rx_buf, "ALREADY") == NULL)
        {
            ESP_Log("[ESP] TCP 连接失败。\r\n");
            ESP_Log_RxBuf("TCP_FAIL");
            return;
        }
    }
    ESP_Log("[ESP] TCP 连接成功（CONNECT）。\r\n");
    ESP_Send_Cmd("AT+CIPSTATUS\r\n", "OK", 1000);
    ESP_Log_RxBuf("CIPSTATUS");

    // 开启透传模式 (UART <-> WiFi 透明传输)
    ESP_Send_Cmd("AT+CIPMODE=1\r\n", "OK", 1000);
    ESP_Send_Cmd("AT+CIPSEND\r\n", ">", 2000); // 等待出现 '>' 符号
    HAL_Delay(500);

    // 发送注册包 (告诉服务器我是谁)
    ESP_Register();
    g_esp_ready = 1;
    ESP_Log("[ESP] 系统就绪（4通道），开始上报数据。\r\n");

    // 步骤 5: 启动 USART2 接收（DMA + IDLE）
    // 用于接收 /api/node/heartbeat 的响应中的 command/reset
    // 此时正式退出 AT 模式，进入数据监听模式
    g_uart2_at_mode = 0;
    ESP_StreamRx_Start();

    /* === 初始化 4 个通道的元数据 (与 api.py 逻辑严格对应) === */
    // Channel 0: 直流母线(+) -> api.py 识别 "直流" 且无 "负/-" -> voltage
    node_channels[0].id = 0;
    strncpy(node_channels[0].label, "直流母线(+)", 31);
    strncpy(node_channels[0].unit, "V", 7);

    // Channel 1: 直流母线(-) -> api.py 识别 "直流" 且有 "负/-" -> voltage_neg
    node_channels[1].id = 1;
    strncpy(node_channels[1].label, "直流母线(-)", 31);
    strncpy(node_channels[1].unit, "V", 7);

    // Channel 2: 负载电流 -> api.py 识别 "负载" 或 "电流" -> current
    node_channels[2].id = 2;
    strncpy(node_channels[2].label, "负载电流", 31);
    strncpy(node_channels[2].unit, "A", 7);

    // Channel 3: 漏电流 -> api.py 识别 "漏" -> leakage
    node_channels[3].id = 3;
    strncpy(node_channels[3].label, "漏电流", 31);
    strncpy(node_channels[3].unit, "mA", 7);
}

/* 通用波形生成与FFT计算函数 */
static void Process_Channel_Data(int ch_id, float base_dc, float ripple_amp, float noise_level)
{
    static float time_t = 0.0f;
    float noise;
    int i;

    // 模拟瞬时值波动 (低频慢变化)
    node_channels[ch_id].current_value = base_dc + ripple_amp * 0.1f * arm_sin_f32(2.0f * PI * 0.5f * time_t);

    // 1. 生成波形 (50Hz + 150Hz谐波 + 噪声)
    for (i = 0; i < WAVEFORM_POINTS; i++)
    {
        // 基础噪声
        noise = ((float)(rand() % 100) / 100.0f - 0.5f) * noise_level;

        // 漏电流特殊处理：加一些随机尖峰
        if (ch_id == 3 && (rand() % 100) > 98)
        {
            noise += 5.0f; // 突发漏电尖峰
        }

        // 信号组合：直流基准 + 50Hz纹波 + 150Hz谐波 + 噪声
        float phase = (float)ch_id * 0.5f; // 不同通道相位错开

        node_channels[ch_id].waveform[i] = node_channels[ch_id].current_value + ripple_amp * arm_sin_f32(2.0f * PI * 50.0f * ((float)i * 0.0005f) + phase) + (ripple_amp * 0.3f) * arm_sin_f32(2.0f * PI * 150.0f * ((float)i * 0.0005f)) + noise;
    }

    // 2. FFT 计算 (使用 CMSIS-DSP 库)
    arm_rfft_fast_f32(&S, node_channels[ch_id].waveform, fft_output_buf, 0);
    arm_cmplx_mag_f32(fft_output_buf, fft_mag_buf, WAVEFORM_POINTS / 2);

    // 3. 填充 FFT (归一化处理)
    node_channels[ch_id].fft_data[0] = 0; // 去直流分量
    for (i = 1; i < FFT_POINTS; i++)
    {
        // 归一化公式：幅度 = (FFT模值 / (N/2)) * 2
        node_channels[ch_id].fft_data[i] = (fft_mag_buf[i] / (float)(WAVEFORM_POINTS / 2)) * 2.0f;
    }

    // 4. 恢复波形 (因为 arm_rfft_fast_f32 是 In-Place 运算，会破坏原 buffer)
    // 这里为了 JSON 发送原始波形，需要重新生成一遍
    for (i = 0; i < WAVEFORM_POINTS; i++)
    {
        float phase = (float)ch_id * 0.5f;
        node_channels[ch_id].waveform[i] = node_channels[ch_id].current_value + ripple_amp * arm_sin_f32(2.0f * PI * 50.0f * ((float)i * 0.0005f) + phase) + (ripple_amp * 0.3f) * arm_sin_f32(2.0f * PI * 150.0f * ((float)i * 0.0005f));
    }

    if (ch_id == 3)
        time_t += 0.05f; // 时间步进
}

void ESP_Update_Data_And_FFT(void)
{
    // 更新 4 个通道的数据
    // Ch0: 母线+ (375V, 纹波5V, 噪声2V)
    Process_Channel_Data(0, 375.0f, 5.0f, 2.0f);

    // Ch1: 母线- (375V, 纹波5V, 噪声2V)
    Process_Channel_Data(1, 375.0f, 5.0f, 2.0f);

    // Ch2: 负载电流 (12.5A, 纹波3A, 噪声0.5A)
    Process_Channel_Data(2, 12.5f, 3.0f, 0.5f);

    // Ch3: 漏电流 (20mA, 纹波5mA, 噪声2mA)
    Process_Channel_Data(3, 20.0f, 5.0f, 2.0f);
}

static void StrTrimInPlace(char *s)
{
    if (!s)
        return;
    // trim left
    char *p = s;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        p++;
    if (p != s)
        memmove(s, p, strlen(p) + 1);
    // trim right
    size_t n = strlen(s);
    while (n > 0)
    {
        char c = s[n - 1];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        {
            s[n - 1] = 0;
            n--;
        }
        else
            break;
    }
}

static void ESP_SetFaultCode(const char *code)
{
    if (!code)
        return;
    char c0 = code[0];
    char c1 = code[1];
    char c2 = code[2];
    if (c0 == 'e')
        c0 = 'E'; // 兼容小写
    if (c0 != 'E')
        return;
    if (c1 < '0' || c1 > '9')
        return;
    if (c2 < '0' || c2 > '9')
        return;
    g_fault_code[0] = 'E';
    g_fault_code[1] = c1;
    g_fault_code[2] = c2;
    g_fault_code[3] = 0;
    ESP_Log("[控制台] 故障码已切换为: %s（下一次上报生效）\r\n", g_fault_code);
}

static void ESP_Console_HandleLine(char *line)
{
    if (!line)
        return;
    StrTrimInPlace(line);
    if (line[0] == 0)
        return;

    if (strcmp(line, "help") == 0 || strcmp(line, "?") == 0)
    {
        ESP_Log("[控制台] 可用命令：\r\n");
        ESP_Log("  - E00/E01/E02... ：切换上报故障码\r\n");
        ESP_Log("  - help 或 ?      ：显示帮助\r\n");
        return;
    }

    // 格式: E01
    if ((line[0] == 'E' || line[0] == 'e') && strlen(line) == 3)
    {
        ESP_SetFaultCode(line);
        return;
    }

    // 格式: fault E01
    if (strncmp(line, "fault", 5) == 0)
    {
        char *p = line + 5;
        while (*p == ' ' || *p == '\t')
            p++;
        if (strlen(p) == 3 && (p[0] == 'E' || p[0] == 'e'))
        {
            ESP_SetFaultCode(p);
            return;
        }
    }

    ESP_Log("[控制台] 未识别命令: %s（输入 help 查看帮助）\r\n", line);
}

void ESP_Console_Init(void)
{
#if (ESP_CONSOLE_ENABLE)
    UART_HandleTypeDef *hu = ESP_GetLogUart(); // 默认 USART1
    if (!hu)
        return;

    // 启动 1 字节 RX 中断（比主循环轮询稳定，避免“输入了但没反应”）
    HAL_UART_Receive_IT(hu, &g_console_rx_byte, 1);
    ESP_Log("[控制台] 已启用：输入 E01 回车注入故障；输入 E00 回车清除。\r\n");
#endif
}

/**
 * @brief  控制台轮询任务
 * @note   在主循环中调用，处理用户输入和服务器指令
 */
void ESP_Console_Poll(void)
{
#if (ESP_CONSOLE_ENABLE)
    // 1) 处理“已收到的一整行”控制台指令
    if (g_console_line_ready)
    {
        g_console_line_ready = 0;
        g_console_line[g_console_line_len] = 0;
        ESP_Console_HandleLine(g_console_line);
        g_console_line_len = 0;
    }

    // 2) 处理“服务器下发 reset”指令
    if (g_server_reset_pending)
    {
        g_server_reset_pending = 0;
        ESP_Log("[服务器命令] 收到 reset：清除故障码 -> E00\r\n");
        ESP_SetFaultCode("E00");
    }

#if (ESP_DEBUG)
    // 3) 调试：每 1 秒输出一次统计信息 (确认 RX 是否存活)
    static uint32_t last_dbg = 0;
    uint32_t now = HAL_GetTick();
    if ((now - last_dbg) >= 1000)
    {
        last_dbg = now;
        ESP_Log("[调试] USART2 RX: started=%d, events=%lu, bytes=%lu, restart=%lu\r\n",
                (int)g_usart2_rx_started,
                (unsigned long)g_usart2_rx_events,
                (unsigned long)g_usart2_rx_bytes,
                (unsigned long)g_usart2_rx_restart);
        ESP_Log("[调试] USART2 ERR: total=%lu, ORE=%lu, FE=%lu, NE=%lu, PE=%lu\r\n",
                (unsigned long)g_uart2_err,
                (unsigned long)g_uart2_err_ore,
                (unsigned long)g_uart2_err_fe,
                (unsigned long)g_uart2_err_ne,
                (unsigned long)g_uart2_err_pe);
    }
#endif

    // 4) 链路自恢复：如果长时间收不到服务器任何响应，直接硬复位并重连
    // 这是为了应对极端情况下 TCP 假死或模组死机
    static uint32_t last_link_check = 0;
    static uint8_t no_rx_miss = 0;
    static uint32_t last_rx_bytes_snapshot = 0;
    if ((now - last_link_check) >= 1000)
    {
        last_link_check = now;
        // 只有当“已连接”、“非重连中”且“非AT模式”时才检查
        if (g_esp_ready && !g_link_reconnecting && !g_uart2_at_mode)
        {
            // 正常情况下服务器会对每次心跳返回 HTTP 响应
            uint32_t no_rx_ms = (uint32_t)ESP_NO_SERVER_RX_HARDRESET_SEC * 1000u;
            if (no_rx_ms < 30000u)
                no_rx_ms = 30000u; // 最小阈值 30s

            // 如果 RX 字节数在增长，说明链路正常
            if (g_usart2_rx_bytes != last_rx_bytes_snapshot)
            {
                last_rx_bytes_snapshot = g_usart2_rx_bytes;
                g_last_rx_tick = now;
                no_rx_miss = 0;
            }
            // 否则检查是否超时
            else if (g_last_rx_tick != 0 && (now - g_last_rx_tick) > no_rx_ms)
            {
                no_rx_miss++;
                // 连续 3 次检测都超时，才判定为断线
                if (no_rx_miss >= 3)
                {
                    ESP_Log("[ESP] 警告：%lus 无服务器响应(Δt=%lums, miss=%u) -> 硬复位重连\r\n",
                            (unsigned long)(no_rx_ms / 1000u),
                            (unsigned long)(now - g_last_rx_tick),
                            (unsigned)no_rx_miss);
                    no_rx_miss = 0;
                    g_link_reconnecting = 1;
                    ESP_HardReset();
                    g_link_reconnecting = 0;
                    ESP_Init();
                }
            }
        }
    }
#else
    (void)0;
#endif
}

/**
 * @brief  数据发送主函数
 * @note   负责打包 JSON，通过 DMA 发送
 */
void ESP_Post_Data(void)
{
    if (g_esp_ready == 0)
        return;

    // 流量控制（发送门控）
    // 如果上一次请求的 HTTP 响应还没回来，不要继续堆请求，防止服务器解析失败。
    if (g_waiting_http_response)
    {
        uint32_t now = HAL_GetTick();
        // 关键修复：压力测试不能因为“没识别到 HTTP/1.1”就完全断流。
        // 将超时时间设置为 250ms (2Mbps 传输16KB约80ms + 服务器处理)
        // 如果超过 250ms 还没回包，大概率是丢包了，直接放行发下一包。
        if (g_waiting_http_tick != 0 && (now - g_waiting_http_tick) > 250u)
        {
#if (ESP_DEBUG)
            ESP_Log("[调试] HTTP 门控超时(>250ms)，放行继续发送\r\n");
#endif
            g_waiting_http_response = 0;
        }
        else
        {
            return; // 还在等，跳过本次发送
        }
    }

    // 5Hz 发送频率限制
    static uint32_t last_send_time = 0;
    if (HAL_GetTick() - last_send_time < 200)
        return;
    last_send_time = HAL_GetTick();

    // 开始构建 JSON
    char *p = (char *)http_packet_buf;
    uint32_t body_len;
    uint32_t header_reserve_len = 256;
    int header_len;
    uint32_t total_len;

    // JSON Header
    p += sprintf(p, "{\"node_id\": \"%s\",\"status\": \"online\",\"fault_code\": \"%s\",\"channels\": [", NODE_ID, g_fault_code);

    // 循环写入 4 个通道的数据
    for (int i = 0; i < 4; i++)
    {
        p += sprintf(p, "{"
                        "\"id\": %d, \"channel_id\": %d,"
                        "\"label\": \"%s\", \"name\": \"%s\","
                        "\"value\": %.2f, \"current_value\": %.2f,"
                        "\"unit\": \"%s\","
                        "\"waveform\": [",
                     node_channels[i].id, node_channels[i].id,
                     node_channels[i].label, node_channels[i].label, // name冗余label
                     node_channels[i].current_value, node_channels[i].current_value,
                     node_channels[i].unit);

        // 波形数据 (STEP=4, 发256点)
        Helper_FloatArray_To_String(p, node_channels[i].waveform, WAVEFORM_POINTS, WAVEFORM_SEND_STEP);
        p += strlen(p);

        // 频谱数据
        p += sprintf(p, "], \"fft_spectrum\": [");
        Helper_FloatArray_To_String(p, node_channels[i].fft_data, FFT_POINTS, 1);
        p += strlen(p);

        // 结束当前 channel
        p += sprintf(p, "]}");
        if (i < 3)
            p += sprintf(p, ","); // 逗号分隔
    }

    p += sprintf(p, "]}"); // JSON End

    body_len = (uint32_t)(p - (char *)http_packet_buf);

    // 拼接 HTTP Header
    // 先把 Body 往后挪，腾出 Header 空间
    memmove(http_packet_buf + header_reserve_len, http_packet_buf, body_len);
    header_len = sprintf((char *)http_packet_buf,
                         "POST /api/node/heartbeat HTTP/1.1\r\n"
                         "Host: %s:%d\r\n"
                         "Content-Type: application/json\r\n"
                         "Content-Length: %u\r\n"
                         "\r\n",
                         SERVER_IP, SERVER_PORT, body_len);
    // 把 Body 接回来
    memmove(http_packet_buf + header_len, http_packet_buf + header_reserve_len, body_len);
    total_len = (uint32_t)header_len + body_len;

    // 统计发送状态
    static uint32_t tx_try = 0, tx_ok = 0, tx_busy = 0, tx_err = 0;
    static uint32_t last_tx_log = 0;
    tx_try++;

    // ⚠️ 关键步骤：DMA 发送前必须 Clean DCache
    // 确保 CPU 写入缓冲区的最新 JSON 数据被刷入物理内存，DMA 才能搬运正确的数据。
    DCache_CleanByAddr_Any(http_packet_buf, total_len);

    // 启动 DMA 发送
    HAL_StatusTypeDef st = HAL_UART_Transmit_DMA(&huart2, (uint8_t *)http_packet_buf, (uint16_t)total_len);
    if (st == HAL_OK)
        tx_ok++;
    else if (st == HAL_BUSY)
        tx_busy++;
    else
        tx_err++;

    if (st == HAL_OK)
    {
        // 标记开始等待回包
        g_waiting_http_response = 1;
        g_waiting_http_tick = HAL_GetTick();
    }

#if (ESP_DEBUG)
    // 调试日志
    uint32_t now = HAL_GetTick();
    if ((now - last_tx_log) >= 1000)
    {
        last_tx_log = now;
        ESP_Log("[调试] TX: try=%lu ok=%lu busy=%lu err=%lu len=%lu gState=%d rxStarted=%d\r\n",
                (unsigned long)tx_try, (unsigned long)tx_ok, (unsigned long)tx_busy, (unsigned long)tx_err,
                (unsigned long)total_len,
                (int)huart2.gState,
                (int)g_usart2_rx_started);
    }
#endif
}

// ---------------- 串口 RX 回调：调试串口输入 E01/E00 注入/清除故障 ----------------
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
#if (ESP_CONSOLE_ENABLE)
    UART_HandleTypeDef *hu = ESP_GetLogUart();
    if (huart == hu)
    {
        // 若上一条命令还未被主循环处理，为避免覆盖缓冲区，先丢弃后续输入
        if (g_console_line_ready)
        {
            HAL_UART_Receive_IT(hu, &g_console_rx_byte, 1);
            return;
        }

        uint8_t ch = g_console_rx_byte;
        if (ch == '\r' || ch == '\n')
        {
            if (g_console_line_len > 0)
            {
                g_console_line_ready = 1;
            }
        }
        else
        {
            if (g_console_line_len < (sizeof(g_console_line) - 1))
            {
                g_console_line[g_console_line_len++] = (char)ch;
            }
            else
            {
                g_console_line_len = 0; // 溢出清空
            }
        }
        HAL_UART_Receive_IT(hu, &g_console_rx_byte, 1);
        return;
    }
#endif
    (void)huart;
}

// ---------------- USART2 流式接收：解析后端 /api/node/heartbeat 响应里的 command=reset ----------------
static void ESP_StreamRx_Start(void)
{
    memset(g_stream_rx_buf, 0, sizeof(g_stream_rx_buf));
    memset(g_stream_window, 0, sizeof(g_stream_window));
    g_stream_window_len = 0;

    // 确保 RX 状态干净（避免因为之前的阻塞接收/异常导致启动失败）
    (void)HAL_UART_AbortReceive(&huart2);

    // 启动 IDLE DMA 接收
    HAL_StatusTypeDef st = HAL_UARTEx_ReceiveToIdle_DMA(&huart2, g_stream_rx_buf, sizeof(g_stream_rx_buf));
    if (st == HAL_OK)
    {
        g_usart2_rx_started = 1;
        g_usart2_rx_restart++;
        g_last_rx_tick = HAL_GetTick();
        // 关闭半传输中断 (HT)，减少一半的中断频率，这对 2Mbps 通信至关重要
        if (huart2.hdmarx)
        {
            __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
        }
        ESP_Log("[ESP] 已开启 USART2 RX(DMA+IDLE)：用于接收服务器下发命令\r\n");
    }
    else
    {
        g_usart2_rx_started = 0;
        ESP_Log("[ESP] 开启 USART2 RX(DMA+IDLE) 失败，status=%d（将无法接收 reset 命令）\r\n", (int)st);
    }
}

static void ESP_StreamRx_Feed(const uint8_t *data, uint16_t len)
{
    if (!data || len == 0)
        return;
    g_usart2_rx_bytes += len;

    // ---------------- 关键修复：先扫描“原始数据块” ----------------
    // 避免因为滑动窗口截断（例如 "HTTP" 和 "/1.1" 在两包里）导致漏判

    // 1) HTTP 响应检测：只要任意数据块里出现 HTTP/1.1，就认为已收到本次请求的回包（解除门控）
    if (g_waiting_http_response && ESP_BufContains(data, len, "HTTP/1.1"))
    {
        g_waiting_http_response = 0;
        g_last_rx_tick = HAL_GetTick();
    }
    // 2) 服务器命令检测：优先在原始数据里扫一遍
    if (ESP_BufContains(data, len, "\"command\"") && ESP_BufContains(data, len, "reset"))
    {
        g_server_reset_pending = 1;
    }

    // ---------------- 滑动窗口逻辑 ----------------
    // 将新数据追加到窗口，移除旧数据，保持窗口大小恒定。
    // 用于解决关键字跨包的问题。
    const uint16_t cap = (uint16_t)(sizeof(g_stream_window) - 1);
    if (len >= cap)
    {
        // 如果新数据比窗口大，直接保留新数据的最后 cap 字节
        memcpy(g_stream_window, data + (len - cap), cap);
        g_stream_window[cap] = 0;
        g_stream_window_len = cap;
    }
    else
    {
        uint16_t new_len = (uint16_t)(g_stream_window_len + len);
        if (new_len > cap)
        {
            uint16_t drop = (uint16_t)(new_len - cap);
            if (drop >= g_stream_window_len)
            {
                g_stream_window_len = 0;
            }
            else
            {
                // 移动旧数据
                memmove(g_stream_window, g_stream_window + drop, g_stream_window_len - drop);
                g_stream_window_len = (uint16_t)(g_stream_window_len - drop);
            }
        }
        // 追加新数据
        memcpy(g_stream_window + g_stream_window_len, data, len);
        g_stream_window_len = (uint16_t)(g_stream_window_len + len);
        g_stream_window[g_stream_window_len] = 0;
    }

    // 识别 JSON：{"command":"reset"}（也允许出现 reset 关键字）
    if (strstr(g_stream_window, "\"command\"") && strstr(g_stream_window, "reset"))
    {
        g_server_reset_pending = 1;
    }

    // 识别 HTTP 响应头
    if (g_waiting_http_response && strstr(g_stream_window, "HTTP/1.1"))
    {
        g_waiting_http_response = 0;
        g_last_rx_tick = HAL_GetTick();
    }
}

/**
 * @brief  UART 接收事件回调 (DMA 满 或 IDLE 空闲时触发)
 * @note   HAL_UARTEx_ReceiveToIdle_DMA 的回调
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == &huart2)
    {
        // AT 阶段禁止 DMA+IDLE 回调介入（会与阻塞式 HAL_UART_Receive 冲突）
        if (g_uart2_at_mode)
        {
            return;
        }
        g_usart2_rx_events++;
        if (Size > 0 && Size <= sizeof(g_stream_rx_buf))
        {
            uint32_t now = HAL_GetTick();
            g_last_rx_tick = now;

            // 只要收到了任何回包字节，就认为本次请求已有响应
            if (g_waiting_http_response)
            {
                g_waiting_http_response = 0;
            }

            // ⚠️ 关键：DMA 接收后 Invalidate DCache
            // 确保 CPU 从 RAM 读取最新数据，而不是 Cache 中的旧值
            DCache_InvalidateByAddr_Any(g_stream_rx_buf, Size);

            // 解析数据
            ESP_StreamRx_Feed(g_stream_rx_buf, Size);
        }

        // 重新启动 RX（若异常，先 Abort 再重启）
        HAL_StatusTypeDef st = HAL_UARTEx_ReceiveToIdle_DMA(&huart2, g_stream_rx_buf, sizeof(g_stream_rx_buf));
        if (st != HAL_OK)
        {
            (void)HAL_UART_AbortReceive(&huart2);
            st = HAL_UARTEx_ReceiveToIdle_DMA(&huart2, g_stream_rx_buf, sizeof(g_stream_rx_buf));
        }
        if (st == HAL_OK)
        {
            g_usart2_rx_started = 1;
            if (huart2.hdmarx)
            {
                __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
            }
        }
        else
        {
            g_usart2_rx_started = 0;
        }
        return;
    }
}

/**
 * @brief  UART 错误回调
 * @note   典型是 ORE（接收溢出）导致后续不再进 RxEventCallback。
 * 这是 2Mbps 无流控通信中最关键的容错逻辑。
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart2)
    {
        g_uart2_err++;
        uint32_t ec = huart->ErrorCode;
        if (ec & HAL_UART_ERROR_ORE)
            g_uart2_err_ore++;
        if (ec & HAL_UART_ERROR_FE)
            g_uart2_err_fe++;
        if (ec & HAL_UART_ERROR_NE)
            g_uart2_err_ne++;
        if (ec & HAL_UART_ERROR_PE)
            g_uart2_err_pe++;

        // 发生错误时，当前这次 HTTP 回包很可能已经丢了；不要继续门控傻等
        if (g_waiting_http_response)
        {
            g_waiting_http_response = 0;
        }

        // 场景 A: AT(阻塞)阶段
        // 如果此时去重启 ReceiveToIdle_DMA，会把 HAL 的 RxState 锁死为 BUSY_RX，
        // 导致后续 HAL_UART_Receive 直接 HAL_BUSY，从而出现“无限超时/假死”。
        if (g_uart2_at_mode)
        {
            ESP_Clear_Error_Flags();
            (void)HAL_UART_Abort(&huart2);
            g_usart2_rx_started = 0;
            return;
        }

        // 场景 B: 透传阶段
        // 清除错误并重启 DMA 接收，确保能继续收数据
        ESP_Clear_Error_Flags();
        (void)HAL_UART_AbortReceive(&huart2);

        HAL_StatusTypeDef st = HAL_UARTEx_ReceiveToIdle_DMA(&huart2, g_stream_rx_buf, sizeof(g_stream_rx_buf));
        g_usart2_rx_started = (st == HAL_OK) ? 1 : 0;

        if (st == HAL_OK && huart2.hdmarx)
        {
            __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
        }
        return;
    }
}

void ESP_Register(void)
{
    char *body_start = (char *)http_packet_buf + 256;
    ESP_Log("[ESP] 正在注册设备...\r\n");
    sprintf(body_start, "{\"device_id\":\"%s\",\"location\":\"%s\",\"hw_version\":\"v1.0_4CH\"}", NODE_ID, NODE_LOCATION);
    uint32_t body_len = strlen(body_start);
    int h_len = sprintf((char *)http_packet_buf,
                        "POST /api/register HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: application/json\r\nContent-Length: %u\r\n\r\n",
                        SERVER_IP, SERVER_PORT, body_len);
    memmove(http_packet_buf + h_len, body_start, body_len);
    HAL_UART_Transmit(&huart2, http_packet_buf, h_len + body_len, 1000);

    // 关键：读一下服务器 HTTP 响应，确认注册是否真的到达后端
    memset(esp_rx_buf, 0, sizeof(esp_rx_buf));
    uint32_t start = HAL_GetTick();
    uint16_t idx = 0;
    while ((HAL_GetTick() - start) < 800)
    {
        uint8_t ch;
        if (HAL_UART_Receive(&huart2, &ch, 1, 5) == HAL_OK)
        {
            if (idx < sizeof(esp_rx_buf) - 1)
            {
                esp_rx_buf[idx++] = ch;
                esp_rx_buf[idx] = 0;
            }
            if (strstr((char *)esp_rx_buf, "HTTP/1.1"))
            {
                ESP_Log("[ESP] 注册响应已收到\r\n");
                break;
            }
        }
    }
    if (idx == 0)
    {
        ESP_Log("[ESP] 注册无响应（未收到HTTP头）\r\n");
    }
    HAL_Delay(200);
}

/* 辅助函数 */

// 丢弃 UART2 残留输出（避免污染关键字匹配）
static void ESP_Uart2_Drain(uint32_t ms)
{
    uint32_t start = HAL_GetTick();
    uint8_t ch;
    while ((HAL_GetTick() - start) < ms)
    {
        if (HAL_UART_Receive(&huart2, &ch, 1, 0) == HAL_OK)
        {
            // eat
        }
        else
        {
            HAL_Delay(1);
        }
    }
}

// 阻塞等待关键字（用于透传复用探测 / +++ 等待 OK）
static uint8_t ESP_Wait_Keyword(const char *kw, uint32_t timeout_ms)
{
    if (!kw || !*kw)
        return 0;
    uint32_t start = HAL_GetTick();
    uint16_t idx = 0;
    memset(esp_rx_buf, 0, sizeof(esp_rx_buf));
    while ((HAL_GetTick() - start) < timeout_ms)
    {
        uint8_t ch;
        if (HAL_UART_Receive(&huart2, &ch, 1, 5) == HAL_OK)
        {
            if (idx < sizeof(esp_rx_buf) - 1)
            {
                esp_rx_buf[idx++] = ch;
                esp_rx_buf[idx] = 0;
            }
            if (strstr((char *)esp_rx_buf, kw))
                return 1;
        }
    }
    return 0;
}

// 严格退出透传：满足 guard time，发送 +++，并等待 OK
static uint8_t ESP_Exit_Transparent_Mode_Strict(uint32_t timeout_ms)
{
    // 停 RX DMA，避免与阻塞式接收冲突
    (void)HAL_UART_AbortReceive(&huart2);
    ESP_Clear_Error_Flags();

    // guard time (before)：期间不要向模块发送任何字节 (通常需 > 1s)
    HAL_Delay(1200);
    HAL_UART_Transmit(&huart2, (uint8_t *)"+++", 3, 100);
    // guard time (after)
    HAL_Delay(1200);

    // 等待 OK（有些固件会返回 "OK\r\n"）
    return ESP_Wait_Keyword("OK", timeout_ms);
}

// MCU 复位后：优先复用“现有透传+TCP”连接（无需断电 ESP）
static uint8_t ESP_TryReuseTransparent(void)
{
    // 如果当前就在命令模式，AT 会立刻 OK；这种情况不需要复用，走常规初始化更稳
    if (ESP_Send_Cmd("AT\r\n", "OK", 200))
        return 0;

    // 在透传里：直接发一包最小 heartbeat 探测，看是否收到 HTTP/1.1
    (void)HAL_UART_AbortReceive(&huart2);
    ESP_Uart2_Drain(100);

    char *body = (char *)http_packet_buf + 256;
    sprintf(body, "{\"node_id\":\"%s\",\"status\":\"online\",\"fault_code\":\"%s\"}", NODE_ID, g_fault_code);
    uint32_t body_len = (uint32_t)strlen(body);
    int h_len = sprintf((char *)http_packet_buf,
                        "POST /api/node/heartbeat HTTP/1.1\r\n"
                        "Host: %s:%d\r\n"
                        "Content-Type: application/json\r\n"
                        "Content-Length: %lu\r\n"
                        "\r\n",
                        SERVER_IP, SERVER_PORT, (unsigned long)body_len);
    memmove(http_packet_buf + h_len, body, body_len);

    // 探测包小，阻塞发送即可
    HAL_UART_Transmit(&huart2, http_packet_buf, (uint16_t)(h_len + body_len), 500);
    if (ESP_Wait_Keyword("HTTP/1.1", 800))
    {
        // 复用成功：重启 RX DMA，用于接收 reset 指令 & 更新时间戳
        ESP_StreamRx_Start();
        g_last_rx_tick = HAL_GetTick();
        return 1;
    }
    return 0;
}

// 软重连：不断电、不重置 WiFi，只重建 TCP + 透传
static void ESP_SoftReconnect(void)
{
    if (g_link_reconnecting)
        return;
    g_link_reconnecting = 1;

    // 暂停一段时间，满足 guard time 才能退出透传
    g_esp_ready = 0;
    (void)HAL_UART_AbortReceive(&huart2);

    if (!ESP_Exit_Transparent_Mode_Strict(2000))
    {
        ESP_Log("[ESP] 软重连失败：无法退出透传 -> 硬复位ESP8266\r\n");
        g_link_reconnecting = 0;
        ESP_HardReset();
        // 复位后走完整初始化（会重建 WiFi/TCP/透传）
        ESP_Init();
        return;
    }

    // 重建 TCP 并重新进入透传
    ESP_Send_Cmd("AT+CIPCLOSE\r\n", "OK", 1500);
    char cmd_buf[128];
    sprintf(cmd_buf, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", SERVER_IP, SERVER_PORT);
    if (!ESP_Send_Cmd(cmd_buf, "CONNECT", 10000))
    {
        ESP_Log("[ESP] 软重连失败：CIPSTART -> 硬复位ESP8266\r\n");
        g_link_reconnecting = 0;
        ESP_HardReset();
        ESP_Init();
        return;
    }
    ESP_Send_Cmd("AT+CIPMODE=1\r\n", "OK", 1000);
    ESP_Send_Cmd("AT+CIPSEND\r\n", ">", 2000);
    HAL_Delay(200);

    ESP_StreamRx_Start();
    g_last_rx_tick = HAL_GetTick();
    g_esp_ready = 1;
    g_link_reconnecting = 0;
    ESP_Log("[ESP] 软重连完成\r\n");
}

static void ESP_Clear_Error_Flags(void)
{
    volatile uint32_t isr = huart2.Instance->ISR;
    volatile uint32_t rdr = huart2.Instance->RDR;
    (void)isr;
    (void)rdr;
    // 仅清除错误标志，不操作数据
    __HAL_UART_CLEAR_OREFLAG(&huart2);
    __HAL_UART_CLEAR_NEFLAG(&huart2);
    __HAL_UART_CLEAR_FEFLAG(&huart2);
}

// 硬件复位 ESP8266：拉低 RST 一段时间再拉高（不需要断电）
static void ESP_HardReset(void)
{
#ifdef ESP8266_RST_Pin
    // 硬复位前先停掉 UART2 DMA/中断，避免复位过程中输出乱码引发中断风暴
    g_uart2_at_mode = 1;
    ESP_ForceStop_DMA();

    // RST 低有效：低 120ms -> 高，等待启动完成
    HAL_GPIO_WritePin(ESP8266_RST_GPIO_Port, ESP8266_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(120);
    HAL_GPIO_WritePin(ESP8266_RST_GPIO_Port, ESP8266_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(1500);
    ESP_Clear_Error_Flags();
    ESP_Uart2_Drain(200);
#else
    // 若硬件未接 RST 引脚，只能提示用户断电
    ESP_Log("[ESP] 未定义 ESP8266_RST_Pin，无法硬复位（请接 RST/EN 到GPIO）\r\n");
#endif
}

static uint8_t ESP_Send_Cmd(const char *cmd, const char *reply, uint32_t timeout)
{
    uint32_t start;
    uint16_t idx = 0;
    ESP_Clear_Error_Flags();
    memset(esp_rx_buf, 0, sizeof(esp_rx_buf));

#if (ESP_DEBUG)
    // 打印命令（敏感信息脱敏）
    if (cmd && (strncmp(cmd, "AT+CWJAP=", 9) == 0))
    {
        ESP_Log("[ESP 指令] >> AT+CWJAP=\"%s\",\"******\"\r\n", WIFI_SSID);
    }
    else if (cmd)
    {
        ESP_Log("[ESP 指令] >> %s", cmd);
    }
#endif

    HAL_UART_Transmit(&huart2, (uint8_t *)cmd, strlen(cmd), 100);
    start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout)
    {
        uint8_t ch;
        // 轮询方式接收一个字节
        if (HAL_UART_Receive(&huart2, &ch, 1, 5) == HAL_OK)
        {
            if (idx < sizeof(esp_rx_buf) - 1)
            {
                esp_rx_buf[idx++] = ch;
                esp_rx_buf[idx] = 0;
                if (strstr((char *)esp_rx_buf, reply) != NULL)
                {
#if (ESP_DEBUG)
                    ESP_Log("[ESP 期望] << %s\r\n", reply);
#endif
                    return 1;
                }
            }
        }
    }
#if (ESP_DEBUG)
    ESP_Log("[ESP 超时] 等待关键字: %s\r\n", reply);
    ESP_Log_RxBuf("TIMEOUT");
#endif
    return 0;
}

static void Helper_FloatArray_To_String(char *dest, float *data, int count, int step)
{
    int i;
    for (i = 0; i < count; i += step)
    {
        dest += sprintf(dest, "%.1f", data[i]);
        if (i + step < count)
            *dest++ = ',';
    }
    *dest = 0;
}

static void ESP_Exit_Transparent_Mode(void)
{
    HAL_Delay(200);
    HAL_UART_Transmit(&huart2, (uint8_t *)"+++", 3, 100);
    HAL_Delay(1000);
}

static void ESP_Log(const char *format, ...)
{
    char log_buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    va_end(args);
#if (ESP_DEBUG)
    // 直接打到指定调试串口（避免 printf 重定向配置不一致导致“看不到日志”）
    UART_HandleTypeDef *hu = ESP_GetLogUart();
    if (hu)
    {
        HAL_UART_Transmit(hu, (uint8_t *)log_buf, (uint16_t)strlen(log_buf), 100);
    }
    else
    {
        printf("%s", log_buf);
    }
#else
    (void)log_buf;
#endif
}