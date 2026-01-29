#include "ADS131A04_EVB.h"
#include "delay.h"
#include "main.h"
#include "mcp_myspi.h"
#include "stdio.h"

float ADS131A04_value[1024];
float ADSA_B[4][1024], ADSA_B2[4][1024];
int CJ_flag = 0;
int ADS131A04_flag = 0, ADS131A04_flag2 = 0;
int number = 0, number2 = 0;
float ADS131A04_Buf[4] = {0};
int AD_i = 0;

void START() {
  number = 0;
  ADS131A04_flag = 0;
}
void START2() {
  number2 = 0;
  ADS131A04_flag2 = 0;
}

#if !USE_AD7606
/*

├── GPIO_Init,SPI_DMA_Init (cubemx)
├── 通信基本函数
│   ├── 发送命令函数                       ADS131A04_Write_Command
│   └── 读写寄存器函数                     ADS131A04_Write_Reg
│       ├── 设置输入时钟预分频              ADS131A04_ICLK_SET_PREDIV
│       ├── 设置调制时钟预分频和过采样率      ADS131A04_MODULATION_SET_PREDIV
│       ├── 设置通道增益                   ADS131A04_Gain_SET
│       └── 设置参考电压                   ADS131A04_REF_SET
├── 上电初始化
│   └── 手册上电初始化                     ADS131A04_Power_Up
├── 应用函数
│   ├── 通道使能         ADS_Set_CH_Enable
│   ├── 采样点数设置      ADS_Set_n
│   ├── 填充数据函数      ADS_Fill_Data
│   ├── 开始接收函数      ADS_Start
│   ├── 停止接收函数      ADS_Stop
│   └── 应用初始化       ADS131A04_Init
├── 中断函数
│   ├── PP开启时
│   │   ├── ADS_HalfPP_Callback
│   │   └── ADS_FullPP_Callback
│   └── 不开启
│       └── ADS_FullPP_Callback

 */
/**初始化ADS131**/
void ADS131A0X_Init(void) { ADS131A0X_soft_spi(); }
void ADS131A0X_Delay(void) {
  uint32_t i;
  for (i = 0; i < 1; i++)
    __NOP();
}
/*****************************************************************************************
 * Function Name: ADS13_SPI
 * Description  : 通过SPI总线与ADS131通信
 * Arguments    : com:需写入的数据
 * Return Value : 返回读取的数据
 ******************************************************************************************/
unsigned char ADS13_SPI(unsigned char com) {
  return (ADS131A0X_SendRecive_8Bit(com));
}

/* write a command to ADS131a02 */
uint16_t ADS13_WRITE_CMD(uint16_t command) {
  uint16_t receive = 0x0000;
  unsigned int i;
  ADS131A04_CS_L;
  //	delay_us(1);
  // ADS131A0X_Delay();
  receive = ADS13_SPI((uint8_t)(command >> 8));
  receive <<= 8;
  receive |= ADS13_SPI((uint8_t)(command));
  ADS13_SPI(0X00); // 填满3个字节   24bit
  ADS13_SPI(0X00); // 填满4个字节   32bit
  ADS131A04_CS_H;
  //		delay_us(1);
  // ADS131A0X_Delay();
  return receive; // 返回接收的数据
}

/*****************************************************************************************
 * Function Name: ADS13_REG
 * Description  : 对ADS131内部寄存器进行操作
 * Arguments    : com:读写寄存器地址，data：需写入的数据
 * Return Value : 返回读取的数据
 ******************************************************************************************/
uint16_t ADS13_REG(unsigned char com, unsigned data) {
  unsigned int i;
  uint16_t temp = 0X0000;
  if ((com & 0x20) == 0x20) // 判断是否为读寄存器指令 if里面为读
  {
    ADS13_WRITE_CMD(com);
    temp = ADS13_WRITE_CMD(ADS131A04_NULL_COMMAND);
  } else // 写寄存器
  {
    ADS131A04_CS_L;
    //		delay_us(1);
    // ADS131A0X_Delay();
    ADS13_SPI(com);
    ADS13_SPI(data);
    ADS13_SPI(0X00); // 补齐0  24bit
    ADS13_SPI(0X00); // 补齐0  32bit
    // delay_us(1);
    // ADS131A0X_Delay();
    ADS131A04_CS_H;
  }
  // delay_us(1);
  return temp;
}

