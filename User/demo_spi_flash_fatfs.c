/*
*********************************************************************************************************
*
*	ģ������ : SPI Flash��Fat�ļ�ϵͳ��ʾģ�顣
*	�ļ����� : demo_spi_flash_fatfs.c
*	��    �� : V1.0
*	˵    �� : ��������ֲFatFS�ļ�ϵͳ���汾 R0.10b������ʾ��δ����ļ�����ȡ�ļ�������Ŀ¼��ɾ���ļ�
*			   ���������ļ���д�ٶȡ�
*
*	�޸ļ�¼ :
*		�汾��   ����        ����     ˵��
*		V1.0    2014-06-15  armfly  ��ʽ����
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "ff.h"			/* FatFS�ļ�ϵͳģ��*/
#include "demo_spi_flash_fatfs.h"

/* ���ڲ��Զ�д�ٶ� */
#define TEST_FILE_LEN			(2*1024*1024)	/* ���ڲ��Ե��ļ����� */
#define BUF_SIZE				(4*1024)		/* ÿ�ζ�дSD����������ݳ��� */
uint8_t g_TestBuf[BUF_SIZE];

/* �������ļ��ڵ��õĺ������� */
static void FileFormat(void);
static void DispMenu(void);
static void ViewRootDir(void);
static void CreateNewFile(void);
static void ReadFileData(void);
static void CreateDir(void);
static void DeleteDirFile(void);
static void WriteFileTest(void);
static void ModifyFileTest(void);
static void  ReadDir(void);


/* FatFs API�ķ���ֵ */
static const char * FR_Table[]= 
{
	"FR_OK���ɹ�",				                             /* (0) Succeeded */
	"FR_DISK_ERR���ײ�Ӳ������",			                 /* (1) A hard error occurred in the low level disk I/O layer */
	"FR_INT_ERR������ʧ��",				                     /* (2) Assertion failed */
	"FR_NOT_READY����������û�й���",			             /* (3) The physical drive cannot work */
	"FR_NO_FILE���ļ�������",				                 /* (4) Could not find the file */
	"FR_NO_PATH��·��������",				                 /* (5) Could not find the path */
	"FR_INVALID_NAME����Ч�ļ���",		                     /* (6) The path name format is invalid */
	"FR_DENIED�����ڽ�ֹ���ʻ���Ŀ¼�������ʱ��ܾ�",         /* (7) Access denied due to prohibited access or directory full */
	"FR_EXIST���ļ��Ѿ�����",			                     /* (8) Access denied due to prohibited access */
	"FR_INVALID_OBJECT���ļ�����Ŀ¼������Ч",		         /* (9) The file/directory object is invalid */
	"FR_WRITE_PROTECTED������������д����",		             /* (10) The physical drive is write protected */
	"FR_INVALID_DRIVE���߼���������Ч",		                 /* (11) The logical drive number is invalid */
	"FR_NOT_ENABLED�������޹�����",			                 /* (12) The volume has no work area */
	"FR_NO_FILESYSTEM��û����Ч��FAT��",		             /* (13) There is no valid FAT volume */
	"FR_MKFS_ABORTED�����ڲ�������f_mkfs()����ֹ",	         /* (14) The f_mkfs() aborted due to any parameter error */
	"FR_TIMEOUT���ڹ涨��ʱ�����޷���÷��ʾ�����",		 /* (15) Could not get a grant to access the volume within defined period */
	"FR_LOCKED�������ļ�������Բ������ܾ�",				 /* (16) The operation is rejected according to the file sharing policy */
	"FR_NOT_ENOUGH_CORE���޷����䳤�ļ���������",		     /* (17) LFN working buffer could not be allocated */
	"FR_TOO_MANY_OPEN_FILES����ǰ�򿪵��ļ�������_FS_SHARE", /* (18) Number of open files > _FS_SHARE */
	"FR_INVALID_PARAMETER��������Ч"	                     /* (19) Given parameter is invalid */
};


