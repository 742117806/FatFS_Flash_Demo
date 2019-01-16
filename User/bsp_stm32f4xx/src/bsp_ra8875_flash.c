/*
*********************************************************************************************************
*
*	ģ������ : RA8875оƬ��ҵĴ���Flash����ģ��
*	�ļ����� : bsp_ra8875_flash.c
*	��    �� : V1.3
*	˵    �� : ����RA8875��ҵĴ���Flash ���ֿ�оƬ��ͼ��оƬ����֧�� SST25VF016B��MX25L1606E ��
*			   W25Q64BVSSIG�� ͨ��TFT��ʾ�ӿ���SPI���ߺ�PWM���߿���7�������ϵĴ���Flash��
*				����ע�� RA8875����֧����Ҵ���Flash��д�������������Ӷ���ĵ��ӿ��ص�·����ʵ�֡�
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		V1.0    2012-06-25 armfly  �����װ�
*
*	Copyright (C), 2011-2012, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
	STM32F4XX ʱ�Ӽ���.
		HCLK = 168M
		PCLK1 = HCLK / 4 = 42M
		PCLK2 = HCLK / 2 = 84M

		SPI2��SPI3 �� PCLK1, ʱ��42M
		SPI1       �� PCLK2, ʱ��84M

		STM32F4 ֧�ֵ����SPIʱ��Ϊ 37.5 Mbits/S, �����Ҫ��Ƶ��
*/

#ifdef STM32_X3 	/* ������ STM32-X3 ������ */
	/*
		������STM32-X4 ���߷��䣺 ����Flash�ͺ�Ϊ W25Q64BVSSIG (80MHz)
		PB12 = CS
		PB13 = SCK
		PB14 = MISO
		PB15 = MOSI

		STM32Ӳ��SPI�ӿ� = SPI2
	*/
	#define SPI_FLASH			SPI2

	#define ENABLE_SPI_RCC() 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI2, ENABLE)

	#define SPI_BAUD			SPI_BaudRatePrescaler_1		/* ѡ��2��Ƶʱ, SCKʱ�� = 21M */

	/* Ƭѡ�����õ�ѡ��  */
	#define W25_CS_LOW()		GPIOB->BSRRH = GPIO_Pin_12

	/* Ƭѡ�����ø߲�ѡ�� */
	#define W25_CS_HIGH()		GPIOB->BSRRL = GPIO_Pin_12
#else
	/*
		������STM32-V5 ������TFT�ӿ��е�SPI���߷��䣺 ����Flash�ͺ�Ϊ W25Q64BVSSIG (80MHz)
		PB3/SPI3_SCK/SPI1_SCK
		PB4/SPI3_MISO/SPI1_MISO
		PB5/SPI3_MOSI/SPI1_MOSI
		PI10/TP_NCS

		STM32Ӳ��SPI�ӿ� = SPI3 ���� SPI1

		����SPI1��ʱ��Դ��84M, SPI3��ʱ��Դ��42M��Ϊ�˻�ø�����ٶȣ������ѡ��SPI1��
	*/
	//#define SPI_FLASH			SPI3
	#define SPI_FLASH			SPI1

	//#define ENABLE_SPI_RCC() 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE)
	#define ENABLE_SPI_RCC() 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE)

	/*
		��SPIʱ�������2��Ƶ����֧�ֲ���Ƶ��
		�����SPI1��2��ƵʱSCKʱ�� = 42M��4��ƵʱSCKʱ�� = 21M
		�����SPI3, 2��ƵʱSCKʱ�� = 21M
	*/
	#define SPI_BAUD			SPI_BaudRatePrescaler_8

	/* ƬѡGPIO�˿�  */
	#define W25_CS_GPIO			GPIOI
	#define W25_CS_PIN			GPIO_Pin_10

	#define W25_PWM_GPIO		GPIOF
	#define W25_PWM_PIN			GPIO_Pin_6
#endif

/* Ƭѡ�����õ�ѡ��  */
#define W25_CS_LOW()       W25_CS_GPIO->BSRRH = W25_CS_PIN
/* Ƭѡ�����ø߲�ѡ�� */
#define W25_CS_HIGH()      W25_CS_GPIO->BSRRL = W25_CS_PIN


/*
	PWM�����õ�ѡ��
	PWM = 1  ���ģʽ֧��STM32��дRA8875��ҵĴ���Flash
	PWM = 0 ������������ģʽ����RA8875 DMA��ȡ��ҵĴ���Flash
*/
#define W25_PWM_LOW()       W25_PWM_GPIO->BSRRH = W25_PWM_PIN
#define W25_PWM_HIGH()      W25_PWM_GPIO->BSRRL = W25_PWM_PIN


