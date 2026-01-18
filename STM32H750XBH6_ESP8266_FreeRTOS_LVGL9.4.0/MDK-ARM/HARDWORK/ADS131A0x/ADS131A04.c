#include "ADS131A04_EVB.h"
#include "delay.h"
#include "main.h"
#include "stdio.h"

float ADS131A04_value[1000];
int ADS131A04_flag;

float ADS131A04_Buf[4] = {0};
int AD_i = 0;
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
void ADS131A0X_Init(void)
{
	ADS131A0X_soft_spi();
}
void ADS131A0X_Delay(void)
{
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
unsigned char ADS13_SPI(unsigned char com)
{
	return (ADS131A0X_SendRecive_8Bit(com));
}

/* write a command to ADS131a02 */
uint16_t ADS13_WRITE_CMD(uint16_t command)
{
	uint16_t receive = 0x0000;
	unsigned int i;
	ADS131A04_CS_L;
//	delay_us(1);
//ADS131A0X_Delay();
	receive = ADS13_SPI((uint8_t)(command >> 8));
	receive <<= 8;
	receive |= ADS13_SPI((uint8_t)(command));
	ADS13_SPI(0X00); // 填满3个字节   24bit
	ADS13_SPI(0X00); // 填满4个字节   32bit
	ADS131A04_CS_H;
//		delay_us(1);
//ADS131A0X_Delay();
	return receive; // 返回接收的数据
}

/*****************************************************************************************
 * Function Name: ADS13_REG
 * Description  : 对ADS131内部寄存器进行操作
 * Arguments    : com:读写寄存器地址，data：需写入的数据
 * Return Value : 返回读取的数据
 ******************************************************************************************/
uint16_t ADS13_REG(unsigned char com, unsigned data)
{
	unsigned int i;
	uint16_t temp = 0X0000;
	if ((com & 0x20) == 0x20) // 判断是否为读寄存器指令 if里面为读
	{
		ADS13_WRITE_CMD(com);
		temp = ADS13_WRITE_CMD(ADS131A04_NULL_COMMAND);
	}
	else // 写寄存器
	{
		ADS131A04_CS_L;
//		delay_us(1);
//ADS131A0X_Delay();
		ADS13_SPI(com);
		ADS13_SPI(data);
		ADS13_SPI(0X00); // 补齐0  24bit
		ADS13_SPI(0X00); // 补齐0  32bit
		//delay_us(1);
//ADS131A0X_Delay();
		ADS131A04_CS_H;
	}
	    //delay_us(1);
	return temp;
}

/*****************************************************************************************
 * Function Name: ADS13_PowerOnInit
 * Description  : ADS13上电复位
 * Arguments    : NONE
 * Return Value : NONE
 ******************************************************************************************/
int adcenable_flag = 0;
int Dev_ID;
void ADS13_PowerOnInit(void)
{
	uint8_t RREG = 0X20;	   // 读
	uint8_t WREG = 0X40;	   // 写
	uint16_t RECEIVE = 0X0000; // 接收spi返回的字符
	uint16_t ID_M, ID_L;
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

	ADS13_REG(WREG | A_SYS_CFG, 0X60);			// External reference voltage
	ADS13_REG(WREG | D_SYS_CFG, 0X3E);			// Fixed six device words per frame for the ADS131A04
	ADS13_REG(WREG | CLK1, 0X01);				// ADC CLK1  fICLK = fCLKIN(16.384mhz) / 1
	ADS13_REG(WREG | CLK2,0x0F);                // fMOD = fICLK / 1  fICLK = fCLKIN / 4096
	ADS13_REG(WREG | ADC_ENA, 0X0F);			// ADC CHANNEL ENABLE ALL

	ADS13_WRITE_CMD(0x2B00); // read A_SYS_CFG
	RECEIVE = ADS13_WRITE_CMD(ADS131A04_NULL_COMMAND);

	ADS13_WRITE_CMD(0x2C00); // read D_SYS_CFG
	RECEIVE = ADS13_WRITE_CMD(ADS131A04_NULL_COMMAND);

	ADS13_WRITE_CMD(ADS131A04_WAKEUP_COMMAND); // WAKEUP ADS131
	RECEIVE = ADS13_WRITE_CMD(ADS131A04_NULL_COMMAND);

	adcenable_flag = 1; // 初始化完毕标志位
}

uint8_t ADC_READY(void)
{
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
void Read_ADS131A0X_BUFFER_DATA(void)
{
	uint8_t i = 0;
	ADS131A04_CS_L;
	for (i = 0; i < 24; i++)
	{
		recBuffer[i] = ADS13_SPI(dummy_send[i]);
	}
	ADS131A04_CS_H;
}

unsigned long Read_ADS131A0X_Value(float ADS[]) // 读取ADC
{
	unsigned long value = 0;
	union
	{
		unsigned long voltage;
		struct
		{
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
	ADS[2] = DataFormatting(volt2.voltage, 2.5f * 2.0f, 1);
	ADS[3] = DataFormatting(volt3.voltage, 2.5f * 2.0f, 1);
	return 1;
}

// 添加1
uint32_t ADS131A04_Write_Command(uint32_t command)
{
	uint32_t rev = 0;
	uint32_t cmd;
	cmd = (uint32_t)command << 16;
	ADS13_WRITE_CMD(command);
	return rev;
}
// 添加2
uint32_t ADS131A04_Write_Reg(uint8_t reg_add, uint8_t data)
{
	uint32_t temp = 0;
	uint32_t volatile com_t = 0;
	com_t = ((uint32_t)reg_add << 24) + ((uint32_t)data << 16);
	if ((reg_add & 0x20) == 0x20)
	{
		ADS131A04_Write_Command(((uint32_t)reg_add << 24) >> 16);
		temp = ADS131A04_Write_Command(ADS_NULL);
	}
	else
	{
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
ADS_res ADS131A04_MODULATION_SET_PREDIV(uint8_t pre_divider_mod, uint8_t pre_divider_osr)
{
	uint8_t temp = 0x00;
	uint32_t rev = ADS131A04_Write_Reg(WRITE_CMD | CLK2, (pre_divider_mod << 5) | pre_divider_osr);
	printf("\n\rMODULATION_SET_PREDIV接收到的回复:%x\n\r", rev);
	return ADS_OK;
}
/*-------------------------------------------------------------
  *  @brief     设置ADC同步模式时的预分频系数
  *  @param     pre_divider     预分频参数，详细见参数列表
  *             ADS_INCLK_DIV_2,ADS_INCLK_DIV_4,ADS_INCLK_DIV_6,ADS_INCLK_DIV_8,ADS_INCLK_DIV_10,ADS_INCLK_DIV_12,ADS_INCLK_DIV_14
  *  @return    已置位数据
  *  @note      none
  *  @Sample usage:     ADS131A04_ICLK_SET_PREDIV(INCLK_DIV_4)
 -------------------------------------------------------------*/
ADS_res ADS131A04_ICLK_SET_PREDIV(uint8_t pre_divider)
{
	uint8_t temp = 0x00;
	uint32_t rev = ADS131A04_Write_Reg(WRITE_CMD | CLK1, temp | (pre_divider << 1));
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
ADS_res ADS131A04_REF_SET(uint8_t ref_mode, uint8_t ref_source)
{
	uint8_t temp = 0x00;
	switch (ref_mode)
	{
	case 0:
		temp = temp | 0x00;
		break;
	case 1:
		temp = temp | 0x10;
		break;
	default:
		return ADS_ERROR;
	}

	switch (ref_source)
	{
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
ADS_res ADS131A04_Gain_SET(uint8_t ch, uint8_t gain)
{
	ADS131A04_Write_Reg(WRITE_CMD | ch, gain);
	return ADS_OK;
}

ADS_res
ADS131A04_Power_Up(void)
{
	uint32_t rev = 0;
	rev = ADS131A04_Write_Command(RESET); // 重置
	while (rev != 0xff040000)
	{
		printf("\n\r上电重置接收到的回复:%x\n\r", rev);
		rev = ADS131A04_Write_Command(RESET); // 接收READY信号
	}
	printf("\n\r已接收到READY\n\r");
	rev = ADS131A04_Write_Command(UNLOCK); // 解锁
	printf("\n\rUNLOCK接收到的回复:%x\n\r", rev);
	ADS131A04_REF_SET(0, 0); // 设置参考电压
	rev = ADS131A04_Write_Reg(READ_CMD | A_SYS_CFG, 0);
	printf("\n\rREAD:%x\n\r", rev);
	ADS131A04_ICLK_SET_PREDIV(ADS_INCLK_DIV_2);						   // 设置输入时钟预分频
	ADS131A04_MODULATION_SET_PREDIV(ADS_INCLK_DIV_2, ADS_OSR_DIV_400); // 设置调制时钟预分频与过采样率预分频

	rev = ADS131A04_Gain_SET(ADS_ADC1, ADCX_GAIN_1);
	printf("\n\rREAD:%x\n\r", rev);
	rev = ADS131A04_Gain_SET(ADS_ADC2, ADCX_GAIN_1);
	printf("\n\rREAD:%x\n\r", rev);
	// 10kHz采样率
	rev = ADS131A04_Write_Reg(WRITE_CMD | ADC_ENA, 0X00 | ADC_ALL_ENABLE); // 开启使能
	printf("\n\r开启使能接收到的回复:%x\n\r", rev);
	rev = ADS131A04_Write_Command(WAKEUP); // 唤醒
	printf("\n\rWAKEUP接收到的回复:%x\n\r", rev);

	return ADS_OK;
}

void swap(float *a, float *b)
{
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
	for (i = 0; i < leng; i++)
	{
		for (j = i + 1; j < leng; j++)
		{
			if (dat[i] > dat[j])
			{
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
double DataFormatting(uint32_t Data, double Vref, uint8_t PGA)
{
	/*
	电压计算公式；
			设：AD采样的电压为Vin ,AD采样二进制值为X，参考电压为 Vr ,内部集成运放增益为G
			Vin = ( (Vr) / G ) * ( x / (2^23 -1))
	*/
	double ReadVoltage;
	if (Data & 0x00800000)
	{
		Data = (~Data) & 0x00FFFFFF;
		ReadVoltage = -(((double)Data) / 8388607) * ((Vref) / ((double)PGA));
	}
	else
		ReadVoltage = (((double)Data) / 8388607) * ((Vref) / ((double)PGA));

	return (ReadVoltage);
}
