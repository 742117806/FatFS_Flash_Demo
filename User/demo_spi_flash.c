/*
*********************************************************************************************************
*
*	ģ������ : ����Flash��д��ʾ����
*	�ļ����� : demo_spi_flash.c
*	��    �� : V1.0
*	˵    �� : ������STM32-V5���������Ĵ���Flash�ͺ�Ϊ W25Q64, 8M�ֽ�
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2013-02-01 armfly  ��ʽ����
*		V1.1    2013-06-20 armfly  ������ȡ���������д���������� getchar() ������ʽ��
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "demo_spi_flash.h"

#define TEST_ADDR		0			/* ��д���Ե�ַ */
#define TEST_SIZE		4096		/* ��д�������ݴ�С */

/* ���������ļ��ڵ��õĺ������� */
static void sfDispMenu(void);
static void sfReadTest(void);
static void sfWriteTest(void);
static void sfErase(void);
static void sfViewData(uint32_t _uiAddr);
static void sfWriteAll(uint8_t _ch);
static void sfTestReadSpeed(void);


/*
*********************************************************************************************************
*	�� �� ��: DemoSpiFlash
*	����˵��: ����EEPROM��д����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoSpiFlash(void)
{
	uint8_t cmd;
	uint32_t uiReadPageNo = 0;

	/* ��⴮��Flash OK */
	printf("��⵽����Flash, ID = %08X, �ͺ�: %s \r\n", g_tSF.ChipID , g_tSF.ChipName);
	printf("    ���� : %dM�ֽ�, ������С : %d�ֽ�\r\n", g_tSF.TotalSize/(1024*1024), g_tSF.PageSize);

	sfDispMenu();		/* ��ӡ������ʾ */

	while(1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */
		
		//cmd = getchar();	/* �Ӵ��ڶ���һ���ַ� (������ʽ) */
		if (comGetChar(COM1, &cmd))	/* �Ӵ��ڶ���һ���ַ�(��������ʽ) */
		{
			switch (cmd)
			{
				case '1':
					printf("\r\n��1 - ������Flash, ��ַ:0x%X,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);
					sfReadTest();		/* ������Flash���ݣ�����ӡ������������ */
					break;

				case '2':
					printf("\r\n��2 - д����Flash, ��ַ:0x%X,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);
					sfWriteTest();		/* д����Flash���ݣ�����ӡд���ٶ� */
					break;

				case '3':
					printf("\r\n��3 - ������������Flash��\r\n");
					sfErase();			/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
					break;

				case '4':
					printf("\r\n��4 - д��������Flash, ȫ0x55��\r\n");
					sfWriteAll(0x55);			/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
					break;

				case '5':
					printf("\r\n��5 - д��������Flash, ȫ0xAA��\r\n");
					sfWriteAll(0xAA);			/* ��������Flash���ݣ�ʵ���Ͼ���д��ȫ0xFF */
					break;

				case '6':
					printf("\r\n��6 - ����������Flash, %dM�ֽڡ�\r\n", g_tSF.TotalSize/(1024*1024));
					sfTestReadSpeed();		/* ����������Flash���ݣ������ٶ� */
					break;

				case 'z':
				case 'Z': /* ��ȡǰ1K */
					if (uiReadPageNo > 0)
					{
						uiReadPageNo--;
					}
					else
					{
						printf("�Ѿ�����ǰ\r\n");
					}
					sfViewData(uiReadPageNo * 1024);
					break;

				case 'x':
				case 'X': /* ��ȡ��1K */
					if (uiReadPageNo < g_tSF.TotalSize / 1024 - 1)
					{
						uiReadPageNo++;
					}
					else
					{
						printf("�Ѿ������\r\n");
					}
					sfViewData(uiReadPageNo * 1024);
					break;

				default:
					sfDispMenu();	/* ��Ч������´�ӡ������ʾ */
					break;

			}
		}
		
		/* �����˲��ͼ���ɺ�̨systick�жϷ������ʵ�֣�����ֻ��Ҫ����bsp_GetKey��ȡ��ֵ���ɡ� */		
		switch (bsp_GetKey())	/* bsp_GetKey()��ȡ��ֵ, �޼�����ʱ���� KEY_NONE = 0 */
		{
			case KEY_DOWN_K1:			/* K1������ */
				break;

			case KEY_UP_K1:				/* K1������ */
				break;

			case KEY_DOWN_K2:			/* K2������ */
				break;

			case KEY_UP_K2:				/* K2������ */
				break;

			case KEY_DOWN_K3:			/* K3������ */
				break;

			case KEY_UP_K3:				/* K3������ */
				break;

			case JOY_DOWN_U:			/* ҡ��UP������ */
				break;

			case JOY_DOWN_D:			/* ҡ��DOWN������ */
				break;

			case JOY_DOWN_L:			/* ҡ��LEFT������ */
				break;

			case JOY_DOWN_R:			/* ҡ��RIGHT������ */
				break;

			case JOY_DOWN_OK:			/* ҡ��OK������ */
				break;

			case JOY_UP_OK:				/* ҡ��OK������ */
				break;

			case KEY_NONE:				/* �޼����� */
			default:
				/* �����ļ�ֵ������ */
				break;
		}		
	}


}

/*
*********************************************************************************************************
*	�� �� ��: sfReadTest
*	����˵��: ������Flash����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfReadTest(void)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[TEST_SIZE];

	/* ��ʼ��ַ = 0�� ���ݳ���Ϊ 256 */
	iTime1 = bsp_GetRunTime();	/* ���¿�ʼʱ�� */
	sf_ReadBuffer(buf, TEST_ADDR, TEST_SIZE);
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */
	printf("������Flash�ɹ����������£�\r\n");

	/* ��ӡ���� */
	for (i = 0; i < TEST_SIZE; i++)
	{
		printf(" %02X", buf[i]);

		if ((i & 31) == 31)
		{
			printf("\r\n");	/* ÿ����ʾ16�ֽ����� */
		}
		else if ((i & 31) == 15)
		{
			printf(" - ");
		}
	}

	/* ��ӡ���ٶ� */
	printf("���ݳ���: %d�ֽ�, ����ʱ: %dms, ���ٶ�: %d Bytes/s\r\n", TEST_SIZE, iTime2 - iTime1, (TEST_SIZE * 1000) / (iTime2 - iTime1));
}


