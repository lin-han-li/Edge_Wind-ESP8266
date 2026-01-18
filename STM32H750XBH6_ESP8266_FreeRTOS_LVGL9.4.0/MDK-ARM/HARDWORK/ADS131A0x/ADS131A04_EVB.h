#ifndef _ads131a04_h_
#define _ads131a04_h_

#include "main.h"
#include "mcp_myspi.h"

/*ADS131A04 Definitions*/
extern float ADS131A04_value[1000];
extern int ADS131A04_flag;

extern float ADS131A04_Buf[4];
extern int AD_i;

// Read/Write Registers OPCODES
#define ADS131A04_READ_REGISTER_COMMAND_OPCODE 0x2000
#define ADS131A04_READ_MULTIPLE_REGISTERS_COMMAND_OPCODE 0x2000
#define ADS131A04_WRITE_REGISTER_COMMAND_OPCODE 0x4000
#define ADS131A04_WRITE_MULTIPLE_REGISTERS_COMMAND_OPCODE 0x6000

#define ADS131A04_READ_REGISTER_RESPONSE_OPCODE 0x2000
#define ADS131A04_READ_MULTIPLE_REGISTERS_RESPONSE_OPCODE 0x6000
#define ADS131A04_WRITE_REGISTER_RESPONSE_OPCODE 0x2000
#define ADS131A04_WRITE_MULTIPLE_REGISTERS_RESPONSE_OPCODE 0x4000

// System Commands
#define ADS131A04_NULL_COMMAND 0X0000
#define ADS131A04_RESET_COMMAND 0X0011
#define ADS131A04_STANDBY_COMMAND 0X0022
#define ADS131A04_WAKEUP_COMMAND 0X0033
#define ADS131A04_LOCK_COMMAND 0X0555
#define ADS131A04_UNCLOCK_COMMAND 0X0655

#define READ_REGISTER_COMMAND(address, register_count) (ADS131A04_READ_REGISTER_COMMAND_OPCODE | (address << 8) | register_count)
// Please note that register_count tells A04 device register_count+1 registers to read
#define READ_MULTIPLE_REGISTERS_COMMAND(address, register_count) (ADS131A04_READ_MULTIPLE_REGISTERS_COMMAND_OPCODE | (address << 8) | register_count)
#define WRITE_REGISTER_COMMAND(address, data) (ADS131A04_WRITE_REGISTER_COMMAND_OPCODE | (address << 8) | data)
#define WRITE_MULTIPLE_REGISTERS_COMMAND(address, register_count) (ADS131A04_WRITE_MULTIPLE_REGISTERS_COMMAND_OPCODE | (address << 8) | register_count)
#define WRITE_MULTIPLE_REGISTERS_ADDITIONAL_WORD(data1, data2) ((data1 << 8) | data2)
// data1 is written to a register at adress N.  data2 is written to next register at address N+1.

// system responses
#define ADS131A04_READY 0xFF04
#define ADS131A04_STANDBY_ACK 0X0022
#define ADS131A04_WAKEUP_ACK 0X0033
#define ADS131A04_LOCK_ACK 0X0555
#define ADS131A04_UNCLOCK_ACK 0X0655

#define WRITE_MULTIPLE_REGISTERS_ACK(address, register_count) (ADS131A04_WRITE_MULTIPLE_REGISTERS_RESPONSE_OPCODE | (address << 8) | register_count)
#define WRITE_REGISTER_ACK(address, data) (ADS131A04_WRITE_REGISTER_RESPONSE_OPCODE | (address << 8) | data)
#define READ_REGISTER_RESPONSE_PARSE(response, op_code, address, data) \
	{                                                                  \
		op_code = (response & 0xE000);                                 \
		address = ((response & 0x1F00) >> 8);                          \
		data = (response & 0x00FF);                                    \
	}
