#include "mcp_myspi.h"
#include "delay.h"
#include "main.h"

#if !USE_AD7606
void ADS131A0X_WaitDRDY(void);
void ADS131A0X_DelaySCLK(void);

void ADS131A0X_soft_spi(void)
{
	ADS131A04_CS_H;
	ADS131A04_SCLK_L;/* SPI总线空闲时，钟线是低电平 */
	ADS131A04_REST_H;
}

void ADS131A0X_DelaySCLK(void)
{
	uint16_t i;
	for (i = 0; i < 3; i++)
		;
}

uint8_t ADS131A0X_SendRecive_8Bit(uint8_t _data)
{

	uint8_t i;
	uint8_t read = 0;
	for (i = 0; i < 8; i++)
	{
		ADS131A04_SCLK_H;
		if (_data & 0x80)
		{
			ADS131A04_DIN_H;
		}
		else
		{
			ADS131A04_DIN_L;
		}
		_data = _data << 1;
		read = read << 1;
		ADS131A0X_DelaySCLK();
		ADS131A04_SCLK_L;
		read = read | READ_ADS131A04_DOUT;
		ADS131A0X_DelaySCLK();
	}
	return read;
}
#endif /* !USE_AD7606 */
