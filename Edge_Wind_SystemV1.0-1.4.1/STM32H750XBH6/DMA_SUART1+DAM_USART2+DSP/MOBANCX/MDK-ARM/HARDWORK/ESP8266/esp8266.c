#include "esp8266.h"
#include "usart.h"      
#include "arm_math.h"   
#include <stdio.h>
#include <string.h>
#include <stdlib.h>     
#include <stdarg.h>     

extern UART_HandleTypeDef huart2;

/* ================= 内存分配 ================= */
/* ⚠️ 扩大到 48KB 以容纳 4 个通道的 JSON 数据 */
static uint8_t http_packet_buf[49152]; 
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
static void Process_Channel_Data(int ch_id, float base_dc, float ripple_amp, float noise_level);

/* ================= 核心代码 ================= */

void ESP_Init(void) {
    char cmd_buf[128];
    g_esp_ready = 0; 
    int retry_count = 0;

    if (!fft_initialized) {
        arm_rfft_fast_init_f32(&S, WAVEFORM_POINTS);
        fft_initialized = 1;
    }

    ESP_Log("\r\n[ESP] Init (4-Channel Mode)...\r\n");
    ESP_Clear_Error_Flags();
    ESP_Exit_Transparent_Mode();

    /* 复位流程 */
    ESP_Log("[ESP CMD] >> AT+RST\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t *)"AT+RST\r\n", 8, 100);
    HAL_Delay(3500); 
    ESP_Clear_Error_Flags();

    while(!ESP_Send_Cmd("ATE0\r\n", "OK", 500)) {
        ESP_Log("[ESP] Retrying ATE0...\r\n");
        ESP_Clear_Error_Flags();
        HAL_Delay(500);
        retry_count++;
        if(retry_count > 5) break;
    }

    ESP_Send_Cmd("AT+CWMODE=1\r\n", "OK", 1000); 
    sprintf(cmd_buf, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
    if (!ESP_Send_Cmd(cmd_buf, "GOT IP", 20000)) {
        if (!ESP_Send_Cmd(cmd_buf, "GOT IP", 20000)) {
             ESP_Log("[ESP] WiFi Failed.\r\n");
             return;
        }
    }

    /* TCP 连接 */
    ESP_Send_Cmd("AT+CIPCLOSE\r\n", "OK", 500); 
    sprintf(cmd_buf, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", SERVER_IP, SERVER_PORT);
    if (!ESP_Send_Cmd(cmd_buf, "CONNECT", 10000)) {
        if(strstr((char*)esp_rx_buf, "ALREADY") == NULL) {
             ESP_Log("[ESP] TCP Failed.\r\n");
             return;
        }
    }

    ESP_Send_Cmd("AT+CIPMODE=1\r\n", "OK", 1000);
    ESP_Send_Cmd("AT+CIPSEND\r\n", ">", 2000); 
    HAL_Delay(500);
    
    ESP_Register();
    g_esp_ready = 1;
    ESP_Log("[ESP] SYSTEM READY (4 Channels).\r\n");

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
static void Process_Channel_Data(int ch_id, float base_dc, float ripple_amp, float noise_level) {
    static float time_t = 0.0f;
    float noise;
    int i;
    
    // 模拟瞬时值波动
    node_channels[ch_id].current_value = base_dc + ripple_amp * 0.1f * arm_sin_f32(2.0f * PI * 0.5f * time_t);

    // 1. 生成波形
    for (i = 0; i < WAVEFORM_POINTS; i++) {
        // 基础噪声
        noise = ((float)(rand() % 100) / 100.0f - 0.5f) * noise_level; 
        
        // 漏电流特殊处理：加一些随机尖峰
        if (ch_id == 3 && (rand() % 100) > 98) {
            noise += 5.0f; // 突发漏电尖峰
        }

        // 信号组合：直流基准 + 50Hz纹波 + 150Hz谐波 + 噪声
        float phase = (float)ch_id * 0.5f; // 不同通道相位错开
        
        node_channels[ch_id].waveform[i] = node_channels[ch_id].current_value 
            + ripple_amp * arm_sin_f32(2.0f * PI * 50.0f * ((float)i * 0.0005f) + phase)
            + (ripple_amp * 0.3f) * arm_sin_f32(2.0f * PI * 150.0f * ((float)i * 0.0005f))
            + noise;
    }
    
    // 2. FFT 计算
    arm_rfft_fast_f32(&S, node_channels[ch_id].waveform, fft_output_buf, 0);
    arm_cmplx_mag_f32(fft_output_buf, fft_mag_buf, WAVEFORM_POINTS / 2);
    
    // 3. 填充 FFT (归一化)
    node_channels[ch_id].fft_data[0] = 0; // 去直流
    for(i=1; i < FFT_POINTS; i++) {
        node_channels[ch_id].fft_data[i] = (fft_mag_buf[i] / (float)(WAVEFORM_POINTS / 2)) * 2.0f; 
    }

    // 4. 恢复波形 (FFT运算会破坏原buffer)
    for (i = 0; i < WAVEFORM_POINTS; i++) {
        // 简单重绘，只为了显示
        float phase = (float)ch_id * 0.5f;
        node_channels[ch_id].waveform[i] = node_channels[ch_id].current_value 
            + ripple_amp * arm_sin_f32(2.0f * PI * 50.0f * ((float)i * 0.0005f) + phase)
            + (ripple_amp * 0.3f) * arm_sin_f32(2.0f * PI * 150.0f * ((float)i * 0.0005f));
    }

    if(ch_id == 3) time_t += 0.05f; // 时间步进
}

void ESP_Update_Data_And_FFT(void) {
    // Ch0: 母线+ (375V, 纹波5V, 噪声2V)
    Process_Channel_Data(0, 375.0f, 5.0f, 2.0f);
    
    // Ch1: 母线- (375V, 纹波5V, 噪声2V) - 这里用正值表示幅值，也可以用 -375.0f
    Process_Channel_Data(1, 375.0f, 5.0f, 2.0f); 
    
    // Ch2: 负载电流 (12.5A, 纹波3A, 噪声0.5A)
    Process_Channel_Data(2, 12.5f, 3.0f, 0.5f);
    
    // Ch3: 漏电流 (20mA, 纹波5mA, 噪声2mA) - 单位是mA的话这里直接传数值
    Process_Channel_Data(3, 20.0f, 5.0f, 2.0f); 
}

void ESP_Post_Data(void) {
    if (g_esp_ready == 0) return;
    
    // 5Hz 限流
    static uint32_t last_send_time = 0;
    if (HAL_GetTick() - last_send_time < 200) return; 
    last_send_time = HAL_GetTick();

    char *p = (char *)http_packet_buf;
    uint32_t body_len;
    uint32_t header_reserve_len = 256;
    int header_len;
    uint32_t total_len;
    
    // JSON Header
    p += sprintf(p, "{\"node_id\": \"%s\",\"status\": \"online\",\"fault_code\": \"E00\",\"channels\": [", NODE_ID);
    
    // 循环发送 4 个通道的数据
    for(int i=0; i<4; i++) {
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
        
        // 冗余 fft 字段
        p += sprintf(p, "], \"fft\": [");
        Helper_FloatArray_To_String(p, node_channels[i].fft_data, FFT_POINTS, 1); 
        p += strlen(p);
        
        // 结束当前 channel
        p += sprintf(p, "]}");
        if(i < 3) p += sprintf(p, ","); // 逗号分隔
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
    
    if(HAL_UART_GetState(&huart2) == HAL_UART_STATE_READY) {
        HAL_UART_Transmit_DMA(&huart2, (uint8_t *)http_packet_buf, (uint16_t)total_len);
    }
}

void ESP_Register(void) {
    char *body_start = (char *)http_packet_buf + 256; 
    ESP_Log("[ESP] Registering...\r\n");
    sprintf(body_start, "{\"device_id\":\"%s\",\"location\":\"%s\",\"hw_version\":\"v1.0_4CH\"}", NODE_ID, NODE_LOCATION);
    uint32_t body_len = strlen(body_start);
    int h_len = sprintf((char *)http_packet_buf, 
        "POST /api/register HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: application/json\r\nContent-Length: %u\r\n\r\n",
        SERVER_IP, SERVER_PORT, body_len);
    memmove(http_packet_buf + h_len, body_start, body_len);
    HAL_UART_Transmit(&huart2, http_packet_buf, h_len + body_len, 1000);
    HAL_Delay(500);
}

/* 辅助函数 */
static void ESP_Clear_Error_Flags(void) {
    volatile uint32_t isr = huart2.Instance->ISR;
    volatile uint32_t rdr = huart2.Instance->RDR;
    (void)isr; (void)rdr;
    __HAL_UART_CLEAR_OREFLAG(&huart2);
    __HAL_UART_CLEAR_NEFLAG(&huart2);
    __HAL_UART_CLEAR_FEFLAG(&huart2);
}

static uint8_t ESP_Send_Cmd(const char *cmd, const char *reply, uint32_t timeout) {
    uint32_t start;
    uint16_t idx = 0;
    ESP_Clear_Error_Flags();
    memset(esp_rx_buf, 0, sizeof(esp_rx_buf));
    HAL_UART_Transmit(&huart2, (uint8_t *)cmd, strlen(cmd), 100);
    start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout) {
        uint8_t ch;
        if (HAL_UART_Receive(&huart2, &ch, 1, 5) == HAL_OK) { 
            if (idx < sizeof(esp_rx_buf) - 1) {
                esp_rx_buf[idx++] = ch;
                esp_rx_buf[idx] = 0;
                if (strstr((char *)esp_rx_buf, reply) != NULL) return 1;
            }
        } 
    }
    return 0;
}

static void Helper_FloatArray_To_String(char *dest, float *data, int count, int step) {
    int i;
    for(i = 0; i < count; i += step) {
        dest += sprintf(dest, "%.1f", data[i]); 
        if(i + step < count) *dest++ = ',';
    }
    *dest = 0; 
}

static void ESP_Exit_Transparent_Mode(void) {
    HAL_Delay(200);
    HAL_UART_Transmit(&huart2, (uint8_t *)"+++", 3, 100);
    HAL_Delay(1000);
}

static void ESP_Log(const char *format, ...) {
    char log_buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    va_end(args);
    printf("%s", log_buf);
}

