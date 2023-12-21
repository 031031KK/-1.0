#include "my_music_code.h"

uint16_t NOTE_FREQ[]={0,
	262,294,330,349,392,440,494,
	523,587,659,698,784,880,988,
	1046,1175,1318,1397,1568,1760,1976
};

//-------------------两只老虎--------------------

//频率
uint8_t music_arr_tiger_f[]={
  1,2,3,1,
	1,2,3,1,
	3,4,5,
	3,4,5,
	5,6,5,6,3,1,
	5,6,5,4,3,1,
	1,5,1,
	1,5,1
};

// 1/4拍长
u16 music_arr_tiger_t_echo=150;

//长度
u16 music_arr_tiger_len=(sizeof(music_arr_tiger_f)/sizeof(uint8_t));

//拍长
uint8_t music_arr_tiger_t[]={
  P_1,P_1,P_1,P_1,
	P_1,P_1,P_1,P_1,
	P_1,P_1,P_2,
	P_1,P_1,P_2,
	P_4_3,P_4_1,P_4_3,P_4_1,P_1,P_1,
	P_4_3,P_4_1,P_4_3,P_4_1,P_1,P_1,
	P_1,P_1,P_2,
	P_1,P_1,P_2,
};

//-------------------------春节序曲------------------
//频率
uint8_t music_arr_newyear_f[]={
  3,2,3,5,5,6,
	7,6,7,7,
	3,7,6,5,3,5,
	6,5,6,5,
	5,6,1,6,1,2,
	1,6,5,3,
	5,6,1,2,1,7,
	6,5,6,5,
	3,2,3,5,5,6,
	1,6,3,1,

};

// 1/4拍长
u16 music_arr_newyear_t_echo=200;

//长度
u16 music_arr_newyear_len=(sizeof(music_arr_newyear_f)/sizeof(uint8_t));

//拍长
uint8_t music_arr_newyear_t[]={
  P_2_1,P_1,P_2_1,P_2_1,P_1,P_2_1,
	P_1,P_2_1,P_2_1,P_2,
	P_2_1,P_1,P_2_1,P_2_1,P_1,P_2_1,
	P_4_3,P_4_1,P_1,P_2,
	P_4_3,P_4_1,P_1,P_2_1,P_1,P_2_1,
	P_1,P_2_1,P_2_1,P_2,
	P_4_3,P_4_1,P_1,P_2_1,P_1,P_2_1,
	P_4_3,P_4_1,P_1,P_2,
	P_2_1,P_1,P_2_1,P_2_1,P_1,P_2_1,
	P_1,P_2_1,P_2_1,P_2,
};

//-------------------------难忘今宵------------------
//频率
uint8_t music_arr_unforgetable_f[]={
  2,3,2,1,2,3,2,5,
	5,5,5,2,4,3,2,
	7,1,2,3,1,7,1,2,3,5,
	5,4,3,4,3,2,1,
	
	5,6,5,4,1,3,4,6,5,5,
	2,6,5,6,4,6,5,
	3,4,3,2,5,3,4,3,2,1,
	
	5,4,3,4,3,2,1,
	6,5,4,1,4,6,5,5,
	6,5,4,1,4,6,5,5,
	
	3,2,1,5,1,3,2,2,
	3,2,1,5,1,3,5,5,
	3,2,1,5,1,3,1,1

};

// 1/4拍长
u16 music_arr_unforgetable_t_echo=300;

//长度
u16 music_arr_unforgetable_len=(sizeof(music_arr_unforgetable_f)/sizeof(uint8_t));
	
//拍长
uint8_t music_arr_unforgetable_t[]={
  P_2_1,P_4_1,P_4_1,P_1,P_2_1,P_4_1,P_4_1,P_1,
	P_2_1,P_2_1,P_4_1,P_4_1,P_4_1,P_4_1,P_2,
	P_4_1,P_4_1,P_4_1,P_4_1,P_1,P_4_1,P_4_1,P_4_1,P_4_1,P_1,
	P_2_1,P_2_1,P_4_1,P_4_1,P_4_1,P_4_1,P_2,
	
  P_2_1,P_4_1,P_4_1,P_2_1,P_2_1,P_4_1,P_4_1,P_4_1,P_4_1,P_1,
	P_2_1,P_2_1,P_4_1,P_4_1,P_4_1,P_4_1,P_2,
	P_4_1,P_4_1,P_4_1,P_4_1,P_1,P_4_1,P_4_1,P_4_1,P_4_1,P_1,
	
	P_2_1,P_2_1,P_4_1,P_4_1,P_4_1,P_4_1,P_2,
	P_4_1,P_4_1,P_4_1,P_4_1,P_4_1,P_4_1,P_2_1,P_2,
	P_4_1,P_4_1,P_4_1,P_4_1,P_4_1,P_4_1,P_2_1,P_2,
	
	P_4_1,P_4_1,P_4_1,P_4_1,P_4_1,P_4_1,P_2_1,P_2,
	P_4_1,P_4_1,P_4_1,P_4_1,P_4_1,P_4_1,P_2_1,P_2,
	P_4_1,P_4_1,P_4_1,P_4_1,P_4_1,P_4_1,P_2_1,P_2,
	
};



struct MYMUSIC_T mymusic_t;

u8 M0[]="stop";
u8 M1[]="tow tigers";
u8 M2[]="Spring Festival";
u8 M3[]="unforgetable";

void my_music_code_get(u8 n){
   switch(n){
	   case 0 :{
					mymusic_t.f=NULL;
					mymusic_t.t=NULL;
			    mymusic_t.len=0;
					mymusic_t.t_each=0;
			    mymusic_t.name=M0;
		 };
		 break;
	   case 1:{
					mymusic_t.f=music_arr_tiger_f;
					mymusic_t.t=music_arr_tiger_t;
			    mymusic_t.len=music_arr_tiger_len;
					mymusic_t.t_each=music_arr_tiger_t_echo;
			    mymusic_t.name=M1;
		 };
		 break;
	   case 2:{

					mymusic_t.f=music_arr_newyear_f;
					mymusic_t.t=music_arr_newyear_t;
			    mymusic_t.len=music_arr_newyear_len;
					mymusic_t.t_each=music_arr_newyear_t_echo;
			    mymusic_t.name=M2;
		 };
		 break;
	   case 3:{
			 
					mymusic_t.f=music_arr_unforgetable_f;
					mymusic_t.t=music_arr_unforgetable_t;
			    mymusic_t.len=music_arr_unforgetable_len;
					mymusic_t.t_each=music_arr_unforgetable_t_echo;
			    mymusic_t.name=M3;
		 };
		 break;
	   case 4:{

		 };
		 break;
	   case 5:{

		 };
		 break;
	 }

}


