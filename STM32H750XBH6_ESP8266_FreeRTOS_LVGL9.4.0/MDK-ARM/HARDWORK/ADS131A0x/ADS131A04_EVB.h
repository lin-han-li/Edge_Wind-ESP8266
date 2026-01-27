#ifndef ADS131A04_EVB_H_
#define ADS131A04_EVB_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * =============================================================================
 * ADS131A04 驱动对外接口（基于 TI ADS131A0x 数据手册）
 * =============================================================================
 *
 * 设计目标：在不破坏你当前工程调用方式的前提下，把驱动补齐到“按手册协议工作”。
 * - 正确处理：命令响应、固定/动态帧长度、寄存器读回、STAT 寄存器/故障位、可选 CRC 校验。
 *
 * 你在 `main.c` 里通常只需要：
 *   1) 调一次 `ADS13_PowerOnInit()` 完成上电初始化
 *   2) 选择使用“阻塞读取”或“DMA 双缓冲流式读取”
 *
 * 说明：本工程主链路使用硬件 SPI1 + DMA（DRDY 触发）；软件 SPI 保留作备份。
 */

/* System Commands (16-bit) */
#define ADS131A04_NULL_COMMAND   0x0000
#define ADS131A04_RESET_COMMAND  0x0011
#define ADS131A04_STANDBY_COMMAND 0x0022
#define ADS131A04_WAKEUP_COMMAND 0x0033
#define ADS131A04_LOCK_COMMAND   0x0555
#define ADS131A04_UNLOCK_COMMAND 0x0655

/* 兼容旧驱动宏（ADS131A04.c 使用） */
#ifndef ADS131A04_UNCLOCK_COMMAND
#define ADS131A04_UNCLOCK_COMMAND ADS131A04_UNLOCK_COMMAND
#endif

#ifndef ADS_NULL
#define ADS_NULL ADS131A04_NULL_COMMAND
#endif

#ifndef READ_CMD
#define READ_CMD 0x20
#endif

#ifndef WRITE_CMD
#define WRITE_CMD 0x40
#endif

#ifndef ADS_RES_DEFINED
#define ADS_RES_DEFINED
typedef enum
{
	ADS_OK = 0,
	ADS_ERROR,
} ADS_res;
#endif

/* System Responses */
#define ADS131A04_READY 0xFF04

/* Register Addresses */
#define STAT_1   0x02
#define STAT_S   0x05
#define ERROR_CNT 0x06
#define STAT_M2  0x07
#define A_SYS_CFG 0x0B
#define D_SYS_CFG 0x0C
#define CLK1      0x0D
#define CLK2      0x0E
#define ADC_ENA   0x0F

#ifndef ADS_ADC1
#define ADS_ADC1 0x11
#endif
#ifndef ADS_ADC2
#define ADS_ADC2 0x12
#endif
#ifndef ADS_ADC3
#define ADS_ADC3 0x13
#endif
#ifndef ADS_ADC4
#define ADS_ADC4 0x14
#endif

#ifndef ADS_INCLK_DIV_2
#define ADS_INCLK_DIV_2 1
#endif
#ifndef ADS_INCLK_DIV_4
#define ADS_INCLK_DIV_4 2
#endif
#ifndef ADS_INCLK_DIV_6
#define ADS_INCLK_DIV_6 3
#endif
#ifndef ADS_INCLK_DIV_8
#define ADS_INCLK_DIV_8 4
#endif
#ifndef ADS_INCLK_DIV_10
#define ADS_INCLK_DIV_10 5
#endif
#ifndef ADS_INCLK_DIV_12
#define ADS_INCLK_DIV_12 6
#endif
#ifndef ADS_INCLK_DIV_14
#define ADS_INCLK_DIV_14 7
#endif

#ifndef ADS_OSR_DIV_4096
#define ADS_OSR_DIV_4096 0
#endif
#ifndef ADS_OSR_DIV_2048
#define ADS_OSR_DIV_2048 1
#endif
#ifndef ADS_OSR_DIV_1024
#define ADS_OSR_DIV_1024 2
#endif
#ifndef ADS_OSR_DIV_800
#define ADS_OSR_DIV_800 3
#endif
#ifndef ADS_OSR_DIV_768
#define ADS_OSR_DIV_768 4
#endif
#ifndef ADS_OSR_DIV_512
#define ADS_OSR_DIV_512 5
#endif
#ifndef ADS_OSR_DIV_400
#define ADS_OSR_DIV_400 6
#endif
#ifndef ADS_OSR_DIV_384
#define ADS_OSR_DIV_384 7
#endif
#ifndef ADS_OSR_DIV_256
#define ADS_OSR_DIV_256 8
#endif
#ifndef ADS_OSR_DIV_200
#define ADS_OSR_DIV_200 9
#endif
#ifndef ADS_OSR_DIV_192
#define ADS_OSR_DIV_192 10
#endif
#ifndef ADS_OSR_DIV_128
#define ADS_OSR_DIV_128 11
#endif
#ifndef ADS_OSR_DIV_96
#define ADS_OSR_DIV_96 12
#endif
#ifndef ADS_OSR_DIV_64
#define ADS_OSR_DIV_64 13
#endif
#ifndef ADS_OSR_DIV_48
#define ADS_OSR_DIV_48 14
#endif
#ifndef ADS_OSR_DIV_32
#define ADS_OSR_DIV_32 15
#endif

#ifndef ADC_ALL_ENABLE
#define ADC_ALL_ENABLE 0x0F
#endif

