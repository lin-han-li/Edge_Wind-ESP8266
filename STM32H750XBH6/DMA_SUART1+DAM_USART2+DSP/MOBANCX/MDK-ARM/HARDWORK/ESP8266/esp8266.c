#include "esp8266.h"
#include "usart.h"      
#include "arm_math.h"   
#include <stdio.h>
#include <string.h>
#include <stdlib.h>     
#include <stdarg.h>     

extern UART_HandleTypeDef huart2;

// ---------------- DMA Buffer 放置与 Cache 维护（STM32H7 必需） ----------------
// 说明：
// - DTCM(0x2000_0000) DMA 访问不到；必须把 USART2 DMA TX/RX 缓冲放到 AXI SRAM(0x2400_0000)
// - AXI SRAM 常见开启 DCache，DMA 发送前需要 Clean，DMA 接收后需要 Invalidate
// AC6(armclang) 不支持 __attribute__((zero_init))（那是 AC5 扩展），会报警告。
// 这里仅用 section + aligned；由于变量是未初始化的 static，全都会在启动时清零。
#define AXI_SRAM_SECTION  __attribute__((section(".axi_sram")))
#define DMA_ALIGN32       __attribute__((aligned(32)))

static inline uint32_t _align_down_32(uint32_t x) { return x & ~31u; }
static inline uint32_t _align_up_32(uint32_t x)   { return (x + 31u) & ~31u; }

static void DCache_CleanByAddr_Any(void *addr, uint32_t len)
{
#if defined (SCB_CleanDCache_by_Addr)
    uint32_t a = _align_down_32((uint32_t)addr);
    uint32_t end = _align_up_32(((uint32_t)addr) + len);
    SCB_CleanDCache_by_Addr((uint32_t *)a, (int32_t)(end - a));
#else
    (void)addr; (void)len;
#endif
}

static void DCache_InvalidateByAddr_Any(void *addr, uint32_t len)
{
#if defined (SCB_InvalidateDCache_by_Addr)
    uint32_t a = _align_down_32((uint32_t)addr);
    uint32_t end = _align_up_32(((uint32_t)addr) + len);
    SCB_InvalidateDCache_by_Addr((uint32_t *)a, (int32_t)(end - a));
#else
    (void)addr; (void)len;
#endif
}

/* ================= 内存分配 ================= */
/* ⚠️ 扩大到 48KB 以容纳 4 个通道的 JSON 数据 */
static uint8_t http_packet_buf[49152] AXI_SRAM_SECTION DMA_ALIGN32;
static uint8_t esp_rx_buf[512]; 

/* 4 个通道 */
Channel_Data_t node_channels[4]; 
volatile uint8_t g_esp_ready = 0;

/* DSP 相关 */
static arm_rfft_fast_instance_f32 S; 
static uint8_t fft_initialized = 0;
static float32_t fft_output_buf[WAVEFORM_POINTS]; 
static float32_t fft_mag_buf[WAVEFORM_POINTS]; 

/* 内部声明 */
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

// 当前上报的故障码（默认正常 E00）
static char g_fault_code[4] = "E00";

// ---------- 串口控制台（调试串口 RX 中断） ----------
static uint8_t g_console_rx_byte = 0;
static volatile uint8_t g_console_line_ready = 0;
static char g_console_line[32];
static volatile uint8_t g_console_line_len = 0;

// ---------- 服务器下发命令（从 USART2 收到的 HTTP 响应中提取） ----------
static volatile uint8_t g_server_reset_pending = 0;
// RX DMA 缓冲：加大以降低 IDLE 重启频率，减少 ORE/FE 风险（压力测试下回包会比较密集）
static uint8_t g_stream_rx_buf[4096] AXI_SRAM_SECTION DMA_ALIGN32;
static char g_stream_window[256];
static uint16_t g_stream_window_len = 0;
static volatile uint32_t g_usart2_rx_events = 0;
static volatile uint32_t g_usart2_rx_bytes = 0;
static volatile uint8_t g_usart2_rx_started = 0;
static volatile uint32_t g_usart2_rx_restart = 0;
static volatile uint32_t g_uart2_err = 0;
static volatile uint32_t g_uart2_err_ore = 0;
static volatile uint32_t g_uart2_err_fe = 0;
static volatile uint32_t g_uart2_err_ne = 0;
static volatile uint32_t g_uart2_err_pe = 0;