#define READ_MULTIPLE_REGISTERS_ACK(address, register_count) (ADS131A04_READ_MULTIPLE_REGISTERS_RESPONSE_OPCODE | (address << 8) | register_count)
#define READ_MULTIPLE_REGISTER_RESPONSE_PARSE(response, data1, data2) \
	{                                                                 \
		data1 = ((response & 0xFF00) >> 8);                           \
		data2 = (response & 0x00FF);                                  \
	}

/* ADS131A0x Register Settings Export */
/******************************************************************************/
/* This file contains the register map settings stub */
// General defines
#define ADS131A0x_REGISTER_COUNT 17
/* Register #define values (register address and value) */
/******************************************************************************/
/* This section contains the defines for register address and register settings */
// Register Addresses
#define ID_MSB 0x00	   // ID Control Register MSB
#define ID_LSB 0x01	   // ID Control Register LSB
#define STAT_1 0x02	   // Status 1 Register
#define STAT_P 0x03	   // Positive Input Fault Detect Status
#define STAT_N 0x04	   // Negative Input Fault Detect Status
#define STAT_S 0x05	   // SPI Status Register
#define ERROR_CNT 0x06 // Error Count Register
#define STAT_M2 0x07   // Hardware Mode Pin Status Register
// #define RESERVED														0x08
// #define RESERVED														0x09
// #define RESERVED														0x0A
#define A_SYS_CFG 0x0B // Analog System Configuration Register
#define D_SYS_CFG 0x0C // Digital System Configuration Register
#define CLK1 0x0D	   // Clock Configuration 1 Register
#define CLK2 0x0E	   // Clock Configuration 2 Register
#define ADC_ENA 0x0F   // ADC Channel Enable Register
// #define RESERVED														0x10
#define ADS131_ADC1 0x11 // ADC Channel 1 Digital Gain Configuration
#define ADS131_ADC2 0x12 // ADC Channel 2 Digital Gain Configuration
#define ADS131_ADC3 0x13 // ADC Channel 3 Digital Gain Configuration
#define ADS131_ADC4 0x14 // ADC Channel 4 Digital Gain Configuration

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/*****************************************************
 * Local Prototypes
 *****************************************************/
void ADS13_PowerOnInit(void);
unsigned long Read_ADS131A0X_Value(float ADS[]);
double DataFormatting(uint32_t Data, double Vref, uint8_t PGA);




//添加
typedef enum  //返回值
{
    ADS_OK = 0,
    ADS_ERROR,
}ADS_res;

/*-----------------------HELP-----------------------*/
// Fetch the upper byte of a 32-bit word and return it as an 16-bit value
#define UPPER_BYTE(x)           ((uint16_t) ((0xFFFF0000 & x) >> 16))

// Fetch the lower byte of a 32-bit word and return it as an 16-bit value
#define LOWER_BYTE(x)           ((uint16_t)  (0x0000FFFF & x))

// Combine two 8-bit values into a 16-bit word
#define COMBINE_BYTES(x, y)     (((uint32_t) x << 16) | ((uint32_t) y & 0x0000FFFF))

/*-----------------------COMMAND-----------------------*/
// SYSTEM CAMMAND 基本上都是16bit
#define ADS_NULL 0x0000   // NULL
#define RESET 0x0011   // 复位为POR值
#define STANDBY 0x0022 // 进入备用模式
#define WAKEUP 0x0033  // 退出备用模式
#define LOCK 0x0555    // 锁定ADC寄存器
#define UNLOCK 0x0655  // 解锁

#define READ_CMD    0x20
#define WRITE_CMD    0x40
/*-----------------------ADDRESS-----------------------*/

