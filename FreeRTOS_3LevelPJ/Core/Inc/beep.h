/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    beep.h
  * @brief   This file contains all the function prototypes for
  *          the beep.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BEEP_H__
#define __BEEP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "portmacro.h"
#include "systick.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#if 1
//#define BITBAND(addr,bitnum).((addr & 0xF000000) +0x200000 ((addr &0xFFFFF) <<5)+ (bi tnum<<2))
//#define MEM_ADDR (addr)* ((volatile unsigned long *) (addr))
//#define BIT_ADDR (addr, bitnum)  MEM_ADDR(BITBAND (addr, bitnum)

//#define PAout(n) BIT_ADDR(GPIOB_BASE+20,n)
#define BEEP PAout(15)
#define BEEP_PORT   GPIOA
#define BEEP_PIN      GPIO_PIN_15
#define BEEP_PORT_RCC RCC_APB2Periph_GPIOA
#endif
/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */
void BEEP_Init(void);
void Sound(u16 frq);
void play(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ SEG_H__ */

