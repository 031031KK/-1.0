#ifndef __SysTick__H

#define __SysTick__H

#include "stm32f103xb.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned short uc16;
typedef unsigned int u32;

void SysTick_Init(u8 SYSCLK);

void delay_us(u32 nus);

void delay_ms(u16 nms);

#endif
