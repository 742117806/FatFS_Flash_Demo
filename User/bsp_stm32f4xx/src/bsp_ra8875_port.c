/*
*********************************************************************************************************
*
*	ģ������ : RA8875оƬ��MCU�ӿ�ģ��
*	�ļ����� : bsp_ra8875_port.c
*	��    �� : V1.6
*	˵    �� : RA8875�ײ�������������
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2013-02-01 armfly  ��ʽ����
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

#ifdef RA_SOFT_8080_8		/* ���ģ��8080���ߣ�8 bit */
	#error "STM32_V5 dont's surport soft 8080"
#endif

#ifdef RA_SOFT_SPI			/* ���ģSPI���� */
	#error "STM32_V5 dont's surport soft SPI"
#endif

#ifdef RA_HARD_SPI	/* Ӳ�� SPI ���� (��Ҫ�ı�RA8875���ϵ�2������λ��) */
	/*
	��1��������STM32-V5������,STM32F407IGT6
		PD3/LCD_BUSY		--- ����оƬæ       ��RA8875����RA8875оƬ��æ�ź�)
		PF6/LCD_PWM			--- LCD����PWM����  ��RA8875������˽ţ�������RA8875����)

		PI10/TP_NCS			--- ����оƬ��Ƭѡ		(RA8875������SPI�ӿڴ���оƬ��
		PB3/SPI3_SCK		--- ����оƬSPIʱ��		(RA8875������SPI�ӿڴ���оƬ��
		PB4/SPI3_MISO		--- ����оƬSPI������MISO(RA8875������SPI�ӿڴ���оƬ��
		PB5/SPI3_MOSI		--- ����оƬSPI������MOSI(RA8875������SPI�ӿڴ���оƬ��

		PC3/TP_INT			--- ����оƬ�ж� ������RA8875������RA8875������ж�)
	*/
	#define RA8875_CS_0()	GPIOI->BSRRH = GPIO_Pin_10
	#define RA8875_CS_1()	GPIOI->BSRRL = GPIO_Pin_10

	#define SPI_WRITE_DATA	0x00
	#define SPI_READ_DATA	0x40
	#define SPI_WRITE_CMD	0x80
	#define SPI_READ_STATUS	0xC0

	static uint8_t SPI_ShiftByte(uint8_t _ucByte);
#endif

#ifdef RA_HARD_8080_16		/* 8080Ӳ������ ��������RA8875��ȱʡģʽ�� */
	/*
		������STM32-V5��������߷�����

		PD0/FSMC_D2
		PD1/FSMC_D3
		PD4/FSMC_NOE		--- �������źţ�OE = Output Enable �� N ��ʾ����Ч
		PD5/FSMC_NWE		--- д�����źţ�WE = Output Enable �� N ��ʾ����Ч
		PD8/FSMC_D13
		PD9/FSMC_D14
		PD10/FSMC_D15
		PD13/FSMC_A18		--- ��ַ RS
		PD14/FSMC_D0
		PD15/FSMC_D1

		PE4/FSMC_A20		--- ����Ƭѡһ������
		PE5/FSMC_A21		--- ����Ƭѡһ������
		PE7/FSMC_D4
		PE8/FSMC_D5
		PE9/FSMC_D6
		PE10/FSMC_D7
		PE11/FSMC_D8
		PE12/FSMC_D9
		PE13/FSMC_D10
		PE14/FSMC_D11
		PE15/FSMC_D12

		PG12/FSMC_NE4		--- ��Ƭѡ��TFT, OLED �� AD7606��

		PC3/TP_INT			--- ����оƬ�ж� ������RA8875������RA8875������ж�)

		---- ������ TFT LCD�ӿ������ź� ��FSMCģʽ��ʹ�ã�----
		PD3/LCD_BUSY		--- ����оƬæ       ��RA8875����RA8875оƬ��æ�ź�)
		PF6/LCD_PWM			--- LCD����PWM����  ��RA8875������˽ţ�������RA8875����)

		PI10/TP_NCS			--- ����оƬ��Ƭѡ		(RA8875������SPI�ӿڴ���оƬ��
		PB3/SPI3_SCK		--- ����оƬSPIʱ��		(RA8875������SPI�ӿڴ���оƬ��
		PB4/SPI3_MISO		--- ����оƬSPI������MISO(RA8875������SPI�ӿڴ���оƬ��
		PB5/SPI3_MOSI		--- ����оƬSPI������MOSI(RA8875������SPI�ӿڴ���оƬ��
	*/
	#define RA8875_BASE		((uint32_t)(0x6C000000 | 0x00000000))

	#define RA8875_REG		*(__IO uint16_t *)(RA8875_BASE +  (1 << (18 + 1)))	/* FSMC 16λ����ģʽ�£�FSMC_A18���߶�Ӧ�����ַA19 */
	#define RA8875_RAM		*(__IO uint16_t *)(RA8875_BASE)

	#define RA8875_RAM_ADDR		RA8875_BASE