/*****************************************************************************************
 * Function Name: ADS13_PowerOnInit
 * Description  : ADS13上电复位 (当前配置：用户自定义参数 0x16)
 * Arguments    : NONE
 * Return Value : NONE
 ******************************************************************************************/
 int adcenable_flag = 0;
 int Dev_ID;
 
 void ADS13_PowerOnInit(void)
 {
     uint8_t RREG = 0X20;       // 读
     uint8_t WREG = 0X40;       // 写
     uint16_t RECEIVE = 0X0000; // 接收spi返回的字符
     
     ADS131A0X_Init();
     delay_ms(20);
 
     do
     {
         ADS13_WRITE_CMD(ADS131A04_RESET_COMMAND); // RESET ADS131
         RECEIVE = ADS13_WRITE_CMD(ADS131A04_NULL_COMMAND);
         printf("receive data is : %X...\n", RECEIVE);
         if (RECEIVE == ADS131A04_READY || RECEIVE == 0xFF02) // ADS131A02/4 is ready.break loop
             printf("ADS131A02 is ready!\n  Configuring Registers...\n");
         delay_ms(5);
     } while (RECEIVE != ADS131A04_READY && RECEIVE != 0xFF02); // 初始化成功则跳出循环
 
     Dev_ID = RECEIVE & 0x000F;
     ADS13_WRITE_CMD(ADS131A04_UNCLOCK_COMMAND); // 解锁寄存器
     RECEIVE = ADS13_WRITE_CMD(ADS131A04_NULL_COMMAND);
 
     ADS13_WRITE_CMD(0x2000); // read id
     RECEIVE = ADS13_WRITE_CMD(ADS131A04_NULL_COMMAND);
 
     ADS13_WRITE_CMD(0x2100); // read id
     RECEIVE = ADS13_WRITE_CMD(ADS131A04_NULL_COMMAND);
 
     // --- 核心配置修改开始 ---
     ADS13_REG(WREG | A_SYS_CFG, 0X60);          // External reference voltage
     ADS13_REG(WREG | D_SYS_CFG, 0X3E);          // Fixed six device words per frame
 
     // 1. 设置 CLK1
     // 输入 16.384MHz -> 2分频 -> fICLK = 8.192MHz
     // 值 0x02 (二进制 0000 0010, CLK_DIV=001)
     ADS13_REG(WREG | CLK1, 0X02);               
 
     // 2. 设置 CLK2
     // 值 0x16 (二进制 0001 0110) - 用户验证的稳定参数
     // 高3位(ICLK_DIV) = 000 (该位域设置为0，配合特定硬件/时序)
     // 低4位(OSR)      = 0110 (OSR=400) -> 对应 10.24 kSPS 档位
     ADS13_REG(WREG | CLK2, 0x16);                
 
     ADS13_REG(WREG | ADC_ENA, 0X0F);            // ADC CHANNEL ENABLE ALL
     // --- 核心配置修改结束 ---
 
     ADS13_WRITE_CMD(0x2B00); // read A_SYS_CFG
     RECEIVE = ADS13_WRITE_CMD(ADS131A04_NULL_COMMAND);
 
     ADS13_WRITE_CMD(0x2C00); // read D_SYS_CFG
     RECEIVE = ADS13_WRITE_CMD(ADS131A04_NULL_COMMAND);
 
     ADS13_WRITE_CMD(ADS131A04_WAKEUP_COMMAND); // WAKEUP ADS131
     RECEIVE = ADS13_WRITE_CMD(ADS131A04_NULL_COMMAND);
 
     adcenable_flag = 1; // 初始化完毕标志位
 }
 
uint8_t ADC_READY(void) {
  uint16_t ret = 0;
  uint16_t RECEIVE;
  ADS13_WRITE_CMD(0x2200); // read D_SYS_CFG
  RECEIVE = ADS13_WRITE_CMD(ADS131A04_NULL_COMMAND);
  RECEIVE = RECEIVE & 0x0002;
  ret = (uint8_t)RECEIVE;
  return ret;
}

uint8_t dummy_send[24] = {0};
uint8_t recBuffer[24];

/* 软SPI坏帧统计（用于验证尖峰是否来自“坏帧/撕裂帧/位滑移帧”） */
static volatile uint32_t g_ads_soft_total = 0;
static volatile uint32_t g_ads_soft_bad = 0;
static volatile uint32_t g_ads_soft_last_bad_status = 0;
static uint8_t g_ads_soft_last_bad_raw[24];