#define CMD_AAI       0xAD  	/* AAI �������ָ��(FOR SST25VF016B) */
#define CMD_DISWR	  0x04		/* ��ֹд, �˳�AAI״̬ */
#define CMD_EWRSR	  0x50		/* ����д״̬�Ĵ��������� */
#define CMD_WRSR      0x01  	/* д״̬�Ĵ������� */
#define CMD_WREN      0x06		/* дʹ������ */
#define CMD_READ      0x03  	/* ������������ */
#define CMD_RDSR      0x05		/* ��״̬�Ĵ������� */
#define CMD_RDID      0x9F		/* ������ID���� */
#define CMD_SE        0x20		/* ������������ */
#define CMD_BE        0xC7		/* ������������ */
#define DUMMY_BYTE    0xA5		/* ���������Ϊ����ֵ�����ڶ����� */

#define WIP_FLAG      0x01		/* ״̬�Ĵ����е����ڱ�̱�־��WIP) */


W25_T g_tW25;

void w25_ReadInfo(void);
static uint8_t w25_SendByte(uint8_t _ucValue);
static void w25_WriteEnable(void);
static void w25_WriteStatus(uint8_t _ucValue);
static void w25_WaitForWriteEnd(void);
static void bsp_CfgSPIForW25(void);

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitRA8875Flash
*	����˵��: ��ʼ������FlashӲ���ӿڣ�����STM32��SPIʱ�ӡ�GPIO)
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitRA8875Flash(void)
{

#ifdef STM32_X3		/* ������ STM32-X3 ������ */
	/*
		������STM32-X3 ���߷��䣺 ����Flash�ͺ�Ϊ W25Q64BVSSIG (80MHz)
		PB12 = CS
		PB13 = SCK
		PB14 = MISO
		PB15 = MOSI

		STM32Ӳ��SPI�ӿ� = SPI2
	*/
	{
		GPIO_InitTypeDef GPIO_InitStructure;

		/* ʹ��GPIO ʱ�� */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

		/* ���� SCK, MISO �� MOSI Ϊ���ù��� */
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		/* ����Ƭѡ����Ϊ�������ģʽ */
		W25_CS_HIGH();		/* Ƭѡ�øߣ���ѡ�� */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	}
#else
	/*
		������STM32-V5 ��������߷��䣺  ����Flash�ͺ�Ϊ W25Q64BVSSIG (80MHz)
		PB3/SPI3_SCK
		PB4/SPI3_MISO
		PB5/SPI3_MOSI
		PF8/W25_CS

		STM32Ӳ��SPI�ӿ� = SPI3
	*/
	{
		GPIO_InitTypeDef GPIO_InitStructure;

		/* ʹ��GPIO ʱ�� */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOI| RCC_AHB1Periph_GPIOF, ENABLE);

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
		W25_CS_HIGH();		/* Ƭѡ�øߣ���ѡ�� */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = W25_CS_PIN;
		GPIO_Init(W25_CS_GPIO, &GPIO_InitStructure);

		/* ����TFT�ӿ��е�PWM��ΪΪ�������ģʽ��PWM = 1ʱ ����дRA8875��ҵĴ���Flash */
		/* PF6/LCD_PWM  �����ڵ���RA8875���ı��� */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = W25_PWM_PIN;
		GPIO_Init(W25_PWM_GPIO, &GPIO_InitStructure);
	}
#endif

	/* ����SPIӲ���������ڷ��ʴ���Flash */
	bsp_CfgSPIForW25();
}

/*
*********************************************************************************************************
*	�� �� ��: w25_CtrlByMCU
*	����˵��: ����Flash����Ȩ����MCU ��STM32��
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void w25_CtrlByMCU(void)
{
	/*
		PWM�����õ�ѡ��
		PWM = 1  ���ģʽ֧��STM32��дRA8875��ҵĴ���Flash
		PWM = 0 ������������ģʽ����RA8875 DMA��ȡ��ҵĴ���Flash
	*/
	W25_PWM_HIGH();
}

/*
*********************************************************************************************************
*	�� �� ��: w25_CtrlByRA8875
*	����˵��: ����Flash����Ȩ����RA8875
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void w25_CtrlByRA8875(void)
{
	/*
		PWM�����õ�ѡ��
		PWM = 1  ���ģʽ֧��STM32��дRA8875��ҵĴ���Flash
		PWM = 0 ������������ģʽ����RA8875 DMA��ȡ��ҵĴ���Flash
	*/
	W25_PWM_LOW();
}

