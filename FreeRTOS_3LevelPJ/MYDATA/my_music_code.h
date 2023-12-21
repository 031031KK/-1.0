#ifndef __MY_MUSIC_CODE_H
#define __MY_MUSIC_CODE_H
#include "main.h"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

//定义拍
#define P_4_1 1
#define P_2_1 2
#define P_4_3 3
#define P_1 4
#define P_2 5
#define P_3 6
#define P_4 7

//C调音符与频率
extern uint16_t NOTE_FREQ[];

//====音乐数量====== 
#define MYMUSIC_NUM 3

struct MYMUSIC_T{
	  u8 now_flg;//当前歌曲
		u8 *f;//频率表
		u8 *t;//拍数表
		u16 t_each;//每1/4拍大小
	  u16 len;//数据长度
	  u16 now_len;
	  u8 *name;
};

extern struct MYMUSIC_T mymusic_t;


void my_music_code_get(u8 n);
#endif