/*
*********************************************************************************************************
*	�� �� ��: DemoFatFS
*	����˵��: FatFS�ļ�ϵͳ��ʾ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DemoFatFS(void)
{
	uint8_t cmd;
	
	/* ��ӡ�����б��û�����ͨ�����ڲ���ָ�� */
	DispMenu();
	bsp_StartAutoTimer(1, 100);
	while(1)
	{
		bsp_Idle();		/* ���������bsp.c�ļ����û������޸��������ʵ��CPU���ߺ�ι�� */
		
		if(bsp_CheckTimer(1))
		{
			bsp_LedToggle(1);
		}

		if (comGetChar(COM1, &cmd))	/* �Ӵ��ڶ���һ���ַ�(��������ʽ) */
		{
			printf("\r\n");
			switch (cmd)
			{
				case '0':
					printf("��0 - FileFormat��\r\n");
					FileFormat();		/* ��ʾSD����Ŀ¼�µ��ļ��� */
					break;
				
				case '1':
					printf("��1 - ViewRootDir��\r\n");
					ViewRootDir();		/* ��ʾSD����Ŀ¼�µ��ļ��� */
					break;

				case '2':
					printf("��2 - CreateNewFile��\r\n");
					CreateNewFile();		/* ����һ�����ļ�,д��һ���ַ��� */
					break;

				case '3':
					printf("��3 - ReadFileData��\r\n");
					ReadFileData();		/* ��ȡ��Ŀ¼��armfly.txt������ */
					break;

				case '4':
					printf("��4 - CreateDir��\r\n");
					CreateDir();		/* ����Ŀ¼ */
					break;

				case '5':
					printf("��5 - DeleteDirFile��\r\n");
					DeleteDirFile();	/* ɾ��Ŀ¼���ļ� */
					break;

				case '6':
					printf("��6 - TestSpeed��\r\n");
					WriteFileTest();	/* �ٶȲ��� */
					break;
				case '7':
					printf("��7 - ModifyFileTest��\r\n");
					ModifyFileTest();	/*�����޸��ļ�����*/
                    break;
				case '8':
					printf("��8 - ReadDir��\r\n");
                    ReadDir();
					
                    break;		
				default:
					DispMenu();
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
*	�� �� ��: DispMenu
*	����˵��: ��ʾ������ʾ�˵�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispMenu(void)
{
	printf("\r\n------------------------------------------------\r\n");
	printf("��һ��ʹ����ѡ������0����SPI Flash��ʽ��\r\n");
	printf("��ѡ���������:\r\n");
	printf("0 - ��SPI_Flash�����ļ�ϵͳ��ʽ��\r\n");
	printf("1 - ��ʾ��Ŀ¼�µ��ļ��б�\r\n");
	printf("2 - ����һ�����ļ�armfly.txt\r\n");
	printf("3 - ��armfly.txt�ļ�������\r\n");
	printf("4 - ����Ŀ¼\r\n");
	printf("5 - ɾ���ļ���Ŀ¼\r\n");
	printf("6 - ��д�ļ��ٶȲ���\r\n");
	printf("7 - �ļ������޸Ĳ���\r\n");
	printf("8 - ������ǰ·�������ļ���Ŀ¼����\r\n");
}


/*
*********************************************************************************************************
*	�� �� ��: FileFormat
*	����˵��: �ļ�ϵͳ��ʽ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void FileFormat(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */
	

	/* �����ļ�ϵͳ */
	result = f_mount(&fs, "0:", 0);	
	
	if (result != FR_OK)
	{
		printf("�����ļ�ϵͳʧ�� (%s)\r\n", FR_Table[result]);
	}
	else
	{
		printf("�����ļ�ϵͳ�ɹ� (%s)\r\n", FR_Table[result]);
	}
	
	/* ��һ��ʹ�ñ�����и�ʽ�� */
	result = f_mkfs("0:", FM_ANY, 0, work, sizeof work);//"0:"�Ǿ�꣬������ #define SPI_FLASH		0
	if (result != FR_OK)
	{
		printf("��ʽ��ʧ�� (%s)\r\n", FR_Table[result]);
	}
	else
	{
		printf("��ʽ���ɹ� (%s)\r\n", FR_Table[result]);
	}

	/* ж���ļ�ϵͳ */
	result  = f_mount(NULL, "0:", 0);
	if (result != FR_OK)
	{
		printf("ж���ļ�ϵͳʧ�� (%s)\r\n", FR_Table[result]);
	}
	else
	{
		printf("ж���ļ�ϵͳ�ɹ� (%s)\r\n", FR_Table[result]);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: ViewRootDir
*	����˵��: ��ʾSD����Ŀ¼�µ��ļ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ViewRootDir(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t cnt = 0;


	/* �����ļ�ϵͳ */
	result = f_mount(&fs, "0:", 0);	
	if (result != FR_OK)
	{
		printf("�����ļ�ϵͳʧ�� (%s)\r\n", FR_Table[result]);
	}
	else
	{
		printf("�����ļ�ϵͳ�ɹ� (%s)\r\n", FR_Table[result]);
	}
	
	/* �򿪸��ļ��� */
	//result = f_opendir(&DirInf, "0:/"); /* ���������������ӵ�ǰĿ¼��ʼ */
	
	result = f_opendir(&DirInf, "0:/"); /* ���������������ӵ�ǰĿ¼��ʼ */
	if (result != FR_OK)
	{
		printf("�򿪸�Ŀ¼ʧ�� (%s)\r\n", FR_Table[result]);
		return;
	}

	/* ��ȡ��ǰ�ļ����µ��ļ���Ŀ¼ */


	printf("����        |  �ļ���С | ���ļ��� | ���ļ���\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf,&FileInf); 		/* ��ȡĿ¼��������Զ����� */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')
		{
			continue;
		}

		/* �ж����ļ�������Ŀ¼ */
		if (FileInf.fattrib & AM_DIR)
		{
			printf("(0x%02d)Ŀ¼  ", FileInf.fattrib);
		}
		else
		{
			printf("(0x%02d)�ļ�  ", FileInf.fattrib);
		}

		/* ��ӡ�ļ���С, ���4G */
		printf(" %10d", FileInf.fsize);

		printf("  %s |", FileInf.fname);	/* ���ļ��� */

		printf("  %s\r\n", (char *)FileInf.altname);	/* ���ļ��� */
	}

	/* ж���ļ�ϵͳ */
	result  = f_mount(NULL, "0:", 0);
	if (result != FR_OK)
	{
		printf("ж���ļ�ϵͳʧ�� (%s)\r\n", FR_Table[result]);
	}
	else
	{
		printf("ж���ļ�ϵͳ�ɹ� (%s)\r\n", FR_Table[result]);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: CreateNewFile
*	����˵��: ��SD������һ�����ļ����ļ�������д��www.armfly.com��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void CreateNewFile(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;

 	/* �����ļ�ϵͳ */
	result = f_mount(&fs, "0:", 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("�����ļ�ϵͳʧ�� (%s)\r\n",  FR_Table[result]);
	}

	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, "0:/"); /* ���������������ӵ�ǰĿ¼��ʼ */
	if (result != FR_OK)
	{
		printf("�򿪸�Ŀ¼ʧ�� (%s)\r\n",  FR_Table[result]);
		return;
	}

	/* ���ļ� */
	result = f_open(&file, "armfly.txt", FA_CREATE_ALWAYS | FA_WRITE);

	/* дһ������ */
	result = f_write(&file, "FatFS Write Demo \r\n www.armfly.com \r\n", 34, &bw);
	if (result == FR_OK)
	{
		printf("armfly.txt �ļ�д��ɹ�\r\n");
	}
	else
	{
		printf("armfly.txt �ļ�д��ʧ��\r\n");
	}

	/* �ر��ļ�*/
	f_close(&file);

	/* ж���ļ�ϵͳ */
	result  = f_mount(NULL, "0:", 0);
}

/*
*********************************************************************************************************
*	�� �� ��: ReadFileData
*	����˵��: ��ȡ�ļ�armfly.txtǰ128���ַ�������ӡ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ReadFileData(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	char buf[128];

 	/* �����ļ�ϵͳ */
	result = f_mount(&fs, "0:", 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("�����ļ�ϵͳʧ��(%s)\r\n",  FR_Table[result]);
	}

	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, "/"); /* ���������������ӵ�ǰĿ¼��ʼ */
	if (result != FR_OK)
	{
		printf("�򿪸�Ŀ¼ʧ��(%s)\r\n",  FR_Table[result]);
		return;
	}

	/* ���ļ� */
	result = f_open(&file, "armfly.txt", FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		printf("Don't Find File : armfly.txt\r\n");
		return;
	}

	/* ��ȡ�ļ� */
	result = f_read(&file, &buf, sizeof(buf) - 1, &bw);
	if (bw > 0)
	{
		buf[bw] = 0;
		printf("\r\narmfly.txt �ļ����� : \r\n%s\r\n", buf);
	}
	else
	{
		printf("\r\narmfly.txt �ļ����� : \r\n");
	}

	/* �ر��ļ�*/
	f_close(&file);

	/* ж���ļ�ϵͳ */
	result  = f_mount(NULL, "0:", 0);
}

/*
*********************************************************************************************************
*	�� �� ��: CreateDir
*	����˵��: ��SD����Ŀ¼����Dir1��Dir2Ŀ¼����Dir1Ŀ¼�´�����Ŀ¼Dir1_1
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void CreateDir(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;

 	/* �����ļ�ϵͳ */
	result = f_mount(&fs, "0:", 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("�����ļ�ϵͳʧ�� (%s)\r\n",  FR_Table[result]);
	}

	/* ����Ŀ¼/Dir1 */
	result = f_mkdir("/Dir1");
	if (result == FR_OK)
	{
		printf("f_mkdir Dir1 Ok\r\n");
	}
	else if (result == FR_EXIST)
	{
		printf("Dir1 Ŀ¼�Ѿ�����(%s)\r\n",  FR_Table[result]);
	}
	else
	{
		printf("f_mkdir Dir1 ʧ�� (%s)\r\n",  FR_Table[result]);
		return;
	}

	/* ����Ŀ¼/Dir2 */
	result = f_mkdir("/Dir2");
	if (result == FR_OK)
	{
		printf("f_mkdir Dir2 Ok\r\n");
	}
	else if (result == FR_EXIST)
	{
		printf("Dir2 Ŀ¼�Ѿ�����(%s)\r\n",  FR_Table[result]);
	}
	else
	{
		printf("f_mkdir Dir2 ʧ�� (%s)\r\n",  FR_Table[result]);
		return;
	}

	/* ������Ŀ¼ /Dir1/Dir1_1	   ע�⣺������Ŀ¼Dir1_1ʱ�������ȴ�����Dir1 */
	result = f_mkdir("/Dir1/Dir1_1"); /* */
	if (result == FR_OK)
	{
		printf("f_mkdir Dir1_1 �ɹ�\r\n");
	}
	else if (result == FR_EXIST)
	{
		printf("Dir1_1 Ŀ¼�Ѿ����� (%s)\r\n",  FR_Table[result]);
	}
	else
	{
		printf("f_mkdir Dir1_1 ʧ�� (%s)\r\n",  FR_Table[result]);
		return;
	}

	/* ж���ļ�ϵͳ */
	result  = f_mount(NULL, "0:", 0);
}

/*
*********************************************************************************************************
*	�� �� ��: DeleteDirFile
*	����˵��: ɾ��SD����Ŀ¼�µ� armfly.txt �ļ��� Dir1��Dir2 Ŀ¼
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DeleteDirFile(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	char FileName[13];
	uint8_t i;

 	/* �����ļ�ϵͳ */
	result = f_mount(&fs, "0:", 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("�����ļ�ϵͳʧ�� (%s)\r\n",  FR_Table[result]);
	}

	#if 0
	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, "/"); /* ���������������ӵ�ǰĿ¼��ʼ */
	if (result != FR_OK)
	{
		printf("�򿪸�Ŀ¼ʧ��(%s)\r\n",  FR_Table[result]);
		return;
	}
	#endif

	/* ɾ��Ŀ¼/Dir1 ����Ϊ������Ŀ¼�ǿգ�������Ŀ¼)���������ɾ����ʧ�ܡ�*/
	result = f_unlink("/Dir1");
	if (result == FR_OK)
	{
		printf("ɾ��Ŀ¼Dir1�ɹ�\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("û�з����ļ���Ŀ¼ :%s\r\n", "/Dir1");
	}
	else
	{
		printf("ɾ��Dir1ʧ��(������� = %s) �ļ�ֻ����Ŀ¼�ǿ�\r\n",  FR_Table[result]);
	}

	/* ��ɾ��Ŀ¼/Dir1/Dir1_1 */
	result = f_unlink("/Dir1/Dir1_1");
	if (result == FR_OK)
	{
		printf("ɾ����Ŀ¼/Dir1/Dir1_1�ɹ�\r\n");
	}
	else if ((result == FR_NO_FILE) || (result == FR_NO_PATH))
	{
		printf("û�з����ļ���Ŀ¼ :%s\r\n", "/Dir1/Dir1_1");
	}
	else
	{
		printf("ɾ����Ŀ¼/Dir1/Dir1_1ʧ��(������� = %s) �ļ�ֻ����Ŀ¼�ǿ�\r\n",  FR_Table[result]);
	}

	/* ��ɾ��Ŀ¼/Dir1 */
	result = f_unlink("/Dir1");
	if (result == FR_OK)
	{
		printf("ɾ��Ŀ¼Dir1�ɹ�\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("û�з����ļ���Ŀ¼ :%s\r\n", "/Dir1");
	}
	else
	{
		printf("ɾ��Dir1ʧ��(������� = %s) �ļ�ֻ����Ŀ¼�ǿ�\r\n",  FR_Table[result]);
	}

	/* ɾ��Ŀ¼/Dir2 */
	result = f_unlink("/Dir2");
	if (result == FR_OK)
	{
		printf("ɾ��Ŀ¼ Dir2 �ɹ�\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("û�з����ļ���Ŀ¼ :%s\r\n", "/Dir2");
	}
	else
	{
		printf("ɾ��Dir2 ʧ��(������� = %s) �ļ�ֻ����Ŀ¼�ǿ�\r\n",  FR_Table[result]);
	}

	/* ɾ���ļ� armfly.txt */
	result = f_unlink("armfly.txt");
	if (result == FR_OK)
	{
		printf("ɾ���ļ� armfly.txt �ɹ�\r\n");
	}
	else if (result == FR_NO_FILE)
	{
		printf("û�з����ļ���Ŀ¼ :%s\r\n", "armfly.txt");
	}
	else
	{
		printf("ɾ��armfly.txtʧ��(������� = %s) �ļ�ֻ����Ŀ¼�ǿ�\r\n",  FR_Table[result]);
	}

	/* ɾ���ļ� speed1.txt */
	for (i = 0; i < 20; i++)
	{
		sprintf(FileName, "Speed%02d.txt", i);		/* ÿд1�Σ���ŵ��� */
		result = f_unlink(FileName);
		if (result == FR_OK)
		{
			printf("ɾ���ļ�%s�ɹ�\r\n", FileName);
		}
		else if (result == FR_NO_FILE)
		{
			printf("û�з����ļ�:%s\r\n", FileName);
		}
		else
		{
			printf("ɾ��%s�ļ�ʧ��(������� = %d) �ļ�ֻ����Ŀ¼�ǿ�\r\n", FileName, result);
		}
	}

	/* ж���ļ�ϵͳ */
	result  = f_mount(NULL, "0:", 0);
}

/*
*********************************************************************************************************
*	�� �� ��: WriteFileTest
*	����˵��: �����ļ���д�ٶ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void WriteFileTest(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	uint32_t i,k;
	uint32_t runtime1,runtime2,timelen;
	uint8_t err = 0;
	char TestFileName[13];
	static uint8_t s_ucTestSn = 0;

	for (i = 0; i < sizeof(g_TestBuf); i++)
	{
		g_TestBuf[i] = (i / 512) + '0';
	}

  	/* �����ļ�ϵͳ */
	result = f_mount(&fs, "0:", 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("�����ļ�ϵͳʧ�� (%s)\r\n",  FR_Table[result]);
	}

	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, "/"); /* ���������������ӵ�ǰĿ¼��ʼ */
	if (result != FR_OK)
	{
		printf("�򿪸�Ŀ¼ʧ�� (%s)\r\n",  FR_Table[result]);
		return;
	}

	/* ���ļ� */
	sprintf(TestFileName, "Speed%02d.txt", s_ucTestSn++);		/* ÿд1�Σ���ŵ��� */
	result = f_open(&file, TestFileName, FA_CREATE_ALWAYS | FA_WRITE);

	/* дһ������ */
	printf("��ʼд�ļ�%s %dKB ...\r\n", TestFileName, TEST_FILE_LEN / 1024);
	runtime1 = bsp_GetRunTime();	/* ��ȡϵͳ����ʱ�� */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		result = f_write(&file, g_TestBuf, sizeof(g_TestBuf), &bw);
		if (result == FR_OK)
		{
			if (((i + 1) % 8) == 0)
			{
				printf(".");
			}
		}
		else
		{
			err = 1;
			printf("%s�ļ�дʧ��\r\n", TestFileName);
			break;
		}
	}
	runtime2 = bsp_GetRunTime();	/* ��ȡϵͳ����ʱ�� */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		printf("\r\n  д��ʱ : %dms   ƽ��д�ٶ� : %dB/S (%dKB/S)\r\n",
			timelen,
			(TEST_FILE_LEN * 1000) / timelen,
			((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}

	f_close(&file);		/* �ر��ļ�*/


	/* ��ʼ���ļ����� */
	result = f_open(&file, TestFileName, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		printf("û���ҵ��ļ�: %s\r\n", TestFileName);
		return;
	}

	printf("��ʼ���ļ� %dKB ...\r\n", TEST_FILE_LEN / 1024);
	runtime1 = bsp_GetRunTime();	/* ��ȡϵͳ����ʱ�� */
	for (i = 0; i < TEST_FILE_LEN / BUF_SIZE; i++)
	{
		result = f_read(&file, g_TestBuf, sizeof(g_TestBuf), &bw);
		if (result == FR_OK)
		{
			if (((i + 1) % 8) == 0)
			{
				printf(".");
			}

			/* �Ƚ�д��������Ƿ���ȷ�������ᵼ�¶����ٶȽ�����͵� 3.5MBytes/S */
			for (k = 0; k < sizeof(g_TestBuf); k++)
			{
				if (g_TestBuf[k] != (k / 512) + '0')
				{
				  	err = 1;
					printf("Speed1.txt �ļ����ɹ����������ݳ���\r\n");
					break;
				}
			}
			if (err == 1)
			{
				break;
			}
		}
		else
		{
			err = 1;
			printf("Speed1.txt �ļ���ʧ��\r\n");
			break;
		}
	}
	runtime2 = bsp_GetRunTime();	/* ��ȡϵͳ����ʱ�� */

	if (err == 0)
	{
		timelen = (runtime2 - runtime1);
		printf("\r\n  ����ʱ : %dms   ƽ�����ٶ� : %dB/S (%dKB/S)\r\n", timelen,
			(TEST_FILE_LEN * 1000) / timelen, ((TEST_FILE_LEN / 1024) * 1000) / timelen);
	}

	/* �ر��ļ�*/
	f_close(&file);

	/* ж���ļ�ϵͳ */
	result  = f_mount(NULL, "0:", 0);
}

/*
*********************************************************************************************************
*	�� �� ��: ModifyFileTest
*	����˵��: �����ļ������޸�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ModifyFileTest(void)
{
   	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
//	uint32_t i,k;
//	uint32_t runtime1,runtime2,timelen;
//	uint8_t err = 0;
	char TestStr[13];
	static uint8_t s_ucTestSn = 0;
	
	/* �����ļ�ϵͳ */
	result = f_mount(&fs, "0:", 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("�����ļ�ϵͳʧ�� (%s)\r\n",  FR_Table[result]);
	}
	
	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, "/"); /* ���������������ӵ�ǰĿ¼��ʼ */
	if (result != FR_OK)
	{
		printf("�򿪸�Ŀ¼ʧ�� (%s)\r\n", FR_Table[result]);
		return;
	}
	/*���ļ�*/
	result = f_open(&file, "armfly.txt", FA_OPEN_ALWAYS  | FA_WRITE |FA_READ);
	if (result !=  FR_OK)
	{
		printf("Don't Find File : armfly.txt\r\n");
		return;
	}
	s_ucTestSn ++;
	sprintf(TestStr,"�ú�ѧϰ%d",s_ucTestSn);
	result = f_write(&file,TestStr,9,&bw);
	if (result == FR_OK)
	{
		printf("armfly.txt �ļ�д��ɹ�\r\n");
	}
	else
	{
		printf("armfly.txt �ļ�д��ʧ��\r\n");
	}
		/* �ر��ļ�*/
	f_close(&file);

	/* ж���ļ�ϵͳ */
	result  = f_mount(NULL, "0:", 0);
	
	
}

