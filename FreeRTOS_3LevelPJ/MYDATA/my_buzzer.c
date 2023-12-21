#include "my_buzzer.h"
#include "my_music_code.h"
#include "tim.h"

#define NOTE_VOL 99
/*

����������Ƶ�ʷ�����

note_f��Ƶ��,ȡֵ����������

vol:������ȡֵ��Χ����:[0,100]

*/
void my_passive_buzzer_set(uint16_t note_f)
{
		//�����Զ���װ��ֵ,�����µ�Ƶ��
		uint16_t Autoreload=(8000000.0/(float)NOTE_FREQ[note_f])-1;
		
		//��������
    uint16_t volume=(((float)Autoreload)/100.0)*NOTE_VOL;
	
	  //�����Զ���װ��ֵ
		__HAL_TIM_SET_AUTORELOAD(&htim1,Autoreload);
		
	  //��������
		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,volume);
	
	  //�������ֵ
		__HAL_TIM_SET_COUNTER(&htim1,0);

}


u32 paly_delay_ms=0;
void my_buzzer_play(){
    static u8 last_flg=255;//��¼��һ�׸�
	  //�����л�
	  if(last_flg!=mymusic_t.now_flg){
			mymusic_t.now_len=0;
			my_music_code_get(mymusic_t.now_flg);
			last_flg=mymusic_t.now_flg;
		}
		
		if(mymusic_t.now_flg==0){
			my_passive_buzzer_set(0);
			return;//ֹͣ���治����
		}
		
		if(paly_delay_ms==0){
			  paly_delay_ms=mymusic_t.t_each*mymusic_t.t[mymusic_t.now_len];
				my_passive_buzzer_set(mymusic_t.f[mymusic_t.now_len]);
				
			  if(mymusic_t.now_len>=(mymusic_t.len-1)){
				    mymusic_t.now_len=0;
				}else{
				    mymusic_t.now_len++;
				}
		}else{
		   paly_delay_ms--;
		}
    
}



//��ʱ��1ms�ж�
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
   if(htim->Instance == TIM2){
		 
		 my_buzzer_play();
         
     
     }
}