#define STAT_1   0x02       //寄存错误的寄存器
#define STAT_P   0x03       //该寄存器存储每个通道上的正输入是否超过COMP_TH[2:0]位设置的阈值的状态
#define STAT_N   0x04       //该寄存器存储每个通道上的负输入是否超过COMP_TH[2:0]位设置的阈值的状态
#define STAT_S   0x05       //这个寄存器表示SPI故障条件的检测
#define ERROR_CNT   0x06    //该寄存器对Hamming和CRC错误进行计数。这个寄存器在读取时被清除
#define STAT_M2   0x07      //这个寄存器表示检测硬件模式引脚的捕获状态
#define A_SYS_CFG   0x0B    //该寄存器配置ADS131A0x的模拟特性，重要
#define D_SYS_CFG   0x0C
#define CLK1   0x0D
#define CLK2   0x0E
#define ADC_ENA   0x0F


#define ADS_ADC1   0x11
#define ADS_ADC2   0x12
#define ADS_ADC3   0x13
#define ADS_ADC4   0x14
/*-----------------------GPIO-----------------------*/
#define DRDY_GPIO_Port  GPIOA
#define DRDY_GPIO_Pin   GPIO_PIN_8


/*-----------------------寄存器参数-----------------------*/
#define ADS_INCLK_DIV_2 1
#define ADS_INCLK_DIV_4 2
#define ADS_INCLK_DIV_6 3
#define ADS_INCLK_DIV_8 4
#define ADS_INCLK_DIV_10 5
#define ADS_INCLK_DIV_12 6
#define ADS_INCLK_DIV_14 7

#define ADS_OSR_DIV_4096 0
#define ADS_OSR_DIV_2048 1
#define ADS_OSR_DIV_1024 2
#define ADS_OSR_DIV_800 3
#define ADS_OSR_DIV_768 4
#define ADS_OSR_DIV_512 5
#define ADS_OSR_DIV_400 6
#define ADS_OSR_DIV_384 7
#define ADS_OSR_DIV_256 8
#define ADS_OSR_DIV_200 9
#define ADS_OSR_DIV_192 10
#define ADS_OSR_DIV_128 11
#define ADS_OSR_DIV_96 12
#define ADS_OSR_DIV_64 13
#define ADS_OSR_DIV_48 14
#define ADS_OSR_DIV_32 15

#define ADC_ALL_ENABLE 0X0F
#define ADC_ALL_DISABLE 0

#define ADCX_GAIN_1  0x00
#define ADCX_GAIN_2  0x01
#define ADCX_GAIN_4  0x02
#define ADCX_GAIN_8  0x03
#define ADCX_GAIN_16 0x04
/*
读取地址为a aaaa的单个寄存器
DEVICE WORD                             (001a aaaa 0000 0000)b
COMMAND STATUS RESPONSE                 REG

RREGS读取(nnnn nnnn + 1)从地址a aaaa开始的寄存器
DEVICE WORD                             (001a aaaa nnnn nnnn)b
COMMAND STATUS RESPONSE                 RREGS

写一个地址为aaaa的单寄存器，数据为dddd dddd
DEVICE WORD                             (010a aaaa dddd dddd)b
COMMAND STATUS RESPONSE                 REG (updated register)


WREGS Write (nnnn nnnn + 1) registers beginning at address a aaaa. Additional device words are required to send data (dddd dddd) to register
address (a) and data (eeee eeee) to register address (a+1). Each device word contains data for two registers.
The data frame size is extended by (n / 2) device words to allow for command completion.
写(nnnn nnnn + 1)从地址a aaaa开始的寄存器。发送数据(dddd dddd)注册需要额外的设备字地址(a)和数据(eeee eeee)注册地址(a+i)。每个设备字包含两个寄存器的数据。数据帧的大小由(n / 2)个设备字扩展，以允许命令完成。

DEVICE WORD                             (011a aaaa nnnn nnnn)b
ADDITIONAL                              DEVICE WORD (dddd dddd eeee eeee)b
COMMAND STATUS RESPONSE                 ACK =(010a_aaaa_nnnn_nnnn)b

*/
ADS_res
ADS131A04_Power_Up(void);




#endif