/* 坏帧插值统计 */
static volatile uint32_t g_ads_interp_runs = 0;
static volatile uint32_t g_ads_interp_points = 0;
static volatile uint32_t g_ads_interp_max_run = 0;

typedef struct
{
    uint8_t pending;
    uint8_t have_prev;
    uint16_t start_idx;
    uint16_t run_len;
    float prev[4];
} ads_interp_state_t;

static ads_interp_state_t g_interp_b = {0};
static ads_interp_state_t g_interp_b2 = {0};

static inline ads_interp_state_t *ADS131A04_SelectInterpState(float (*buf)[1024])
{
    if (buf == ADSA_B2) return &g_interp_b2;
    return &g_interp_b;
}

void ADS131A04_GetInterpStats(uint32_t *runs, uint32_t *points, uint32_t *max_run)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    if (runs) *runs = g_ads_interp_runs;
    if (points) *points = g_ads_interp_points;
    if (max_run) *max_run = g_ads_interp_max_run;
    __set_PRIMASK(primask);
}

void ADS131A04_ResetInterpStats(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    g_ads_interp_runs = 0;
    g_ads_interp_points = 0;
    g_ads_interp_max_run = 0;
    __set_PRIMASK(primask);
}

void ADS131A04_InterpUpdate(float (*buf)[1024], uint16_t idx, const float sample[4], uint8_t is_good)
{
    ads_interp_state_t *st = ADS131A04_SelectInterpState(buf);
    if (!st || !buf || !sample) return;

    /* 每次从 0 开始填充时，重置本轮插值状态 */
    if (idx == 0) {
        st->pending = 0;
        st->run_len = 0;
        st->have_prev = 0;
    }

    if (is_good)
    {
        if (st->pending && st->run_len > 0 && st->have_prev)
        {
            uint16_t N = st->run_len;
            for (uint16_t k = 1; k <= N; k++)
            {
                float t = (float)k / (float)(N + 1u);
                for (uint8_t ch = 0; ch < 4; ch++)
                {
                    buf[ch][st->start_idx + k - 1] = st->prev[ch] + (sample[ch] - st->prev[ch]) * t;
                }
            }
            g_ads_interp_runs++;
            g_ads_interp_points += N;
            if (N > g_ads_interp_max_run) g_ads_interp_max_run = N;
            st->pending = 0;
            st->run_len = 0;
        }
        for (uint8_t ch = 0; ch < 4; ch++) st->prev[ch] = sample[ch];
        st->have_prev = 1;
    }
    else
    {
        if (!st->have_prev)
        {
            for (uint8_t ch = 0; ch < 4; ch++) st->prev[ch] = sample[ch];
            st->have_prev = 1;
        }
        if (!st->pending)
        {
            st->pending = 1;
            st->start_idx = idx;
            st->run_len = 1;
        }
        else
        {
            if (st->run_len < 1024u) st->run_len++;
        }
    }
}

void ADS131A04_GetSoftSpiBadFrameStats(uint32_t *total, uint32_t *bad, uint32_t *last_status, uint8_t out24[24])
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    if (total) *total = g_ads_soft_total;
    if (bad) *bad = g_ads_soft_bad;
    if (last_status) *last_status = g_ads_soft_last_bad_status;
    if (out24) {
        for (int i = 0; i < 24; i++) out24[i] = g_ads_soft_last_bad_raw[i];
    }
    __set_PRIMASK(primask);
}

void ADS131A04_ResetSoftSpiBadFrameStats(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    g_ads_soft_total = 0;
    g_ads_soft_bad = 0;
    g_ads_soft_last_bad_status = 0;
    for (int i = 0; i < 24; i++) g_ads_soft_last_bad_raw[i] = 0;
    __set_PRIMASK(primask);
}

void Read_ADS131A0X_BUFFER_DATA(void) {
  uint8_t i = 0;
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  ADS131A04_CS_L;
  for (i = 0; i < 24; i++) {
    recBuffer[i] = ADS13_SPI(dummy_send[i]);
  }
  ADS131A04_CS_H;
  __set_PRIMASK(primask);
}

