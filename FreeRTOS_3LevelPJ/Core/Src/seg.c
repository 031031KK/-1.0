/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "seg.h"

/* USER CODE BEGIN 0 */
static unsigned char DispBuffer[4]={0xff,0xff,0xff,0xff}; //显示缓冲区

/*共阳数码管显示码表
bit7					bit0
DP G F E D C B A
因为是共阳，所以低电平（0）时亮，高电平（1）时灭
*/
static unsigned char table[24]={
	0xc0,		//0
	0xf9,		//1
	0xa4,		//2
	0xb0,		//3
	0x99,		//4
	0x92,		//5
	0x82,		//6
	0xf8,		//7
	0x80,		//8
	0x90,		//9
	0x88,		//A
	0x83,		//B
	0xc6,		//C
	0xa1,		//D
	0x86,		//E
	0x8e, 	//F
	0x8c, 	//P
	0xc1,		//U
	0x91,		//Y
	0xc7,		//L
	0x00,		/* *:全亮 */
	0xff, 	/* #:熄灭 */
	0xbf,		/* -:横杠 */
	0x8F		/* $:EFG亮，开机LED动画使用 */
};

void GPIO_ResetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
  assert_param(IS_GPIO_PIN(GPIO_Pin));
  
  GPIOx->BRR = GPIO_Pin;
}

unsigned char LEDDisplaySearchTable(char a)
{
	unsigned char ret = 0;
	
	switch(a)
	{
		case '0':
			ret = table[0];
			break;
		case '1':
			ret = table[1];
			break;
		case '2':
			ret = table[2];
			break;
		case '3':
			ret = table[3];
			break;
		case '4':
			ret = table[4];
			break;
		case '5':
			ret = table[5];
			break;
		case '6':
			ret = table[6];
			break;
		case '7':
			ret = table[7];
			break;
		case '8':
			ret = table[8];
			break;
		case '9':
			ret = table[9];
			break;
		case 'A':
			ret = table[10];
			break;
		case 'B':
			ret = table[11];
			break;
		case 'C':
			ret = table[12];
			break;
		case 'D':
			ret = table[13];
			break;
		case 'E':
			ret = table[14];
			break;
		case 'F':
			ret = table[15];
			break;
		case 'P':
			ret = table[16];
			break;
		case 'U':
			ret = table[17];
			break;
		case 'Y':
			ret = table[18];
			break;
		case 'L':
			ret = table[19];
			break;
		case '*':
			ret = table[20];
			break;
		case '#':
			ret = table[21];
			break;
		case '-':
			ret = table[22];
			break;
		case '$':
			ret = table[23];
			break;
	}
	return ret;
}

void LEDDisplayString(char str[], unsigned char dotPos)
{
	int i;
	for(i=0;i<4;i++) //拆分数字，最多显示4位
	{
		DispBuffer[i] = LEDDisplaySearchTable(str[3-i]); //临时存放
	}	
	
	if(dotPos < 4)
	{
		DispBuffer[dotPos] -= 0x80; //显示小数点
	}
}

void LEDDisplay2Half(char a, unsigned char number, unsigned char dotPos)
{
	unsigned char digit; //码表数组下标
	unsigned char digitseg; //存放码表变量
	unsigned char SegBuff[4]; //码表临时存放数组
	unsigned char i;
	
	DispBuffer[3] = LEDDisplaySearchTable(a); //临时存放
	
	for(i=0;i<3;i++) //拆分数字，最多显示3位
	{
		digit = number % 10; //拆分数字，取余操作
		number /= 10; //拆分数字，除10操作
		digitseg = table[digit]; //查表，得到7段字形表
		SegBuff[i] = digitseg; //临时存放
	}
	DispBuffer[0] = SegBuff[0]; //写入显示缓存
	DispBuffer[1] = SegBuff[1]; //写入显示缓存
	DispBuffer[2] = SegBuff[2]; //写入显示缓存
	
	if(dotPos < 4)
	{
		DispBuffer[dotPos] -= 0x80; //显示小数点
	}
}

void LEDDisplayNumber(unsigned int number, unsigned char dotPos)
{
	unsigned char digit; //码表数组下标
	unsigned char digitseg; //存放码表变量
	unsigned char SegBuff[4]; //码表临时存放数组
	unsigned char i;
	for(i=0;i<4;i++) //拆分数字，最多显示4位
	{
		digit = number % 10; //拆分数字，取余操作
		number /= 10; //拆分数字，除10操作
		digitseg = table[digit]; //查表，得到7段字形表
		SegBuff[i] = digitseg; //临时存放
	}
	DispBuffer[0] = SegBuff[0]; //写入显示缓存
	DispBuffer[1] = SegBuff[1]; //写入显示缓存
	DispBuffer[2] = SegBuff[2]; //写入显示缓存
	DispBuffer[3] = SegBuff[3];	//写入显示缓存
	
	if(dotPos < 4)
	{
		DispBuffer[dotPos] -= 0x80; //显示小数点
	}
}

void DisplayScanOneTimeOneSeg(void)
{
	static unsigned short com;//扫描计数变量
	
	com++;				//每次调用后切换一次显示
	if(com >= 4)	//com的值在0.1.2.3之间切换
		com = 0;
	ALLOFF;				//切换前将全部显示暂时关闭，避免虚影
	
	switch(com)
	{
		case 0:
			GPIOB->ODR =0xFFFF;
			GPIO_ResetBits(GPIOB,GPIO_PIN_12);
			GPIOB->ODR &= ((DispBuffer[3])|0xFF00);		
			break;
		case 1:
			GPIOB->ODR =0xFFFF;
			GPIO_ResetBits(GPIOB,GPIO_PIN_13);
			GPIOB->ODR &= ((DispBuffer[2])|0xFF00);		
			break;
		case 2:
			GPIOB->ODR =0xFFFF;
			GPIO_ResetBits(GPIOB,GPIO_PIN_14);
			GPIOB->ODR &= ((DispBuffer[1])|0xFF00);	
			break;
		case 3:
			GPIOB->ODR =0xFFFF;
			GPIO_ResetBits(GPIOB,GPIO_PIN_15);
			GPIOB->ODR &= ((DispBuffer[0])|0xFF00);	
			break;		
	}
}

/* USER CODE END 0 */