#ifndef ADCX_GAIN_1
#define ADCX_GAIN_1  0x00
#endif
#ifndef ADCX_GAIN_2
#define ADCX_GAIN_2  0x01
#endif
#ifndef ADCX_GAIN_4
#define ADCX_GAIN_4  0x02
#endif
#ifndef ADCX_GAIN_8
#define ADCX_GAIN_8  0x03
#endif
#ifndef ADCX_GAIN_16
#define ADCX_GAIN_16 0x04
#endif

/* RREG / WREG helper macros (see datasheet Table 13) */
#define ADS131_CMD_RREG(addr) (uint16_t)(0x2000u | (((uint16_t)(addr) & 0x1Fu) << 8) | 0x00u)
#define ADS131_CMD_WREG(addr, data) (uint16_t)(0x4000u | (((uint16_t)(addr) & 0x1Fu) << 8) | ((uint16_t)(data) & 0x00FFu))

/* STAT_S bits (see datasheet STAT_S register) */
#define ADS131_STAT_S_F_STARTUP (1u << 2)
#define ADS131_STAT_S_F_CS      (1u << 1)
#define ADS131_STAT_S_F_FRAME   (1u << 0)

/* STAT_1 lower bits (see datasheet STAT_1 register) */
#define ADS131_STAT_1_F_WDT     (1u << 3)
#define ADS131_STAT_1_F_RESYNC  (1u << 2)
#define ADS131_STAT_1_F_DRDY    (1u << 1)
#define ADS131_STAT_1_F_CHECK   (1u << 0)

typedef enum
{
	ADS131A04_SPS_1000  = 1000,
	ADS131A04_SPS_2000  = 2000,
	ADS131A04_SPS_4000  = 4000,
	ADS131A04_SPS_8000  = 8000,
	ADS131A04_SPS_16000 = 16000,
	ADS131A04_SPS_32000 = 32000,
	ADS131A04_SPS_64000 = 64000
} ADS131A04_Sps;

/* ---------------- DMA 环形缓冲采集（4096 帧环形，每 1024 帧打印一次） ----------------
 *
 * - 每次 DRDY（每帧）得到 4 通道各 1 个采样点
 * - 驱动内部维护 4096 帧环形缓冲，按 1024 帧切成 4 个 block（0/1/2/3）
 * - 主循环通过 ADS131A04_TakeBlock() 取出“满 1024 帧”的 block，然后批量处理/打印
 * - 打印期间 DMA 会继续写入其他 block；若打印过慢导致 4 个 block 都占用，将丢弃新数据（计数增加）
 *
 * 注意：打印 1024 行文本会很慢；如果采样率太高，可能来不及打印而发生 overrun。
 */
#define ADS131A04_BLOCK_FRAMES (1024u)
#define ADS131A04_RING_FRAMES  (4096u)
#define ADS131A04_RING_BLOCKS  (ADS131A04_RING_FRAMES / ADS131A04_BLOCK_FRAMES)

typedef struct
{
	int32_t ch[4]; /* 24-bit 二补码已符号扩展到 int32_t */
} ADS131A04_RawSample;

/* TIM2 双缓冲采集（与参考工程一致） */
extern float ADSA_B[4][1024];
extern float ADSA_B2[4][1024];
extern float ADS131A04_Buf[4];
extern int ADS131A04_flag;
extern int ADS131A04_flag2;
extern int number;
extern int number2;

void ADS13_PowerOnInit(void);
unsigned long Read_ADS131A0X_Value(float ADS[4]);
double DataFormatting(uint32_t Data, double Vref, uint8_t PGA);

/* DMA 双缓冲采集 */
void ADS131A04_StartDmaStreaming(void);
void ADS131A04_StopDmaStreaming(void);
uint8_t ADS131A04_GetLatestValues(float ADS[4]);

/* 取出一个“满 1024 帧”的块（不拷贝，返回指针）。
 * - 返回 1：成功取到 block；*out 指向长度为 ADS131A04_BLOCK_FRAMES 的数组
 * - 返回 0：当前没有满块
 */
uint8_t ADS131A04_TakeBlock(const ADS131A04_RawSample **out);

/* 释放由 ADS131A04_TakeBlock() 取到的块。
 * 目的：保证“打印块A期间采集块B”时，不会被 DMA 写指针覆盖正在打印的块。
 */
void ADS131A04_ReleaseBlock(const ADS131A04_RawSample *blk);

/* 运行期统计：块 overrun（主循环来不及取走满块，被采集覆盖）次数 */
uint32_t ADS131A04_GetBlockOverrunCount(void);

/* 将原始码值转换为电压（单位：V）。
 * ch_index: 0..3（用于保留你原工程的 CH3 经验校准系数）
 */
float ADS131A04_RawToVoltCh(uint8_t ch_index, int32_t code);

/* 采样率设置（CLKIN=16.384MHz，内部固定 CLK_DIV=2/ICLK_DIV=2） */
uint8_t ADS131A04_SetSampleRate(ADS131A04_Sps sps);

/* 可选：调试/诊断接口（更贴近手册） */
uint8_t ADS131A04_ReadReg(uint8_t addr, uint8_t *data);
uint8_t ADS131A04_WriteReg(uint8_t addr, uint8_t data, uint8_t *readback);
uint8_t ADS131A04_ReadStat1(uint8_t *stat1);
uint8_t ADS131A04_ReadStatS(uint8_t *stat_s);
uint8_t ADS131A04_ReadErrorCnt(uint8_t *err_cnt);
uint8_t ADS131A04_ReadStatM2(uint8_t *stat_m2);

#ifdef __cplusplus
}
#endif
#endif /* ADS131A04_EVB_H_ */

