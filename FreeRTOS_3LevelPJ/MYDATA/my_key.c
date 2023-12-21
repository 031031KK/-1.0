#include "my_oled.h"
#include "my_delay.h"
#include "my_music_code.h"



////C调音符与频率
//extern uint16_t NOTE_FREQ[];

////====音乐数量====== 
//#define MYMUSIC_NUM 3

//struct MYMUSIC_T{
//	  u8 now_flg;//当前歌曲
//		u8 *f;//频率表
//		u8 *t;//拍数表
//		u16 t_each;//每1/4拍大小
//	  u16 len;//数据长度
//	  u16 now_len;
//};

//extern struct MYMUSIC_T mymusic_t;

	//GPIO中断回调函数
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

   //判断进入中断的GPIOs
    if(KEY1_Pin==GPIO_Pin){
       for(int i=0;i<10000;i++) for(int j=0;j<300;j++);
			 if(0 == HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)){
				 
			  if(mymusic_t.now_flg<MYMUSIC_NUM) mymusic_t.now_flg++;
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);//单独输出电平取反
				 
			 }
			
     }
		   //判断进入中断的GPIOs
    if(KEY2_Pin==GPIO_Pin){
       for(int i=0;i<10000;i++) for(int j=0;j<300;j++);
			 if(0 == HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)){
				 
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);//单独输出电平取反

			  if(mymusic_t.now_flg>0) mymusic_t.now_flg--;
				 
			 }

     }

}