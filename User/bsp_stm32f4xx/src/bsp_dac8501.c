/*
*********************************************************************************************************
*
*	ģ������ : DAC8501 ����ģ��(��ͨ����16λDAC)
*	�ļ����� : bsp_dac8501.c
*	��    �� : V1.0
*	˵    �� : DAC8501ģ���CPU֮�����SPI�ӿڡ�����������֧��Ӳ��SPI�ӿں����SPI�ӿڡ�
*			  ͨ�����л���
*
*	�޸ļ�¼ :
*		�汾��  ����         ����     ˵��
*		V1.0    2014-01-17  armfly  ��ʽ����
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

#define SOFT_SPI		/* ������б�ʾʹ��GPIOģ��SPI�ӿ� */
//#define HARD_SPI		/* ������б�ʾʹ��CPU��Ӳ��SPI�ӿ� */

/*
	DAC8501ģ�����ֱ�Ӳ嵽STM32-V5������CN19��ĸ(2*4P 2.54mm)�ӿ���

    DAC8501ģ��    STM32F407������
	  VCC   ------  3.3V
	  GND   ------  GND
      SCLK  ------  PB3/SPI3_SCK
      MOSI  ------  PB5/SPI3_MOSI
      CS1   ------  PF7/NRF24L01_CSN
	  CS2   ------  PA4/NRF905_TX_EN/NRF24L01_CE/DAC1_OUT
			------  PB4/SPI3_MISO
			------  PH7/NRF24L01_IRQ

*/

/*
	DAC8501��������:
	1������2.7 - 5V;  ������ʹ��3.3V��
	4���ο���ѹ2.5V (�Ƽ�ȱʡ�ģ����õģ�

	��SPI��ʱ���ٶ�Ҫ��: �ߴ�30MHz�� �ٶȺܿ�.
	SCLK�½��ض�ȡ����, ÿ�δ���24bit���ݣ� ��λ�ȴ�
*/

#if !defined(SOFT_SPI) && !defined(HARD_SPI)
 	#error "Please define SPI Interface mode : SOFT_SPI or HARD_SPI"
#endif

#ifdef SOFT_SPI		/* ���SPI */
	/* ����GPIO�˿� */
	#define RCC_SCLK 	RCC_AHB1Periph_GPIOB
	#define PORT_SCLK	GPIOB
	#define PIN_SCLK	GPIO_Pin_3

	#define RCC_MOSI 	RCC_AHB1Periph_GPIOB
	#define PORT_MOSI	GPIOB
	#define PIN_MOSI		GPIO_Pin_5

	#define RCC_CS1 	RCC_AHB1Periph_GPIOF
	#define PORT_CS1	GPIOF
	#define PIN_CS1		GPIO_Pin_7
	
	/* ��2·Ƭѡ */
	#define RCC_CS2 	RCC_AHB1Periph_GPIOA
	#define PORT_CS2	GPIOA
	#define PIN_CS2		GPIO_Pin_4

	/* ���������0����1�ĺ� */
	#if 0 /* �⺯����ʽ */
		#define CS1_0()		GPIO_ResetBits(PORT_CS1, PIN_CS1)
		#define CS1_1()		GPIO_SetBits(PORT_CS1, PIN_CS1)

		#define SCLK_0()	GPIO_ResetBits(PORT_SCLK, PIN_SCLK)
		#define SCLK_1()	GPIO_SetBits(PORT_SCLK, PIN_SCLK)

		#define MOSI_0()	GPIO_ResetBits(PORT_MOSI, PIN_MOSI)
		#define MOSI_1()	GPIO_SetBits(PORT_MOSI, PIN_MOSI)

		#define CS2_0()		GPIO_ResetBits(PORT_CS2, PIN_CS2)
		#define CS2_1()		GPIO_SetBits(PORT_CS2, PIN_CS2)
	#else	/* ֱ�Ӳ����Ĵ���������ٶ� */
		#define CS1_0()		PORT_CS1->BSRRH = PIN_CS1 
		#define CS1_1()		PORT_CS1->BSRRL = PIN_CS1

		#define SCLK_0()	PORT_SCLK->BSRRH = PIN_SCLK
		#define SCLK_1()	PORT_SCLK->BSRRL = PIN_SCLK

		#define MOSI_0()	PORT_MOSI->BSRRH = PIN_MOSI
		#define MOSI_1()	PORT_MOSI->BSRRL = PIN_MOSI

		#define CS2_0()		PORT_CS2->BSRRH = PIN_CS2
		#define CS2_1()		PORT_CS2->BSRRL = PIN_CS2
	#endif
#endif

#ifdef HARD_SPI		/* Ӳ��SPI (δ��) */
	;
#endif

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitDAC8501
*	����˵��: ����STM32��GPIO��SPI�ӿڣ��������� ADS1256
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitDAC8501(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

#ifdef SOFT_SPI
	SCLK_0();		/* SPI���߿���ʱ�������ǵ͵�ƽ */
	CS1_1();		/* �Ȳ�ѡ��, SYNC�൱��SPI�豸��Ƭѡ */
	CS2_1();

	/* ��GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_SCLK | RCC_MOSI | RCC_CS1 | RCC_CS2, ENABLE);

	/* ���ü����������IO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO������ٶ� */

	GPIO_InitStructure.GPIO_Pin = PIN_SCLK;
	GPIO_Init(PORT_SCLK, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_MOSI;
	GPIO_Init(PORT_MOSI, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_CS1;
	GPIO_Init(PORT_CS1, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_CS2;
	GPIO_Init(PORT_CS2, &GPIO_InitStructure);
#endif

	DAC8501_SendData(0, 0);
	DAC8501_SendData(1, 0);
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8501_SendData
*	����˵��: ��SPI���߷���24��bit���ݡ�
*	��    ��: _ch, ͨ��, 
*		     _data : ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8501_SendData(uint8_t _ch, uint16_t _dac)
{
	uint8_t i;
	uint32_t data;

	/*
		DAC8501.pdf page 12 ��24bit����

		DB24:18 = xxxxx ����
		DB17�� PD1
		DB16�� PD0

		DB15��0  16λ����

		���� PD1 PD0 ����4�ֹ���ģʽ
		      0   0  ---> ��������ģʽ
		      0   1  ---> �����1Kŷ��GND
		      1   0  ---> ���100Kŷ��GND
		      1   1  ---> �������
	*/

	data = _dac; /* PD1 PD0 = 00 ����ģʽ */

	if (_ch == 0)
	{
		CS1_0();
	}
	else
	{
		CS2_0();
	}

	/*��DAC8501 SCLKʱ�Ӹߴ�30M����˿��Բ��ӳ� */
	for(i = 0; i < 24; i++)
	{
		if (data & 0x800000)
		{
			MOSI_1();
		}
		else
		{
			MOSI_0();
		}
		SCLK_1();
		data <<= 1;
		SCLK_0();
	}
	
	if (_ch == 0)
	{
		CS1_1();
	}
	else
	{
		CS2_1();
	}

}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