// 链路健康：最后一次收到服务器响应的时间戳（tick）
static volatile uint32_t g_last_rx_tick = 0;
static volatile uint8_t g_link_reconnecting = 0;
static uint8_t g_boot_hardreset_done = 0;
// HTTP 回包门控：避免在透传里“HTTP pipelining”堆积导致服务器解析失败/最终离线
static volatile uint8_t g_waiting_http_response = 0;
static volatile uint32_t g_waiting_http_tick = 0;

// USART2 当前是否处于 AT 命令阶段（阻塞式 HAL_UART_Receive/Send_Cmd）
// 处于该阶段时，不能让 ErrorCallback/RxEventCallback 继续重启 ReceiveToIdle_DMA，否则会锁死 HAL 状态机。
static volatile uint8_t g_uart2_at_mode = 0;

static uint8_t ESP_BufContains(const uint8_t *buf, uint16_t len, const char *needle)
{
    if (!buf || len == 0 || !needle || !*needle)
        return 0;
    uint16_t nlen = (uint16_t)strlen(needle);
    if (nlen == 0 || nlen > len)
        return 0;
    // 朴素扫描：len<=4096，needle 很短，够快且可控
    for (uint16_t i = 0; i + nlen <= len; i++)
    {
        if (memcmp(buf + i, needle, nlen) == 0)
            return 1;
    }
    return 0;
}

static void ESP_ForceStop_DMA(void)
{
    // 1) 关闭 IDLE 中断（ReceiveToIdle 依赖 IDLE，AT 阶段不应触发）
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_IDLE);

    // 2) 停止 DMA（包括 DMAR/DMAT），并强制 Abort UART（复位 HAL 状态机）
    (void)HAL_UART_DMAStop(&huart2);
    (void)HAL_UART_Abort(&huart2);

    // 3) 清错误标志，避免残留 FE/ORE 触发中断风暴
    ESP_Clear_Error_Flags();

    // 4) 标记：当前进入 AT 阶段
    g_usart2_rx_started = 0;
    g_waiting_http_response = 0;
}