#endif

/*
*********************************************************************************************************
*	�� �� ��: RA8875_Delaly1us
*	����˵��: �ӳٺ���, ��׼, ��Ҫ����RA8875 PLL����֮ǰ����ָ�����ӳ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8875_Delaly1us(void)
{
	uint8_t i;

	for (i = 0; i < 10; i++);	/* �ӳ�, ��׼ */
}

/*
*********************************************************************************************************
*	�� �� ��: RA8875_Delaly1ms
*	����˵��: �ӳٺ���.  ��Ҫ����RA8875 PLL����֮ǰ����ָ�����ӳ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8875_Delaly1ms(void)
{
	uint16_t i;

	for (i = 0; i < 5000; i++);	/* �ӳ�, ��׼ */
}

/*
*********************************************************************************************************
*	�� �� ��: RA8875_WriteCmd
*	����˵��: дRA8875ָ��Ĵ���
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8875_WriteCmd(uint8_t _ucRegAddr)
{
#ifdef RA_HARD_SPI	/* Ӳ��SPI�ӿ� */
	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_CMD);
	SPI_ShiftByte(_ucRegAddr);
	RA8875_CS_1();
#endif

#ifdef RA_HARD_8080_16			/* 8080Ӳ������ */
	RA8875_REG = _ucRegAddr;	/* ���üĴ�����ַ */
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: RA8875_WriteData
*	����˵��: дRA8875ָ��Ĵ���
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8875_WriteData(uint8_t _ucRegValue)
{
#ifdef RA_HARD_SPI	/* Ӳ��SPI�ӿ� */
	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_DATA);
	SPI_ShiftByte(_ucRegValue);
	RA8875_CS_1();
#endif

#ifdef RA_HARD_8080_16			/* 8080Ӳ������ */
	RA8875_RAM = _ucRegValue;	/* ���üĴ�����ַ */
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: RA8875_ReadData
*	����˵��: ��RA8875�Ĵ���ֵ
*	��    ��: ��
*	�� �� ֵ: �Ĵ���ֵ
*********************************************************************************************************
*/
uint8_t RA8875_ReadData(void)
{
#ifdef RA_HARD_SPI	/* Ӳ��SPI�ӿ� */
	uint16_t value;

	RA8875_CS_0();
	SPI_ShiftByte(SPI_READ_DATA);
	value = SPI_ShiftByte(0x00);
	RA8875_CS_1();

	return value;
#endif

#ifdef RA_HARD_8080_16			/* 8080Ӳ������ */
	uint8_t value;

	value = RA8875_RAM;		/* ��ȡ�Ĵ���ֵ */

	return value;
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: RA8875_WriteData16
*	����˵��: дRA8875�������ߣ�16bit������RGB�Դ�д��
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8875_WriteData16(uint16_t _usRGB)
{
#ifdef RA_HARD_SPI	/* Ӳ��SPI�ӿ� */
	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_DATA);
	SPI_ShiftByte(_usRGB >> 8);
	RA8875_CS_1();

	RA8875_CS_0();
	SPI_ShiftByte(SPI_WRITE_DATA);
	SPI_ShiftByte(_usRGB);
	RA8875_CS_1();
#endif

#ifdef RA_HARD_8080_16		/* 8080Ӳ������ */
	RA8875_RAM = _usRGB;	/* ���üĴ�����ַ */
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: RA8875_ReadData16
*	����˵��: ��RA8875�Դ棬16bit RGB
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint16_t RA8875_ReadData16(void)
{
#ifdef RA_HARD_SPI	/* Ӳ��SPI�ӿ� */
	uint16_t value;

	RA8875_CS_0();
	SPI_ShiftByte(SPI_READ_DATA);
	value = SPI_ShiftByte(0x00);
	value <<= 8;
	value += SPI_ShiftByte(0x00);
	RA8875_CS_1();

	return value;
#endif

#ifdef RA_HARD_8080_16			/* 8080Ӳ������ */
	uint16_t value;

	value = RA8875_RAM;		/* ��ȡ�Ĵ���ֵ */

	return value;
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: RA8875_ReadStatus
*	����˵��: ��RA8875״̬�Ĵ���
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint8_t RA8875_ReadStatus(void)
{
#ifdef RA_HARD_SPI	/* Ӳ��SPI�ӿ� */
	uint8_t value;

	RA8875_CS_0();
	SPI_ShiftByte(SPI_READ_STATUS);
	value = SPI_ShiftByte(0x00);
	RA8875_CS_1();

	return value;
#endif

#ifdef RA_HARD_8080_16			/* 8080Ӳ������ */
	uint8_t value;

	value = RA8875_REG;

	return value;
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: RA8875_GetDispMemAddr
*	����˵��: ����Դ��ַ��
*	��    ��: ��
*	�� �� ֵ: �Դ��ַ
*********************************************************************************************************
*/
uint32_t RA8875_GetDispMemAddr(void)
{
	#ifdef RA_HARD_8080_16		/* 8080Ӳ������ ��������RA8875��ȱʡģʽ�� */
		return RA8875_RAM_ADDR;
	
	#else
		return 0;
	#endif
}

#ifdef RA_HARD_SPI	   /* ����SPI�ӿ�ģʽ */

/*
*********************************************************************************************************
*	�� �� ��: RA8875_InitSPI
*	����˵��: ����STM32��RA8875��SPI���ߣ�ʹ��Ӳ��SPI1, Ƭѡ���������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8875_InitSPI(void)
{
	/*
		������STM32-V5 ��������߷��䣺  ����Flash�ͺ�Ϊ W25Q64BVSSIG (80MHz)
		PB3/SPI3_SCK
		PB4/SPI3_MISO
		PB5/SPI3_MOSI
		PI10/TP_NCS			--- ����оƬ��Ƭѡ		(RA8875������SPI�ӿڴ���оƬ��

		STM32Ӳ��SPI�ӿ� = SPI3
	*/
	{
		GPIO_InitTypeDef GPIO_InitStructure;

		/* ʹ��GPIO ʱ�� */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOF, ENABLE);

		/* ���� SCK, MISO �� MOSI Ϊ���ù��� */
		//GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI3);
		//GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI3);
		//GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI3);
		/* ���� SCK, MISO �� MOSI Ϊ���ù��� */
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		/* ����Ƭѡ����Ϊ�������ģʽ */
		RA8875_CS_1();		/* Ƭѡ�øߣ���ѡ�� */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
		GPIO_Init(GPIOF, &GPIO_InitStructure);
	}
	
	RA8875_LowSpeedSPI();
}

/*
*********************************************************************************************************
*	�� �� ��: RA8875_LowSpeedSPI
*	����˵��: ����STM32�ڲ�SPIӲ���Ĺ���ģʽ���ٶȵȲ��������ڷ���SPI�ӿڵ�RA8875. ���١�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8875_LowSpeedSPI(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* ��SPIʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/* ����SPIӲ������ */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* ���ݷ���2��ȫ˫�� */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* STM32��SPI����ģʽ ������ģʽ */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* ����λ���� �� 8λ */
	/* SPI_CPOL��SPI_CPHA���ʹ�þ���ʱ�Ӻ����ݲ��������λ��ϵ��
	   ��������: ���߿����Ǹߵ�ƽ,��2�����أ������ز�������)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* ʱ�������ز������� */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* ʱ�ӵĵ�2�����ز������� */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			/* Ƭѡ���Ʒ�ʽ��������� */

	/* ���ò�����Ԥ��Ƶϵ�� */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* ����λ������򣺸�λ�ȴ� */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC����ʽ�Ĵ�������λ��Ϊ7�������̲��� */
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, DISABLE);			/* �Ƚ�ֹSPI  */

	SPI_Cmd(SPI1, ENABLE);				/* ʹ��SPI  */
}