unsigned long Read_ADS131A0X_Value(float ADS[]) // 读取ADC
{
  unsigned long value = 0;
  static float last_good[4] = {0};
  static uint8_t have_last = 0;
  union {
    unsigned long voltage;
    struct {
      unsigned char LSB;
      unsigned char NSB;
      unsigned char MSB;
      unsigned char d;
    } byte;
  } volt0, volt1, volt2, volt3;

  while (READ_ADS131A04_DRDY)
    ; // 等待转换完成

  // 获取总线数据 24bit
  Read_ADS131A0X_BUFFER_DATA();
  g_ads_soft_total++;

  /* 解析 STATUS 字并判定坏帧（使用已定义的错误标志） */
  uint32_t status =
      ((uint32_t)recBuffer[0] << 24) | ((uint32_t)recBuffer[1] << 16) |
      ((uint32_t)recBuffer[2] << 8) | ((uint32_t)recBuffer[3]);
  uint16_t stat = (uint16_t)(status & 0xFFFFu);
  stat |= (uint16_t)((status >> 16) & 0xFFFFu);
  if (stat & (ADS131_STAT_1_F_WDT | ADS131_STAT_1_F_RESYNC |
              ADS131_STAT_1_F_CHECK | ADS131_STAT_S_F_FRAME)) {
    g_ads_soft_bad++;
    g_ads_soft_last_bad_status = status;
    for (int i = 0; i < 24; i++) g_ads_soft_last_bad_raw[i] = recBuffer[i];
    if (have_last) {
      ADS[0] = last_good[0];
      ADS[1] = last_good[1];
      ADS[2] = last_good[2];
      ADS[3] = last_good[3];
    } else {
      ADS[0] = 0.0f;
      ADS[1] = 0.0f;
      ADS[2] = 0.0f;
      ADS[3] = 0.0f;
    }
    return 0;
  }

  volt0.byte.MSB = recBuffer[4];
  volt0.byte.NSB = recBuffer[5];
  volt0.byte.LSB = recBuffer[6];
  volt0.byte.d = recBuffer[7];

  volt1.byte.MSB = recBuffer[8];
  volt1.byte.NSB = recBuffer[9];
  volt1.byte.LSB = recBuffer[10];
  volt1.byte.d = recBuffer[11];

  volt2.byte.MSB = recBuffer[12];
  volt2.byte.NSB = recBuffer[13];
  volt2.byte.LSB = recBuffer[14];
  volt2.byte.d = recBuffer[15];

  volt3.byte.MSB = recBuffer[16];
  volt3.byte.NSB = recBuffer[17];
  volt3.byte.LSB = recBuffer[18];
  volt3.byte.d = recBuffer[19];

  //	switch(channel)
  //	{
  //		case 0:value = volt0.voltage;break;
  //		case 1:value = volt1.voltage;break;
  //		case 2:value = volt2.voltage;break;
  //		case 3:value = volt3.voltage;break;
  //	}

  ADS[0] = DataFormatting(volt0.voltage, 2.5f * 2.0f, 1);
  ADS[1] = DataFormatting(volt1.voltage, 2.5f * 2.0f, 1);
  ADS[2] = DataFormatting(volt2.voltage, 2.5f * 2.0f, 1) * 465.95 / 473.20;
  ADS[3] = DataFormatting(volt3.voltage, 2.5f * 2.0f, 1);
  last_good[0] = ADS[0];
  last_good[1] = ADS[1];
  last_good[2] = ADS[2];
  last_good[3] = ADS[3];
  have_last = 1;
  return 1;
}

// 添加1
uint32_t ADS131A04_Write_Command(uint32_t command) {
  uint32_t rev = 0;
  uint32_t cmd;
  cmd = (uint32_t)command << 16;
  ADS13_WRITE_CMD(command);
  return rev;
}
// 添加2
uint32_t ADS131A04_Write_Reg(uint8_t reg_add, uint8_t data) {
  uint32_t temp = 0;
  uint32_t volatile com_t = 0;
  com_t = ((uint32_t)reg_add << 24) + ((uint32_t)data << 16);
  if ((reg_add & 0x20) == 0x20) {
    ADS131A04_Write_Command(((uint32_t)reg_add << 24) >> 16);
    temp = ADS131A04_Write_Command(ADS_NULL);
  } else {
    temp = ADS131A04_Write_Command(com_t >> 16);
  }
  return temp;
}

