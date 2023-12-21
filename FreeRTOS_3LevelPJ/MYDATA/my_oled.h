#ifndef __MY_OLED_H
#define __MY_OLED_H
#include "main.h"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

extern const unsigned char Start_BMP[];
extern const unsigned char meme1[][8*36];

/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,const unsigned char BMP[]);

void fill_picture(unsigned char fill_Data);

//坐标设置
void OLED_Set_Pos(unsigned char x, unsigned char y);
//开启OLED显示
void OLED_Display_On(void);
//关闭OLED显示
void OLED_Display_Off(void);
//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!
void OLED_Clear(void);
void OLED_On(void);
//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示
//size:选择字体 16/12
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 Char_Size);

//显示一个字符号串
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 Char_Size);

void OLED_show_china(unsigned char x,unsigned char y,unsigned char datax);

//初始化SSD1306
void OLED_Init(void);

void my_oled_square(uint8_t sx,uint8_t sy,uint8_t ex,uint8_t ey);

void my_oled_show_progress(u32 max,u32 now);

void OLED_meme_animation();
#endif
