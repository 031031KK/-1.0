#ifndef __MY_MUSIC_CODE_H
#define __MY_MUSIC_CODE_H
#include "main.h"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

//������
#define P_4_1 1
#define P_2_1 2
#define P_4_3 3
#define P_1 4
#define P_2 5
#define P_3 6
#define P_4 7

//C��������Ƶ��
extern uint16_t NOTE_FREQ[];

//====��������====== 
#define MYMUSIC_NUM 3

struct MYMUSIC_T{
	  u8 now_flg;//��ǰ����
		u8 *f;//Ƶ�ʱ�
		u8 *t;//������
		u16 t_each;//ÿ1/4�Ĵ�С
	  u16 len;//���ݳ���
	  u16 now_len;
	  u8 *name;
};

extern struct MYMUSIC_T mymusic_t;


void my_music_code_get(u8 n);
#endif
