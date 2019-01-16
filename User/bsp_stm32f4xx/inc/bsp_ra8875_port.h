/*
*********************************************************************************************************
*
*	ģ������ : RA8875оƬ��MCU֮��Ľӿ�����
*	�ļ����� : bsp_ra8875_port.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/


#ifndef _BSP_RA8875_PORT_H
#define _BSP_RA8875_PORT_H

//#define RA_SOFT_SPI	   	/* ���SPI�ӿ�ģʽ */
//#define RA_HARD_SPI	   	/* Ӳ��SPI�ӿ�ģʽ */
//#define RA_SOFT_8080_8	/* ���ģ��8080�ӿ�,8bit */
#define RA_HARD_8080_16	/* Ӳ��8080�ӿ�,16bit */

void RA8875_Delaly1us(void);
void RA8875_Delaly1ms(void);
uint16_t RA8875_ReadID(void);
void RA8875_WriteCmd(uint8_t _ucRegAddr);
void RA8875_WriteData(uint8_t _ucRegValue);
uint8_t RA8875_ReadData(void);
void RA8875_WriteData16(uint16_t _usRGB);
uint16_t RA8875_ReadData16(void);
uint8_t RA8875_ReadStatus(void);
uint32_t RA8875_GetDispMemAddr(void);

#ifdef RA_HARD_SPI	   /* ����SPI�ӿ�ģʽ */
	void RA8875_InitSPI(void);
	void RA8875_HighSpeedSPI(void);
	void RA8875_LowSpeedSPI(void);
#endif

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