/*
*********************************************************************************************************
*	�� �� ��: w25_SelectChip
*	����˵��: ѡ�񼴽�������оƬ
*	��    ��: _idex = FONT_CHIP ��ʾ�ֿ�оƬ;  idex = BMP_CHIP ��ʾͼ��оƬ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void w25_SelectChip(uint8_t _idex)
{
	/*
		PWM = 1, KOUT3 = 0 д�ֿ�оƬ
		PWM = 1, KOUT3 = 1 дͼ��оƬ
	*/

	if (_idex == FONT_CHIP)
	{
		RA8875_CtrlGPO(3, 0);	/* RA8875 �� KOUT3 = 0 */
	}
	else	/* BMPͼƬоƬ */
	{
		RA8875_CtrlGPO(3, 1);	/* RA8875 �� KOUT3 = 1 */
	}

	w25_ReadInfo();				/* �Զ�ʶ��оƬ�ͺ� */

	W25_CS_LOW();				/* �����ʽ��ʹ�ܴ���FlashƬѡ */
	w25_SendByte(CMD_DISWR);		/* ���ͽ�ֹд�������,��ʹ�����д���� */
	W25_CS_HIGH();				/* �����ʽ�����ܴ���FlashƬѡ */

	w25_WaitForWriteEnd();		/* �ȴ�����Flash�ڲ�������� */

	w25_WriteStatus(0);			/* �������BLOCK��д���� */
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_CfgSPIForW25
*	����˵��: ����STM32�ڲ�SPIӲ���Ĺ���ģʽ���ٶȵȲ��������ڷ���SPI�ӿڵĴ���Flash��
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void bsp_CfgSPIForW25(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* ��SPIʱ�� */
	ENABLE_SPI_RCC();

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
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BAUD;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* ����λ������򣺸�λ�ȴ� */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC����ʽ�Ĵ�������λ��Ϊ7�������̲��� */
	SPI_Init(SPI_FLASH, &SPI_InitStructure);

	SPI_Cmd(SPI_FLASH, DISABLE);			/* �Ƚ�ֹSPI  */

	SPI_Cmd(SPI_FLASH, ENABLE);				/* ʹ��SPI  */
}