static void  ReadDir(void)
{
	FRESULT result;
	FATFS fs;
	DIR dirInfo;
	FILINFO filInfo;
	FIL file;
	uint8_t i;
	uint32_t bw;
	char pathName[32];
	char read_buff[125];
	
	/*�����ļ�ϵͳ*/
	result = f_mount(&fs,"0",0);
	if(result != FR_OK)
	{
		printf("�����ļ�ϵͳʧ��\n");
	}
	
	/*��·��*/
	result = f_opendir(&dirInfo,"0:/DirImage");
	if(result != FR_OK)
	{
		printf("��·��ʧ�� (%s)\r\n", FR_Table[result]);
		return;
	}
	
	/*����Ŀ¼*/
	result = f_mkdir("0:/DirImage/DirTest");
	if(result != FR_OK)
	{
		printf("����·��ʧ�� (%s)\r\n", FR_Table[result]);
		//return;
	}
	
	/*�����ļ� ���ļ�*/
	for(i=0;i<10;i++)
	{
		memset(pathName,0x00,sizeof(pathName));
		sprintf(pathName,"0:/DirImage/test%d.txt",i);
		result = f_open(&file, pathName, FA_OPEN_ALWAYS  | FA_WRITE |FA_READ);
		if (result !=  FR_OK)
		{
			
			printf("Don't Find File : %s\r\n",pathName);
			return;
		}
		else
		{
			result = f_close(&file);
			if (result !=  FR_OK)
			{
				printf("�ر��ļ�ʧ��\n");
				return;
			}
		}
	}
	
	/*������ʾ��ǰĿ¼�����ļ���Ŀ¼*/
	while(1)
	{
		result = f_readdir(&dirInfo,&filInfo);
		if((result != FR_OK)||(filInfo.fname[0]==0))      //�ļ�·����ȡ�������߶���ĩβ��
		{
			break;
		}
		if(filInfo.fattrib & AM_DIR)
		{
			printf("Ŀ¼��%s\r\n",filInfo.fname);
		}
		else
		{
			printf("�ļ���%s\r\n",filInfo.fname);
		}
	}
	
	result  = f_open(&file,"0:/DirImage/test1.txt",FA_WRITE|FA_READ);
	if(result != FR_OK)
	{
       printf("���ļ�ʧ��r\n");
	}
	result = f_write(&file,"write test\r\n",strlen("write test\r\n"),&bw);	
	if (result == FR_OK)
	{
		printf("test1.txt �ļ�д��ɹ�\r\n");
	}
	else
	{
		printf("test1.txt �ļ�д��ʧ��\r\n");
	}	
	
	result = f_lseek(&file,0);
	if (result !=  FR_OK)
	{
		
		printf("f_lseek err\r\n");
		return;
	}
	result = f_read(&file,read_buff,sizeof(read_buff) - 1,&bw);	
	if (bw > 0)
	{
		read_buff[bw] = 0;
		printf("test1.txt �ļ����� : %s", read_buff);
	}
	else
	{
		printf("test1.txt �ļ����� : \r\n");
	}
	
	/* ж���ļ�ϵͳ */
	result  = f_mount(NULL, "0:", 0);
	
	
}


/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
