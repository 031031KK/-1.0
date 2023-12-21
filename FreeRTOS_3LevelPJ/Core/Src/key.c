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
#include "key.h"

/* USER CODE BEGIN 0 */
static BUTTON_T s_Btn1;					/* SW2键PA4*/
static BUTTON_T s_Btn2;					/* SW3键PA5*/
static BUTTON_T s_Btn3;					/* SW4键PA6*/
static BUTTON_T s_Btn1AndBtn2;	/* 组合键，SW1键&SW2键 */

KEY_FIFO_T s_Key;		/* 按键FIFO变量,结构体 */

static void bsp_InitButtonVar(void);
//static void bsp_InitButtonHard(void);
static void bsp_DetectButton(BUTTON_T *_pBtn);


static uint8_t IsKeyDownBtn1(void) 		{if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET) return 0; return 1;}
static uint8_t IsKeyDownBtn2(void) 		{if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET) return 0; return 1;}
static uint8_t IsKeyDownBtn3(void) 		{if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET) return 0; return 1;}
static uint8_t IsKeyDownBtn1Btn2(void) {if (IsKeyDownBtn1() && IsKeyDownBtn2()) return 1; return 0;}	/* 组合键 */
/* USER CODE END 0 */


/* USER CODE BEGIN 2 */
/*
*********************************************************************************************************
*	函 数 名: bsp_InitButton
*	功能说明: 初始化按键
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitButton(void)
{
	bsp_InitButtonVar();		/* 初始化按键变量 */
	//bsp_InitButtonHard();		/* 初始化按键硬件 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_PutKey