/*-------------------------------------------------------------
  *  @brief     设置调制时钟的预分频系数
  *  @param     pre_divider_mod     设置调制时钟的预分频系数，详细见参数列表
                                        ADS_INCLK_DIV_2,ADS_INCLK_DIV_4,ADS_INCLK_DIV_6,ADS_INCLK_DIV_8,ADS_INCLK_DIV_10,ADS_INCLK_DIV_12,ADS_INCLK_DIV_14
  *             pre_divider_osr     设置过采样率
                                        ADS_OSR_DIV_4096,ADS_OSR_DIV_2048,ADS_OSR_DIV_1024,ADS_OSR_DIV_800,ADS_OSR_DIV_768,ADS_OSR_DIV_512
                                        ADS_OSR_DIV_400,ADS_OSR_DIV_384,ADS_OSR_DIV_256,ADS_OSR_DIV_200,ADS_OSR_DIV_192,ADS_OSR_DIV_128
                                        ADS_OSR_DIV_96,ADS_OSR_DIV_64,ADS_OSR_DIV_48,ADS_OSR_DIV_32
  *  @return    已置位数据，将设好的与返回值相或
  *  @note      none
  *  @Sample usage:     ADS131A04_MOD_SET_PREDIV(INCLK_DIV_4)
 -------------------------------------------------------------*/
ADS_res ADS131A04_MODULATION_SET_PREDIV(uint8_t pre_divider_mod,
                                        uint8_t pre_divider_osr) {
  uint8_t temp = 0x00;
  uint32_t rev = ADS131A04_Write_Reg(WRITE_CMD | CLK2,
                                     (pre_divider_mod << 5) | pre_divider_osr);
  printf("\n\rMODULATION_SET_PREDIV接收到的回复:%x\n\r", rev);
  return ADS_OK;
}
/*-------------------------------------------------------------
  *  @brief     设置ADC同步模式时的预分频系数
  *  @param     pre_divider     预分频参数，详细见参数列表
  *
 ADS_INCLK_DIV_2,ADS_INCLK_DIV_4,ADS_INCLK_DIV_6,ADS_INCLK_DIV_8,ADS_INCLK_DIV_10,ADS_INCLK_DIV_12,ADS_INCLK_DIV_14
  *  @return    已置位数据
  *  @note      none
  *  @Sample usage:     ADS131A04_ICLK_SET_PREDIV(INCLK_DIV_4)
 -------------------------------------------------------------*/
ADS_res ADS131A04_ICLK_SET_PREDIV(uint8_t pre_divider) {
  uint8_t temp = 0x00;
  uint32_t rev =
      ADS131A04_Write_Reg(WRITE_CMD | CLK1, temp | (pre_divider << 1));
  printf("\n\rICLK_SET_PREDIV接收到的回复:%x\n\r", rev);
  return ADS_OK;
}
/*-------------------------------------------------------------
  *  @brief     设置ADC参考电压模式
  *  @param     ref_mode        0:采用2.442V    1:采用4V
  *             ref_source      0:采用外部参考电压      1:采用内部参考电压
  *  @return    已置位数据，将设好的与返回值相或
  *  @note      none
  *  @Sample usage:     0X60|ADS131A04_REF_SET(1,1)
 -------------------------------------------------------------*/
ADS_res ADS131A04_REF_SET(uint8_t ref_mode, uint8_t ref_source) {
  uint8_t temp = 0x00;
  switch (ref_mode) {
  case 0:
    temp = temp | 0x00;
    break;
  case 1:
    temp = temp | 0x10;
    break;
  default:
    return ADS_ERROR;
  }

  switch (ref_source) {
  case 0:
    temp = temp | 0x00;
    break;
  case 1:
    temp = temp | 0x08;
    break;
  default:
    return ADS_ERROR;
  }

  uint32_t rev = ADS131A04_Write_Reg(WRITE_CMD | A_SYS_CFG, 0X60 | temp);
  printf("\n\rREF_SET接收到的回复:%x\n\r", rev);
  return ADS_OK;
}
/*-------------------------------------------------------------
  *  @brief     设置ADC对应增益
  *  @param     ch        通道
                                ADS_ADC1
                                ADS_ADC2
                                ADS_ADC3
                                ADS_ADC4
  *             gain        增益设置
                                ADCX_GAIN_1
                                ADCX_GAIN_2
                                ADCX_GAIN_4
                                ADCX_GAIN_8
                                ADCX_GAIN_16
  *  @return    已置位数据，将设好的与返回值相或
  *  @note      none
  *  @Sample usage:     ADS131A04_Gain_SET(ADS_ADC1,ADCX_GAIN_1);
 -------------------------------------------------------------*/
