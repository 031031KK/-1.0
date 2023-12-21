#ifndef __MY_OLED_H
#define __MY_OLED_H
#include "main.h"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

extern const unsigned char Start_BMP[];
extern const unsigned char meme1[][8*36];

/***********������������ʾ��ʾBMPͼƬ128��64��ʼ������(x,y),x�ķ�Χ0��127��yΪҳ�ķ�Χ0��7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,const unsigned char BMP[]);

void fill_picture(unsigned char fill_Data);

//��������
void OLED_Set_Pos(unsigned char x, unsigned char y);
//����OLED��ʾ
void OLED_Display_On(void);
//�ر�OLED��ʾ
void OLED_Display_Off(void);
//��������,������,������Ļ�Ǻ�ɫ��!��û����һ��!!!
void OLED_Clear(void);
void OLED_On(void);
//��ָ��λ����ʾһ���ַ�,���������ַ�
//x:0~127
//y:0~63
//mode:0,������ʾ;1,������ʾ
//size:ѡ������ 16/12
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 Char_Size);

//��ʾһ���ַ��Ŵ�
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 Char_Size);

void OLED_show_china(unsigned char x,unsigned char y,unsigned char datax);

//��ʼ��SSD1306
void OLED_Init(void);

void my_oled_square(uint8_t sx,uint8_t sy,uint8_t ex,uint8_t ey);

void my_oled_show_progress(u32 max,u32 now);

void OLED_meme_animation();
#endif