*	功能说明: 将1个键值压入按键FIFO缓冲区。可用于模拟一个按键。
*	形    参：_KeyCode : 按键代码
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_PutKey(uint8_t _KeyCode)
{
	s_Key.Buf[s_Key.Write] = _KeyCode;

	if (++s_Key.Write  >= KEY_FIFO_SIZE)
	{
		s_Key.Write = 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_GetKey
*	功能说明: 从按键FIFO缓冲区读取一个键值。
*	形    参：无
*	返 回 值: 按键代码
*********************************************************************************************************
*/
uint8_t bsp_GetKey(void)
{
	uint8_t ret;

	if (s_Key.Read == s_Key.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_Key.Buf[s_Key.Read];

		if (++s_Key.Read >= KEY_FIFO_SIZE)
		{
			s_Key.Read = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitButtonHard
*	功能说明: 初始化按键硬件
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
/*
	实验板原理图：
	SW2键  : PA4 (低电平表示按下)
	SW3键  : PA5 (低电平表示按下)
	SW4键  : PA6 (低电平表示按下)
	定义函数判断按键是否按下，返回值1 表示按下，0表示未按下
*/
//static void bsp_InitButtonHard(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;

//	/* 第1步：打开GPIOB的时钟
//	   注意：这个地方可以一次性打开多个GPIO口的时钟
//	*/
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//	
//	/* 第2步：配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
//	GPIO_Init(GPIOA, &GPIO_InitStructure);	/* PA4,PA5,PA6 */
//}
	
/*
*********************************************************************************************************
*	函 数 名: bsp_InitButtonVar
*	功能说明: 初始化按键变量
*	形    参：strName : 例程名称字符串
*			  strDate : 例程发布日期
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_InitButtonVar(void)
{
	/* 对按键FIFO读写指针清零 */
	s_Key.Read = 0;
	s_Key.Write = 0;

	/* 初始化BTN1按键变量，支持按下、弹起、长按 */
	s_Btn1.IsKeyDownFunc = IsKeyDownBtn1;			/* 判断按键按下的函数 */
	s_Btn1.FilterTime = BUTTON_FILTER_TIME;		/* 按键滤波时间 */
	s_Btn1.LongTime = BUTTON_LONG_TIME;				/* 长按时间 */
	s_Btn1.Count = s_Btn1.FilterTime / 2;			/* 计数器设置为滤波时间的一半 */
	s_Btn1.State = 0;													/* 按键缺省状态，0为未按下 */
	s_Btn1.KeyCodeDown = KEY_DOWN_BTN1;				/* 按键按下的键值代码 */
	s_Btn1.KeyCodeUp = KEY_UP_BTN1;						/* 按键弹起的键值代码 */
	s_Btn1.KeyCodeLong = KEY_LONG_BTN1;				/* 按键被持续按下的键值代码 */
	s_Btn1.RepeatSpeed = 0;										/* 按键连发的速度，0表示不支持连发 */
	s_Btn1.RepeatCount = 0;										/* 连发计数器 */		

	/* 初始化BTN2按键变量，支持按下、弹起、长按 */
	s_Btn2.IsKeyDownFunc = IsKeyDownBtn2;			/* 判断按键按下的函数 */
	s_Btn2.FilterTime = BUTTON_FILTER_TIME;		/* 按键滤波时间 */
	s_Btn2.LongTime = BUTTON_LONG_TIME;				/* 长按时间 */
	s_Btn2.Count = s_Btn2.FilterTime / 2;			/* 计数器设置为滤波时间的一半 */
	s_Btn2.State = 0;													/* 按键缺省状态，0为未按下 */
	s_Btn2.KeyCodeDown = KEY_DOWN_BTN2;				/* 按键按下的键值代码 */
	s_Btn2.KeyCodeUp = KEY_UP_BTN2;						/* 按键弹起的键值代码 */
	s_Btn2.KeyCodeLong = KEY_LONG_BTN2;				/* 按键被持续按下的键值代码 */
	s_Btn2.RepeatSpeed = 0;										/* 按键连发的速度，0表示不支持连发 */
	s_Btn2.RepeatCount = 0;										/* 连发计数器 */	
	
		/* 初始化BTN2按键变量，支持按下、弹起、长按 */
	s_Btn3.IsKeyDownFunc = IsKeyDownBtn3;			/* 判断按键按下的函数 */
	s_Btn3.FilterTime = BUTTON_FILTER_TIME;		/* 按键滤波时间 */
	s_Btn3.LongTime = BUTTON_LONG_TIME;				/* 长按时间 */
	s_Btn3.Count = s_Btn3.FilterTime / 2;			/* 计数器设置为滤波时间的一半 */
	s_Btn3.State = 0;													/* 按键缺省状态，0为未按下 */
	s_Btn3.KeyCodeDown = KEY_DOWN_BTN3;				/* 按键按下的键值代码 */
	s_Btn3.KeyCodeUp = KEY_UP_BTN3;						/* 按键弹起的键值代码 */
	s_Btn3.KeyCodeLong = KEY_LONG_BTN3;				/* 按键被持续按下的键值代码 */
	s_Btn3.RepeatSpeed = 0;										/* 按键连发的速度，0表示不支持连发 */
	s_Btn3.RepeatCount = 0;										/* 连发计数器 */

	 /* 初始化组合按键变量，支持按下 */
	 s_Btn1AndBtn2.IsKeyDownFunc = IsKeyDownBtn1Btn2;			/* 判断按键按下的函数 */
	 s_Btn1AndBtn2.FilterTime = BUTTON_FILTER_TIME;				/* 按键滤波时间 */
	 s_Btn1AndBtn2.LongTime = 0;													/* 长按时间 */
	 s_Btn1AndBtn2.Count = s_Btn1AndBtn2.FilterTime / 2;	/* 计数器设置为滤波时间的一半 */
	 s_Btn1AndBtn2.State = 0;															/* 按键缺省状态，0为未按下 */
	 s_Btn1AndBtn2.KeyCodeDown = KEY_DOWN_BTN1_BTN2;			/* 按键按下的键值代码 */
	 s_Btn1AndBtn2.KeyCodeUp = 0;													/* 按键弹起的键值代码，0表示不检测 */
	 s_Btn1AndBtn2.KeyCodeLong = 0;												/* 按键被持续按下的键值代码，0表示不检测 */
	 s_Btn1AndBtn2.RepeatSpeed = 0;												/* 按键连发的速度，0表示不支持连发 */
	 s_Btn1AndBtn2.RepeatCount = 0;												/* 连发计数器 */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DetectButton
*	功能说明: 检测一个按键。非阻塞状态，必须被周期性的调用。
*	形    参：按键结构变量指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_DetectButton(BUTTON_T *_pBtn)
{
	/* 如果没有初始化按键函数，则报错
	if (_pBtn->IsKeyDownFunc == 0)
	{
		printf("Fault : DetectButton(), _pBtn->IsKeyDownFunc undefine");
	}
	*/

	if (_pBtn->IsKeyDownFunc())
	{
		if (_pBtn->Count < _pBtn->FilterTime)
		{
			_pBtn->Count = _pBtn->FilterTime;
		}
		else if(_pBtn->Count < 2 * _pBtn->FilterTime)
		{
			_pBtn->Count++;
		}
		else
		{
			if (_pBtn->State == 0)
			{
				_pBtn->State = 1;

				/* 发送按钮按下的消息 */
				if (_pBtn->KeyCodeDown > 0)
				{
					
				/* 键值放入按键FIFO */
					bsp_PutKey(_pBtn->KeyCodeDown);
				}
			}

			if (_pBtn->LongTime > 0)
			{
				if (_pBtn->LongCount < _pBtn->LongTime)
				{
					/* 发送按钮持续按下的消息 */
					if (++_pBtn->LongCount == _pBtn->LongTime)
					{
						/* 键值放入按键FIFO */
						bsp_PutKey(_pBtn->KeyCodeLong);						
					}
				}
				else
				{
					if (_pBtn->RepeatSpeed > 0)
					{
						if (++_pBtn->RepeatCount >= _pBtn->RepeatSpeed)
						{
							_pBtn->RepeatCount = 0;
							/* 常按键后，每隔10ms发送1个按键 */
							bsp_PutKey(_pBtn->KeyCodeDown);														
						}
					}
				}
			}
		}
	}
	else
	{
		if(_pBtn->Count > _pBtn->FilterTime)
		{
			_pBtn->Count = _pBtn->FilterTime;
		}
		else if(_pBtn->Count != 0)
		{
			_pBtn->Count--;
		}
		else
		{
			if (_pBtn->State == 1)
			{
				_pBtn->State = 0;

				/* 发送按钮弹起的消息 */
				if (_pBtn->KeyCodeUp > 0)
				{
					/* 键值放入按键FIFO */
					bsp_PutKey(_pBtn->KeyCodeUp);			
				}
			}
		}

		_pBtn->LongCount = 0;
		_pBtn->RepeatCount = 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_KeyPro
*	功能说明: 检测所有按键。非阻塞状态，必须被周期性的调用。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_KeyPro(void)
{
	bsp_DetectButton(&s_Btn1);		/* SW2键 */
	bsp_DetectButton(&s_Btn2);		/* SW3键 */
	bsp_DetectButton(&s_Btn3);		/* SW4键 */
	bsp_DetectButton(&s_Btn1AndBtn2);/* 组合键，SW2键&SW3键 */
}
/* USER CODE END 0 */


/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
