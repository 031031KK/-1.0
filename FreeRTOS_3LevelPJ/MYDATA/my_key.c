#include "my_oled.h"
#include "my_delay.h"
#include "my_music_code.h"



////C��������Ƶ��
//extern uint16_t NOTE_FREQ[];

////====��������====== 
//#define MYMUSIC_NUM 3

//struct MYMUSIC_T{
//	  u8 now_flg;//��ǰ����
//		u8 *f;//Ƶ�ʱ�
//		u8 *t;//������
//		u16 t_each;//ÿ1/4�Ĵ�С
//	  u16 len;//���ݳ���
//	  u16 now_len;
//};

//extern struct MYMUSIC_T mymusic_t;

	//GPIO�жϻص�����
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

   //�жϽ����жϵ�GPIOs
    if(KEY1_Pin==GPIO_Pin){
       for(int i=0;i<10000;i++) for(int j=0;j<300;j++);
			 if(0 == HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)){
				 
			  if(mymusic_t.now_flg<MYMUSIC_NUM) mymusic_t.now_flg++;
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);//���������ƽȡ��
				 
			 }
			
     }
		   //�жϽ����жϵ�GPIOs
    if(KEY2_Pin==GPIO_Pin){
       for(int i=0;i<10000;i++) for(int j=0;j<300;j++);
			 if(0 == HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)){
				 
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);//���������ƽȡ��

			  if(mymusic_t.now_flg>0) mymusic_t.now_flg--;
				 
			 }

     }

}