static UART_HandleTypeDef *ESP_GetLogUart(void)
{
#if (ESP_LOG_UART_PORT == 1)
    extern UART_HandleTypeDef huart1;
    return &huart1;
#elif (ESP_LOG_UART_PORT == 2)
    return &huart2; // ⚠️一般不建议：USART2 用于 ESP8266 通信
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

void ESP_Init(void)
{
    char cmd_buf[128];
    g_esp_ready = 0; 
    int retry_count = 0;
    // 进入初始化/重连前：强制接管 USART2（避免 DMA+阻塞式 Receive 的 HAL 状态机冲突）
    g_uart2_at_mode = 1;
    ESP_ForceStop_DMA();

    // 启动时强制硬复位一次：保证 ESP 状态干净（用户要求）
#if (ESP_BOOT_HARDRESET_ONCE)
    if (!g_boot_hardreset_done)
    {
        g_boot_hardreset_done = 1;
        ESP_Log("[ESP] 启动硬复位一次...\r\n");
        ESP_HardReset();
    }
#endif

    // 确保 ESP8266 不处于硬件复位状态（RST 低有效）
#ifdef ESP8266_RST_Pin
    HAL_GPIO_WritePin(ESP8266_RST_GPIO_Port, ESP8266_RST_Pin, GPIO_PIN_SET);
#endif

    if (!fft_initialized)
    {
        arm_rfft_fast_init_f32(&S, WAVEFORM_POINTS);
        fft_initialized = 1;
    }

    ESP_Log("\r\n[ESP] 初始化（4通道模式）...\r\n");
    ESP_Log("[ESP] WiFi 名称(SSID): %s\r\n", WIFI_SSID);
    ESP_Log("[ESP] 服务器地址: %s:%d\r\n", SERVER_IP, SERVER_PORT);
    ESP_Clear_Error_Flags();
    // 优先尝试复用“透传 + TCP”现有连接（MCU 复位时无需断电 ESP）
    if (ESP_TryReuseTransparent())
    {
        g_esp_ready = 1;
        g_uart2_at_mode = 0;
        ESP_Log("[ESP] 复用透传连接成功（无需断电可重连）\r\n");
        return;
    }
    // 复用失败：必须先确保回到 AT 命令模式，否则后续 ATE0/CWMODE/CWJAP 只会 TIMEOUT
    // 注意：TIMEOUT 的本质不是“等待不够”，而是“没收到任何字节”，需要先退出透传并确认 AT 可用。
    ESP_Uart2_Drain(120);
    uint8_t at_ok = 0;
    for (int k = 0; k < 3; k++)
    {
        if (ESP_Send_Cmd("AT\r\n", "OK", 800))
        {
            at_ok = 1;
            break;
        }
        (void)ESP_Exit_Transparent_Mode_Strict(2000);
        ESP_Uart2_Drain(120);
    }
    if (!at_ok)
    {
        // 兜底：硬件复位 ESP8266（无需断电）
        ESP_Log("[ESP] AT无回显，执行硬复位ESP8266...\r\n");
        ESP_HardReset();
        // 硬复位后再次确保处于 AT(阻塞)可用状态
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
    HAL_Delay(3500); 
    ESP_Clear_Error_Flags();

    while (!ESP_Send_Cmd("ATE0\r\n", "OK", 500))
    {
        ESP_Log("[ESP] 关闭回显失败，重试 ATE0...\r\n");
        ESP_Clear_Error_Flags();
        HAL_Delay(500);
        retry_count++;
        if (retry_count > 5)
            break;
    }

    ESP_Send_Cmd("AT+CWMODE=1\r\n", "OK", 1000); 

    // 连接 WiFi（注意：不要在日志里打印明文密码）
    sprintf(cmd_buf, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
    if (!ESP_Send_Cmd(cmd_buf, "GOT IP", 20000))
    {
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
    ESP_Send_Cmd("AT+CIPCLOSE\r\n", "OK", 500); 
    sprintf(cmd_buf, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", SERVER_IP, SERVER_PORT);
    if (!ESP_Send_Cmd(cmd_buf, "CONNECT", 10000))
    {
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

    ESP_Send_Cmd("AT+CIPMODE=1\r\n", "OK", 1000);
    ESP_Send_Cmd("AT+CIPSEND\r\n", ">", 2000); 
    HAL_Delay(500);
    
    ESP_Register();
    g_esp_ready = 1;
    ESP_Log("[ESP] 系统就绪（4通道），开始上报数据。\r\n");

    // 启动 USART2 接收（DMA + IDLE），用于接收 /api/node/heartbeat 的响应中的 command/reset
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
    
    // 模拟瞬时值波动
    node_channels[ch_id].current_value = base_dc + ripple_amp * 0.1f * arm_sin_f32(2.0f * PI * 0.5f * time_t);

    // 1. 生成波形
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
    
    // 2. FFT 计算
    arm_rfft_fast_f32(&S, node_channels[ch_id].waveform, fft_output_buf, 0);
    arm_cmplx_mag_f32(fft_output_buf, fft_mag_buf, WAVEFORM_POINTS / 2);
    
    // 3. 填充 FFT (归一化)
    node_channels[ch_id].fft_data[0] = 0; // 去直流
    for (i = 1; i < FFT_POINTS; i++)
    {
        node_channels[ch_id].fft_data[i] = (fft_mag_buf[i] / (float)(WAVEFORM_POINTS / 2)) * 2.0f; 
    }

    // 4. 恢复波形 (FFT运算会破坏原buffer)
    for (i = 0; i < WAVEFORM_POINTS; i++)
    {
        // 简单重绘，只为了显示
        float phase = (float)ch_id * 0.5f;
        node_channels[ch_id].waveform[i] = node_channels[ch_id].current_value + ripple_amp * arm_sin_f32(2.0f * PI * 50.0f * ((float)i * 0.0005f) + phase) + (ripple_amp * 0.3f) * arm_sin_f32(2.0f * PI * 150.0f * ((float)i * 0.0005f));
    }

    if (ch_id == 3)
        time_t += 0.05f; // 时间步进
}

void ESP_Update_Data_And_FFT(void)
{
    // Ch0: 母线+ (375V, 纹波5V, 噪声2V)
    Process_Channel_Data(0, 375.0f, 5.0f, 2.0f);
    
    // Ch1: 母线- (375V, 纹波5V, 噪声2V) - 这里用正值表示幅值，也可以用 -375.0f
    Process_Channel_Data(1, 375.0f, 5.0f, 2.0f); 
    
    // Ch2: 负载电流 (12.5A, 纹波3A, 噪声0.5A)
    Process_Channel_Data(2, 12.5f, 3.0f, 0.5f);
    
    // Ch3: 漏电流 (20mA, 纹波5mA, 噪声2mA) - 单位是mA的话这里直接传数值
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
        {
            break;
        }
    }
}

static void ESP_SetFaultCode(const char *code)
{
    if (!code)
        return;
    // 允许输入 e01 / E01
    char c0 = code[0];
    char c1 = code[1];
    char c2 = code[2];
    if (c0 == 'e')
        c0 = 'E';
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

    // 直接输入 E01
    if ((line[0] == 'E' || line[0] == 'e') && strlen(line) == 3)
    {
        ESP_SetFaultCode(line);
        return;
    }

    // 兼容输入：fault E01
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

    // 启动 1 字节 RX 中断（比主循环轮询稳定，不会“输入了但没反应”）
    HAL_UART_Receive_IT(hu, &g_console_rx_byte, 1);
    ESP_Log("[控制台] 已启用：输入 E01 回车注入故障；输入 E00 回车清除。\r\n");
#endif
}

void ESP_Console_Poll(void)
{
#if (ESP_CONSOLE_ENABLE)
    // 1) 处理“已收到的一整行”
    if (g_console_line_ready)
    {
        g_console_line_ready = 0;
        g_console_line[g_console_line_len] = 0;
        ESP_Console_HandleLine(g_console_line);
        g_console_line_len = 0;
    }

    // 2) 处理“服务器下发 reset”
    if (g_server_reset_pending)
    {
        g_server_reset_pending = 0;
        ESP_Log("[服务器命令] 收到 reset：清除故障码 -> E00\r\n");
        ESP_SetFaultCode("E00");
    }

#if (ESP_DEBUG)
    // 3) 调试：确认 USART2 是否收到服务器响应（每 1 秒输出一次统计）
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

    // 4) 链路自恢复：如果长时间收不到服务器任何响应，直接硬复位并重连（用户要求）
    static uint32_t last_link_check = 0;
    static uint8_t no_rx_miss = 0;
    static uint32_t last_rx_bytes_snapshot = 0;
    if ((now - last_link_check) >= 1000)
    {
        last_link_check = now;
        if (g_esp_ready && !g_link_reconnecting && !g_uart2_at_mode)
        {
            // 正常情况下服务器会对每次心跳返回 HTTP 响应，所以 RX bytes/events 会持续增长
            // 但压力测试/网络抖动下，6s 太容易误判；这里做两层保护：
            // 1) 最小阈值钳到 30s
            // 2) 必须“连续多次”确认 RX 字节数不增长才触发硬复位
            uint32_t no_rx_ms = (uint32_t)ESP_NO_SERVER_RX_HARDRESET_SEC * 1000u;
            if (no_rx_ms < 30000u) no_rx_ms = 30000u;

            // 如果 RX 字节数在增长，直接清零 miss，并刷新 last_rx_tick（避免“明明有回包却误判无回包”）
            if (g_usart2_rx_bytes != last_rx_bytes_snapshot)
            {
                last_rx_bytes_snapshot = g_usart2_rx_bytes;
                g_last_rx_tick = now;
                no_rx_miss = 0;
            }
            else if (g_last_rx_tick != 0 && (now - g_last_rx_tick) > no_rx_ms)
            {
                no_rx_miss++;
                // 连续 3 次都满足“无增长且超阈值”，才认为真的断了
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

void ESP_Post_Data(void)
{
    if (g_esp_ready == 0)
        return;

    // 压力测试也要稳定：如果上一次请求的 HTTP 响应还没回来，就不要继续“堆请求”
    // 否则会出现服务器端 400/解析失败，最后导致节点超时离线。
    if (g_waiting_http_response)
    {
        uint32_t now = HAL_GetTick();
        // 关键修复：压力测试不能因为“没识别到 HTTP/1.1”就完全断流。
        // 丢一个回包不值得卡 1.5s：把门控超时改短，避免“卡一会发一会”
        if (g_waiting_http_tick != 0 && (now - g_waiting_http_tick) > 250u)
        {
#if (ESP_DEBUG)
            ESP_Log("[调试] HTTP 门控超时(>250ms)，放行继续发送\r\n");
#endif
            g_waiting_http_response = 0;
        }
        else
        {
            return;
        }
    }
    
    // 5Hz 限流
    static uint32_t last_send_time = 0;
    if (HAL_GetTick() - last_send_time < 200)
        return;
    last_send_time = HAL_GetTick();

    char *p = (char *)http_packet_buf;
    uint32_t body_len;
    uint32_t header_reserve_len = 256;
    int header_len;
    uint32_t total_len;
    
    // JSON Header
    p += sprintf(p, "{\"node_id\": \"%s\",\"status\": \"online\",\"fault_code\": \"%s\",\"channels\": [", NODE_ID, g_fault_code);
    
    // 循环发送 4 个通道的数据
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
        
        // 频谱数据（统一字段名：fft_spectrum）
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
    memmove(http_packet_buf + header_reserve_len, http_packet_buf, body_len);
    header_len = sprintf((char *)http_packet_buf, 
        "POST /api/node/heartbeat HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %u\r\n"
        "\r\n",
        SERVER_IP, SERVER_PORT, body_len);
    memmove(http_packet_buf + header_len, http_packet_buf + header_reserve_len, body_len);
    total_len = (uint32_t)header_len + body_len;
    
    // 关键诊断：确认是否真的在发、是否一直 BUSY、每包长度是多少
    static uint32_t tx_try = 0, tx_ok = 0, tx_busy = 0, tx_err = 0;
    static uint32_t last_tx_log = 0;
    tx_try++;

    // DMA 发送前必须 Clean DCache，否则 DMA 可能读到旧数据
    DCache_CleanByAddr_Any(http_packet_buf, total_len);
    HAL_StatusTypeDef st = HAL_UART_Transmit_DMA(&huart2, (uint8_t *)http_packet_buf, (uint16_t)total_len);
    if (st == HAL_OK)
        tx_ok++;
    else if (st == HAL_BUSY)
        tx_busy++;
    else
        tx_err++;

    if (st == HAL_OK)
    {
        g_waiting_http_response = 1;
        g_waiting_http_tick = HAL_GetTick();
    }

#if (ESP_DEBUG)
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
                g_console_line_len = 0;
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

    HAL_StatusTypeDef st = HAL_UARTEx_ReceiveToIdle_DMA(&huart2, g_stream_rx_buf, sizeof(g_stream_rx_buf));
    if (st == HAL_OK)
    {
        g_usart2_rx_started = 1;
        g_usart2_rx_restart++;
        g_last_rx_tick = HAL_GetTick();
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

    // ---------------- 关键修复：先扫描“原始数据块”，避免滑动窗口截断导致漏判 ----------------
    // 1) HTTP 响应：只要任意数据块里出现 HTTP/1.1，就认为已收到本次请求的回包（解除门控）
    if (g_waiting_http_response && ESP_BufContains(data, len, "HTTP/1.1"))
    {
        g_waiting_http_response = 0;
        g_last_rx_tick = HAL_GetTick();
    }
    // 2) 服务器命令：优先在原始数据里扫一遍，避免 reset 恰好落在被截断部分
    if (ESP_BufContains(data, len, "\"command\"") && ESP_BufContains(data, len, "reset"))
    {
        g_server_reset_pending = 1;
    }

    // 滑动窗口（按“块”处理，避免在中断里对每个字节 memmove 导致卡死/溢出）
    const uint16_t cap = (uint16_t)(sizeof(g_stream_window) - 1);
    if (len >= cap)
    {
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
                memmove(g_stream_window, g_stream_window + drop, g_stream_window_len - drop);
                g_stream_window_len = (uint16_t)(g_stream_window_len - drop);
            }
        }
        memcpy(g_stream_window + g_stream_window_len, data, len);
        g_stream_window_len = (uint16_t)(g_stream_window_len + len);
        g_stream_window[g_stream_window_len] = 0;
    }

    // 识别 JSON：{"command":"reset"}（也允许出现 reset 关键字）
    if (strstr(g_stream_window, "\"command\"") && strstr(g_stream_window, "reset"))
    {
        g_server_reset_pending = 1;
    }

    // 识别 HTTP 响应头：用于“发送节流门控”（不允许无限 pipelining）
    if (g_waiting_http_response && strstr(g_stream_window, "HTTP/1.1"))
    {
        g_waiting_http_response = 0;
        g_last_rx_tick = HAL_GetTick();
    }
}

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
            // 关键修复：门控不要死等 "HTTP/1.1"。
            // 只要收到了任何回包字节，就认为本次请求已有响应（否则会出现“<20s 就离线”的断流）。
            if (g_waiting_http_response)
            {
                g_waiting_http_response = 0;
            }
            // DMA 接收后 Invalidate DCache，否则 CPU 可能读到旧数据，导致解析不到 reset 等关键词
            DCache_InvalidateByAddr_Any(g_stream_rx_buf, Size);
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

// USART2 错误回调：典型是 ORE（接收溢出）导致后续不再进 RxEventCallback
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

        // 关键：AT(阻塞)阶段如果还去重启 ReceiveToIdle_DMA，会把 HAL 的 RxState 锁死为 BUSY_RX，
        // 导致后续 HAL_UART_Receive 直接 HAL_BUSY，从而出现“无限超时/假死”。
        // 发生错误时，当前这次 HTTP 回包很可能已经丢了；不要继续门控傻等
        if (g_waiting_http_response)
        {
            g_waiting_http_response = 0;
        }

        if (g_uart2_at_mode)
        {
            ESP_Clear_Error_Flags();
            (void)HAL_UART_Abort(&huart2);
            g_usart2_rx_started = 0;
            return;
        }

        // 透传阶段：清错误并重启接收（同时避免 HT 中断风暴）
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
    if (!kw || !*kw) return 0;
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

    // guard time (before)：期间不要向模块发送任何字节
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
    if (g_link_reconnecting) return;
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
    __HAL_UART_CLEAR_OREFLAG(&huart2);
    __HAL_UART_CLEAR_NEFLAG(&huart2);
    __HAL_UART_CLEAR_FEFLAG(&huart2);
}

// 硬件复位 ESP8266：拉低 RST 一段时间再拉高（不需要断电）
static void ESP_HardReset(void)
{
#ifdef ESP8266_RST_Pin
    // 硬复位前先停掉 UART2 DMA/中断，避免复位过程中错误中断风暴
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