/*
*********************************************************************************************************
*	�� �� ��: w25_EraseSector
*	����˵��: ����ָ��������
*	��    ��:  _uiSectorAddr : ������ַ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void w25_EraseSector(uint32_t _uiSectorAddr)
{
	w25_WriteEnable();								/* ����дʹ������ */

	/* ������������ */
	W25_CS_LOW();									/* ʹ��Ƭѡ */
	w25_SendByte(CMD_SE);								/* ���Ͳ������� */
	w25_SendByte((_uiSectorAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
	w25_SendByte((_uiSectorAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
	w25_SendByte(_uiSectorAddr & 0xFF);				/* ����������ַ��8bit */
	W25_CS_HIGH();									/* ����Ƭѡ */

	w25_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
}

/*
*********************************************************************************************************
*	�� �� ��: w25_EraseChip
*	����˵��: ��������оƬ
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void w25_EraseChip(void)
{
	w25_WriteEnable();								/* ����дʹ������ */

	/* ������������ */
	W25_CS_LOW();									/* ʹ��Ƭѡ */
	w25_SendByte(CMD_BE);							/* ������Ƭ�������� */
	W25_CS_HIGH();									/* ����Ƭѡ */

	w25_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
}

/*
*********************************************************************************************************
*	�� �� ��: w25_WritePage
*	����˵��: ��һ��page��д�������ֽڡ��ֽڸ������ܳ���ҳ���С��4K)
*	��    ��:  	_pBuf : ����Դ��������
*				_uiWriteAddr ��Ŀ�������׵�ַ
*				_usSize �����ݸ��������ܳ���ҳ���С
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void w25_WritePage(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
	uint32_t i, j;

	if (g_tW25.ChipID == SST25VF016B_ID)
	{
		/* AAIָ��Ҫ��������ݸ�����ż�� */
		if ((_usSize < 2) && (_usSize % 2))
		{
			return ;
		}

		w25_WriteEnable();								/* ����дʹ������ */

		W25_CS_LOW();									/* ʹ��Ƭѡ */
		w25_SendByte(CMD_AAI);							/* ����AAI����(��ַ�Զ����ӱ��) */
		w25_SendByte((_uiWriteAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
		w25_SendByte((_uiWriteAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
		w25_SendByte(_uiWriteAddr & 0xFF);				/* ����������ַ��8bit */
		w25_SendByte(*_pBuf++);							/* ���͵�1������ */
		w25_SendByte(*_pBuf++);							/* ���͵�2������ */
		W25_CS_HIGH();									/* ����Ƭѡ */

		w25_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */

		_usSize -= 2;									/* ����ʣ���ֽ��� */

		for (i = 0; i < _usSize / 2; i++)
		{
			W25_CS_LOW();								/* ʹ��Ƭѡ */
			w25_SendByte(CMD_AAI);						/* ����AAI����(��ַ�Զ����ӱ��) */
			w25_SendByte(*_pBuf++);						/* �������� */
			w25_SendByte(*_pBuf++);						/* �������� */
			W25_CS_HIGH();								/* ����Ƭѡ */
			w25_WaitForWriteEnd();						/* �ȴ�����Flash�ڲ�д������� */
		}

		/* ����д����״̬ */
		W25_CS_LOW();
		w25_SendByte(CMD_DISWR);
		W25_CS_HIGH();

		w25_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
	}
	else	/* for MX25L1606E �� W25Q64BV */
	{
		for (j = 0; j < _usSize / 256; j++)
		{
			w25_WriteEnable();								/* ����дʹ������ */

			W25_CS_LOW();									/* ʹ��Ƭѡ */
			w25_SendByte(0x02);								/* ����AAI����(��ַ�Զ����ӱ��) */
			w25_SendByte((_uiWriteAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
			w25_SendByte((_uiWriteAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
			w25_SendByte(_uiWriteAddr & 0xFF);				/* ����������ַ��8bit */

			for (i = 0; i < 256; i++)
			{
				w25_SendByte(*_pBuf++);					/* �������� */
			}

			W25_CS_HIGH();								/* ��ֹƬѡ */

			w25_WaitForWriteEnd();						/* �ȴ�����Flash�ڲ�д������� */

			_uiWriteAddr += 256;
		}

		/* ����д����״̬ */
		W25_CS_LOW();
		w25_SendByte(CMD_DISWR);
		W25_CS_HIGH();

		w25_WaitForWriteEnd();							/* �ȴ�����Flash�ڲ�д������� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: w25_ReadBuffer
*	����˵��: ������ȡ�����ֽڡ��ֽڸ������ܳ���оƬ������
*	��    ��:  	_pBuf : ����Դ��������
*				_uiReadAddr ���׵�ַ
*				_usSize �����ݸ���, ���Դ���PAGE_SIZE,���ǲ��ܳ���оƬ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void w25_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
	/* �����ȡ�����ݳ���Ϊ0���߳�������Flash��ַ�ռ䣬��ֱ�ӷ��� */
	if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > g_tW25.TotalSize)
	{
		return;
	}

	/* ������������ */
	W25_CS_LOW();									/* ʹ��Ƭѡ */
	w25_SendByte(CMD_READ);							/* ���Ͷ����� */
	w25_SendByte((_uiReadAddr & 0xFF0000) >> 16);	/* ����������ַ�ĸ�8bit */
	w25_SendByte((_uiReadAddr & 0xFF00) >> 8);		/* ����������ַ�м�8bit */
	w25_SendByte(_uiReadAddr & 0xFF);				/* ����������ַ��8bit */
	while (_uiSize--)
	{
		*_pBuf++ = w25_SendByte(DUMMY_BYTE);			/* ��һ���ֽڲ��洢��pBuf�������ָ���Լ�1 */
	}
	W25_CS_HIGH();									/* ����Ƭѡ */
}

/*
*********************************************************************************************************
*	�� �� ��: w25_ReadID
*	����˵��: ��ȡ����ID
*	��    ��:  ��
*	�� �� ֵ: 32bit������ID (���8bit��0����ЧIDλ��Ϊ24bit��
*********************************************************************************************************
*/
uint32_t w25_ReadID(void)
{
	uint32_t uiID;
	uint8_t id1, id2, id3;

	W25_CS_LOW();									/* ʹ��Ƭѡ */
	w25_SendByte(CMD_RDID);								/* ���Ͷ�ID���� */
	id1 = w25_SendByte(DUMMY_BYTE);					/* ��ID�ĵ�1���ֽ� */
	id2 = w25_SendByte(DUMMY_BYTE);					/* ��ID�ĵ�2���ֽ� */
	id3 = w25_SendByte(DUMMY_BYTE);					/* ��ID�ĵ�3���ֽ� */
	W25_CS_HIGH();									/* ����Ƭѡ */

	uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;

	return uiID;
}

/*
*********************************************************************************************************
*	�� �� ��: w25_ReadInfo
*	����˵��: ��ȡ����ID,�������������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void w25_ReadInfo(void)
{
	/* �Զ�ʶ����Flash�ͺ� */
	{
		g_tW25.ChipID = w25_ReadID();	/* оƬID */

		switch (g_tW25.ChipID)
		{
			case SST25VF016B:
				g_tW25.TotalSize = 2 * 1024 * 1024;	/* ������ = 2M */
				g_tW25.PageSize = 4 * 1024;			/* ҳ���С = 4K */
				break;

			case MX25L1606E:
				g_tW25.TotalSize = 2 * 1024 * 1024;	/* ������ = 2M */
				g_tW25.PageSize = 4 * 1024;			/* ҳ���С = 4K */
				break;

			case W25Q64BV:
				g_tW25.TotalSize = 8 * 1024 * 1024;	/* ������ = 8M */
				g_tW25.PageSize = 4 * 1024;			/* ҳ���С = 4K */
				break;

			default:		/* ��ͨ�ֿⲻ֧��ID��ȡ */
				g_tW25.TotalSize = 2 * 1024 * 1024;
				g_tW25.PageSize = 4 * 1024;
				break;
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: w25_SendByte
*	����˵��: ����������һ���ֽڣ�ͬʱ��MISO���߲����������ص�����
*	��    ��:  _ucByte : ���͵��ֽ�ֵ
*	�� �� ֵ: ��MISO���߲����������ص�����
*********************************************************************************************************
*/
static uint8_t w25_SendByte(uint8_t _ucValue)
{
	/* �ȴ��ϸ�����δ������� */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_TXE) == RESET);

	/* ͨ��SPIӲ������1���ֽ� */
	SPI_I2S_SendData(SPI_FLASH, _ucValue);

	/* �ȴ�����һ���ֽ�������� */
	while (SPI_I2S_GetFlagStatus(SPI_FLASH, SPI_I2S_FLAG_RXNE) == RESET);

	/* ���ش�SPI���߶��������� */
	return SPI_I2S_ReceiveData(SPI_FLASH);
}

/*
*********************************************************************************************************
*	�� �� ��: w25_WriteEnable
*	����˵��: ����������дʹ������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void w25_WriteEnable(void)
{
	W25_CS_LOW();									/* ʹ��Ƭѡ */
	w25_SendByte(CMD_WREN);								/* �������� */
	W25_CS_HIGH();									/* ����Ƭѡ */
}

/*
*********************************************************************************************************
*	�� �� ��: w25_WriteStatus
*	����˵��: д״̬�Ĵ���
*	��    ��:  _ucValue : ״̬�Ĵ�����ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void w25_WriteStatus(uint8_t _ucValue)
{

	if (g_tW25.ChipID == SST25VF016B_ID)
	{
		/* ��1������ʹ��д״̬�Ĵ��� */
		W25_CS_LOW();									/* ʹ��Ƭѡ */
		w25_SendByte(CMD_EWRSR);							/* ������� ����д״̬�Ĵ��� */
		W25_CS_HIGH();									/* ����Ƭѡ */

		/* ��2������д״̬�Ĵ��� */
		W25_CS_LOW();									/* ʹ��Ƭѡ */
		w25_SendByte(CMD_WRSR);							/* ������� д״̬�Ĵ��� */
		w25_SendByte(_ucValue);							/* �������ݣ�״̬�Ĵ�����ֵ */
		W25_CS_HIGH();									/* ����Ƭѡ */
	}
	else
	{
		W25_CS_LOW();									/* ʹ��Ƭѡ */
		w25_SendByte(CMD_WRSR);							/* ������� д״̬�Ĵ��� */
		w25_SendByte(_ucValue);							/* �������ݣ�״̬�Ĵ�����ֵ */
		W25_CS_HIGH();									/* ����Ƭѡ */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: w25_WaitForWriteEnd
*	����˵��: ����ѭ����ѯ�ķ�ʽ�ȴ������ڲ�д�������
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void w25_WaitForWriteEnd(void)
{
	W25_CS_LOW();									/* ʹ��Ƭѡ */
	w25_SendByte(CMD_RDSR);							/* ������� ��״̬�Ĵ��� */
	while((w25_SendByte(DUMMY_BYTE) & WIP_FLAG) == SET);	/* �ж�״̬�Ĵ�����æ��־λ */
	W25_CS_HIGH();									/* ����Ƭѡ */
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