/*
*********************************************************************************************************
*	�� �� ��: RA8875_HighSpeedSPI
*	����˵��: ����STM32�ڲ�SPIӲ���Ĺ���ģʽ���ٶȵȲ��������ڷ���SPI�ӿڵ�RA8875.
*			  ��ʼ����ɺ󣬿��Խ�SPI�л�������ģʽ�������ˢ��Ч�ʡ�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RA8875_HighSpeedSPI(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* ��SPIʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/* ����SPIӲ������ */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* ���ݷ���2��ȫ˫�� */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* STM32��SPI����ģʽ ������ģʽ */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* ����λ���� �� 8λ */
	/* SPI_CPOL��SPI_CPHA���ʹ�þ���ʱ�Ӻ����ݲ��������λ��ϵ��
	   ��������: ���߿����Ǹߵ�ƽ,��2�����أ������ز�������)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* ʱ�������ز������� */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* ʱ�ӵĵ�2�����ز������� */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			/* Ƭѡ���Ʒ�ʽ��������� */

	/* ���ò�����Ԥ��Ƶϵ�� */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* ����λ������򣺸�λ�ȴ� */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC����ʽ�Ĵ�������λ��Ϊ7�������̲��� */
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, DISABLE);			/* �Ƚ�ֹSPI  */

	SPI_Cmd(SPI1, ENABLE);				/* ʹ��SPI  */
}

/*
*********************************************************************************************************
*	�� �� ��: SPI_ShiftByte
*	����˵��: ��SPI���߷���һ���ֽڣ�ͬʱ���ؽ��յ����ֽ�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static uint8_t SPI_ShiftByte(uint8_t _ucByte)
{
	uint8_t ucRxByte;

	/* �ȴ����ͻ������� */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* ����һ���ֽ� */
	SPI_I2S_SendData(SPI1, _ucByte);

	/* �ȴ����ݽ������ */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* ��ȡ���յ������� */
	ucRxByte = SPI_I2S_ReceiveData(SPI1);

	/* ���ض��������� */
	return ucRxByte;
}

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