ADS_res ADS131A04_Gain_SET(uint8_t ch, uint8_t gain) {
  ADS131A04_Write_Reg(WRITE_CMD | ch, gain);
  return ADS_OK;
}

ADS_res ADS131A04_Power_Up(void) {
  uint32_t rev = 0;
  rev = ADS131A04_Write_Command(ADS131A04_RESET_COMMAND); // 重置
  while (rev != 0xff040000) {
    printf("\n\r上电重置接收到的回复:%x\n\r", rev);
    rev = ADS131A04_Write_Command(ADS131A04_RESET_COMMAND); // 接收READY信号
  }
  printf("\n\r已接收到READY\n\r");
  rev = ADS131A04_Write_Command(ADS131A04_UNLOCK_COMMAND); // 解锁
  printf("\n\rUNLOCK接收到的回复:%x\n\r", rev);
  ADS131A04_REF_SET(0, 0); // 设置参考电压
  rev = ADS131A04_Write_Reg(READ_CMD | A_SYS_CFG, 0);
  printf("\n\rREAD:%x\n\r", rev);
  ADS131A04_ICLK_SET_PREDIV(ADS_INCLK_DIV_2); // 设置输入时钟预分频
  ADS131A04_MODULATION_SET_PREDIV(
      ADS_INCLK_DIV_2, ADS_OSR_DIV_400); // 设置调制时钟预分频与过采样率预分频

  rev = ADS131A04_Gain_SET(ADS_ADC1, ADCX_GAIN_1);
  printf("\n\rREAD:%x\n\r", rev);
  rev = ADS131A04_Gain_SET(ADS_ADC2, ADCX_GAIN_1);
  printf("\n\rREAD:%x\n\r", rev);
  // 10kHz采样率
  rev = ADS131A04_Write_Reg(WRITE_CMD | ADC_ENA,
                            0X00 | ADC_ALL_ENABLE); // 开启使能
  printf("\n\r开启使能接收到的回复:%x\n\r", rev);
  rev = ADS131A04_Write_Command(ADS131A04_WAKEUP_COMMAND); // 唤醒
  printf("\n\rWAKEUP接收到的回复:%x\n\r", rev);

  return ADS_OK;
}

void swap(float *a, float *b) {
  float c;
  c = *a;
  *a = *b;
  *b = c;
}
float GetAverage(float *dat, uint16_t leng) // 中值滤波
{
  float Average = 0;
  uint16_t i, j;
  // 排序，从dat[0]开始排序，从小到大
  for (i = 0; i < leng; i++) {
    for (j = i + 1; j < leng; j++) {
      if (dat[i] > dat[j]) {
        swap(&dat[i], &dat[j]);
      }
    }
  }

  Average = dat[leng / 2];

  return Average;
}

/****************************************
//功能:把读数转化成电压值,输入分别为 ： 读回的二进制值   参考电压   内置增益
*****************************************/
double DataFormatting(uint32_t Data, double Vref, uint8_t PGA) {
  /*
  电压计算公式；
                  设：AD采样的电压为Vin ,AD采样二进制值为X，参考电压为 Vr
  ,内部集成运放增益为G Vin = ( (Vr) / G ) * ( x / (2^23 -1))
  */
  /* 关键修复：
   * ADS131A04 每通道为 24-bit 二补码；SPI 帧里经常是 32-bit
   * word（含状态/填充字节）。 在本工程高负载/软 SPI
   * 抖动场景下，最高字节偶尔会被污染，若不屏蔽会导致“特别大的异常值”。 */
  uint32_t raw24 = Data & 0x00FFFFFFu;
  int32_t code = (int32_t)raw24;
  if (raw24 & 0x00800000u) {
    /* sign-extend 24-bit to 32-bit */
    code |= (int32_t)0xFF000000u;
  }

  /* 注意：分母沿用原工程的 8388607(=2^23-1) */
  double ReadVoltage = (((double)code) / 8388607.0) * (Vref / (double)PGA);
  return ReadVoltage;
}

#endif /* !USE_AD7606 */
