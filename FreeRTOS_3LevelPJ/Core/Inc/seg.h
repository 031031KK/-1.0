/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    seg.h
  * @brief   This file contains all the function prototypes for
  *          the seg.c file
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
#ifndef __SEG_H__
#define __SEG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "portmacro.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define ALLOFF   GPIOB->ODR |= (1<<12) + (1<<13) + (1<<14)+ (1<<15)
#define NO_DOT 255u
#define ALL_DOT 4u
/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */
void LEDDisplayString(char str[], unsigned char dotPos);
void LEDDisplay2Half(char a, unsigned char number, unsigned char dotPos);
void LEDDisplayNumber(unsigned int number, unsigned char dotPos);
void DisplayScanOneTimeOneSeg(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ SEG_H__ */