/*
*********************************************************************************************************
*	�� �� ��: sfTestReadSpeed
*	����˵��: ���Դ���Flash���ٶȡ���ȡ��������Flash�����ݣ�����ӡ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfTestReadSpeed(void)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[TEST_SIZE];
	uint32_t uiAddr;

	/* ��ʼ��ַ = 0�� ���ݳ���Ϊ 256 */
	iTime1 = bsp_GetRunTime();	/* ���¿�ʼʱ�� */
	uiAddr = 0;
	for (i = 0; i < g_tSF.TotalSize / TEST_SIZE; i++, uiAddr += TEST_SIZE)
	{
		sf_ReadBuffer(buf, uiAddr, TEST_SIZE);
	}
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */

	/* ��ӡ���ٶ� */
	printf("���ݳ���: %d�ֽ�, ����ʱ: %dms, ���ٶ�: %d Bytes/s\r\n", g_tSF.TotalSize, iTime2 - iTime1, (g_tSF.TotalSize * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: sfWriteTest
*	����˵��: д����Flash����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfWriteTest(void)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[TEST_SIZE];

	/* �����Ի����� */
	for (i = 0; i < TEST_SIZE; i++)
	{
		buf[i] = i;
	}

	/* дEEPROM, ��ʼ��ַ = 0�����ݳ���Ϊ 256 */
	iTime1 = bsp_GetRunTime();	/* ���¿�ʼʱ�� */
	if (sf_WriteBuffer(buf, TEST_ADDR, TEST_SIZE) == 0)
	{
		printf("д����Flash������\r\n");
		return;
	}
	else
	{
		iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */
		printf("д����Flash�ɹ���\r\n");
	}


	/* ��ӡ���ٶ� */
	printf("���ݳ���: %d�ֽ�, д��ʱ: %dms, д�ٶ�: %dB/s\r\n", TEST_SIZE, iTime2 - iTime1, (TEST_SIZE * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: sfWriteAll
*	����˵��: д����EEPROMȫ������
*	��    �Σ�_ch : д�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfWriteAll(uint8_t _ch)
{
	uint16_t i;
	int32_t iTime1, iTime2;
	uint8_t buf[4 * 1024];

	/* �����Ի����� */
	for (i = 0; i < TEST_SIZE; i++)
	{
		buf[i] = _ch;
	}

	/* дEEPROM, ��ʼ��ַ = 0�����ݳ���Ϊ 256 */
	iTime1 = bsp_GetRunTime();	/* ���¿�ʼʱ�� */
	for (i = 0; i < g_tSF.TotalSize / g_tSF.PageSize; i++)
	{
		if (sf_WriteBuffer(buf, i * g_tSF.PageSize, g_tSF.PageSize) == 0)
		{
			printf("д����Flash������\r\n");
			return;
		}
		printf(".");
		if (((i + 1) % 128) == 0)
		{
			printf("\r\n");
		}
	}
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */

	/* ��ӡ���ٶ� */
	printf("���ݳ���: %dK�ֽ�, д��ʱ: %dms, д�ٶ�: %dB/s\r\n", g_tSF.TotalSize / 1024, iTime2 - iTime1, (g_tSF.TotalSize * 1000) / (iTime2 - iTime1));
}

/*
*********************************************************************************************************
*	�� �� ��: sfErase
*	����˵��: ��������Flash
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfErase(void)
{
	int32_t iTime1, iTime2;

	iTime1 = bsp_GetRunTime();	/* ���¿�ʼʱ�� */
	sf_EraseChip();
	iTime2 = bsp_GetRunTime();	/* ���½���ʱ�� */

	/* ��ӡ���ٶ� */
	printf("��������Flash��ɣ�, ��ʱ: %dms\r\n", iTime2 - iTime1);
	return;
}


/*
*********************************************************************************************************
*	�� �� ��: sfViewData
*	����˵��: ������Flash����ʾ��ÿ����ʾ1K������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfViewData(uint32_t _uiAddr)
{
	uint16_t i;
	uint8_t buf[1024];

	sf_ReadBuffer(buf, _uiAddr,  1024);		/* ������ */
	printf("��ַ��0x%08X; ���ݳ��� = 1024\r\n", _uiAddr);

	/* ��ӡ���� */
	for (i = 0; i < 1024; i++)
	{
		printf(" %02X", buf[i]);

		if ((i & 31) == 31)
		{
			printf("\r\n");	/* ÿ����ʾ16�ֽ����� */
		}
		else if ((i & 31) == 15)
		{
			printf(" - ");
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: sfDispMenu
*	����˵��: ��ʾ������ʾ�˵�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void sfDispMenu(void)
{
	printf("\r\n*******************************************\r\n");
	printf("��ѡ���������:\r\n");
	printf("��1 - ������Flash, ��ַ:0x%X,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);
	printf("��2 - д����Flash, ��ַ:0x%X,����:%d�ֽڡ�\r\n", TEST_ADDR, TEST_SIZE);
	printf("��3 - ������������Flash��\r\n");
	printf("��4 - д��������Flash, ȫ0x55��\r\n");
	printf("��5 - д��������Flash, ȫ0xAA��\r\n");
	printf("��6 - ����������Flash, ���Զ��ٶȡ�\r\n");
	printf("��Z - ��ȡǰ1K����ַ�Զ����١�\r\n");
	printf("��X - ��ȡ��1K����ַ�Զ����ӡ�\r\n");
	printf("��������� - ��ʾ������ʾ\r\n");
	printf("\r\n");
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/