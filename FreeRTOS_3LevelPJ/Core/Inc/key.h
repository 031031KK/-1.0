/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    key.h
  * @brief   This file contains all the function prototypes for
  *          the key.c file
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
#ifndef __KEY_H__
#define __KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "portmacro.h"
#include "queue.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
//#define ALLOFF   GPIOB->ODR |= (1<<12) + (1<<13) + (1<<14)+ (1<<15)
//#define NO_DOT 255u
	
/* 04<|BK2(J1<d50ms, 5%N;10ms
 V;SPA,Px<l2b5=50msW4L,2;1d2EHON*SPP'#,0|@(5/Fp:M04OBA=VVJB<~
*/
#define BUTTON_FILTER_TIME 	5
//#define BUTTON_LONG_TIME 	100		/* 3VPx1Ck#,HON*3$04JB<~ */
#define BUTTON_LONG_TIME 	170		/* 3VPx1.7Ck#,HON*3$04JB<~ */

#define ST2_ST3_BIT_GROUP	(1<<9)
#define ST2_BIT_LONG			(1<<8)
#define ST3_BIT_LONG			(1<<7)
#define ST4_BIT_LONG			(1<<6)
#define ST2_BIT_SHORT			(1<<5)
#define ST3_BIT_SHORT			(1<<4)
#define ST4_BIT_SHORT			(1<<3)
#define ST2_BIT_UP				(1<<2)
#define ST3_BIT_UP				(1<<1)
#define ST4_BIT_UP				(1<<0)
		
#define ST2_KEY_LONG	0x124
#define ST2_KEY_SHORT	0x024

#define ST3_KEY_LONG	0x092
#define ST3_KEY_SHORT	0x012

#define ST4_KEY_LONG	0x049
#define ST4_KEY_SHORT	0x009

#define ST2_ST3_GROUP		0x236

/*
	C?8v04<|6TS&18vH+>V5D=a99Le1dA?!#
	Fd3IT11dA?JGJ5OVBK2(:M6`VV04<|W4L,Ky1XPk5D
*/
typedef struct
{
	/* OBCfJGR;8v:/J}V8Uk#,V8OrEP6O04<|JV7q04OB5D:/J} */
	uint8_t (*IsKeyDownFunc)(void);		/* 04<|04OB5DEP6O:/J},11mJ>04OB */
	uint8_t Count;										/* BK2(Fw<FJ}Fw */
	uint8_t FilterTime;								/* BK2(J1<d(Wn4s255,1mJ>2550ms) */
	uint16_t LongCount;								/* 3$04<FJ}Fw */
	uint16_t LongTime;								/* 04<|04OB3VPxJ1<d, 01mJ>2;<l2b3$04 */
	uint8_t  State;										/* 04<|51G0W4L,#(04OB;9JG5/Fp#) */
	uint8_t KeyCodeUp;								/* 04<|5/Fp5D<|V54zBk, 01mJ>2;<l2b04<|5/Fp */
	uint8_t KeyCodeDown;							/* 04<|04OB5D<|V54zBk, 01mJ>2;<l2b04<|04OB */
	uint8_t KeyCodeLong;							/* 04<|3$045D<|V54zBk, 01mJ>2;<l2b3$04 */
	uint8_t RepeatSpeed;							/* A,Px04<|V\FZ */
	uint8_t RepeatCount;							/* A,Px04<|<FJ}Fw */
}BUTTON_T;

/* 6(Re<|V54zBk
	MF<vJ9SCenum, 2;SC#define#,T-Rr#:
	(1) 1cSZPBTv<|V5,7=1c5wU{K3Pr#,J94zBk?4Fp@4Jf7~5c
	(2)	1`RkFw?I0oNRCG1\Cb<|V5VX84!#
*/
typedef enum
{
	KEY_NONE = 0,			/* 0 1mJ>04<|JB<~ */

	KEY_DOWN_BTN1,		/* BTN1<|04OB */
	KEY_UP_BTN1,			/* BTN1<|5/Fp */
	KEY_LONG_BTN1,		/* BTN1<|3$04 */

	KEY_DOWN_BTN2,		/* BTN2<|04OB */
	KEY_UP_BTN2,			/* BTN2<|5/Fp */
	KEY_LONG_BTN2,		/* BTN2<|3$04 */
	
	KEY_DOWN_BTN3,		/* BTN3<|04OB */
	KEY_UP_BTN3,			/* BTN3<|5/Fp */
	KEY_LONG_BTN3,		/* BTN3<|3$04 */

	KEY_DOWN_BTN1_BTN2	/* Wi:O<|#,BTN1<|:MBTN2<|M,J104OB */
}KEY_ENUM;

/* 04<|FIFOSC5=1dA? */
#define KEY_FIFO_SIZE	20
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE];		/* <|V5;:3eGx */
	uint8_t Read;									/* ;:3eGx6AV8Uk */
	uint8_t Write;								/* ;:3eGxP4V8Uk */
}KEY_FIFO_T;
/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */
void bsp_InitButton(void);
void bsp_PutKey(uint8_t _KeyCode);
uint8_t bsp_GetKey(void);
void bsp_KeyPro(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ KEY_H__ */

