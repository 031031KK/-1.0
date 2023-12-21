/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include "seg.h"
#include "adc.h"
#include "tim.h"
#include "key.h"
#include "stm32f1xx_hal_tim.h"
#include "usart.h"
#include "beep.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#if 1
#define BEEPON		TIM2->CR1 |= TIM_CR1_CEN;
#define BEEPOFF		TIM2->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
#else
#define BEEPON		__HAL_TIM_ENABLE(&htim2)	//TIM2->CR1 |= TIM_CR1_CEN;
#define BEEPOFF		__HAL_TIM_DISABLE(&htim2)	//TIM2->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define STARTINGANIMATION_TIME 5000	/*开机动画s*/

typedef enum //主模式结构体
{
	MAINMODE_SHUTDOWN, //
	MAINMODE_NOTDIAG,
	MAINMODE_DIAG,
	MAINMODE_MAX
}MAINMODE_TYPE;

typedef enum  //子模式结构体
{
	SUBMODE_NULL,
	SUBMODE_INITIAL,
	SUBMODE_AM,
	SUBMODE_USB,
	SUBMODE_BTA,
	SUBMODE_LED,
	SUBMODE_SEG,
	SUBMODE_TEMP,
	SUBMODE_BEEP,
	SUBMODE_MAX
}SUBMODE_TYPE;

typedef enum //按键结构体
{
	KEY_RESET,
	KEY_SRC_SHORT,
	KEY_SRC_LONG,
	KEY_UP_SHORT,
	KEY_UP_LONG,
	KEY_DOWN_SHORT,
	KEY_DOWN_LONG,
	KEY_SRC_UP_GROUP,
	KEY_MAX
}KEY_TYPE;

typedef enum //数码管结构体
{
	SEG_ALLON,
	SEG_ALLOFF,
	SEG_MAX
}SEG_STATUS;

typedef enum
{
	MUSIC_STATUS_ON,
	MUSIC_STATUS_OFF,
	MUSIC_STATUS_MAX
}MUSIC_STATUS;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* 队列关联信息 */
#define KEYMSG_Q_NUM    1  					//按键消息队列的数量  
QueueHandle_t Key_Queue = NULL;   	//按键值消息队列句柄

/* 事件标志组句柄 */
EventGroupHandle_t EventGroupHandler = NULL;

/* 二值信号量句柄 */
SemaphoreHandle_t BinarySemaphore;

/* 状态迁移表用变量 */
typedef void (*FUNC_POINETR)(MAINMODE_TYPE,SUBMODE_TYPE);

/* 状态管理用变量 */
MAINMODE_TYPE Main_Mode = MAINMODE_SHUTDOWN;
SUBMODE_TYPE Sub_Mode = SUBMODE_NULL;
MAINMODE_TYPE Before_Main_Mode = MAINMODE_SHUTDOWN;
SUBMODE_TYPE Before_Sub_Mode = SUBMODE_NULL;

/* AM的波特率 */
signed char Frequency = 80;

/* ADC温度采集值 */
float AD_TempValueFromISR = 0;
unsigned int AD_TempValue = 0;

unsigned char track_num = 1;
#define TRACK_NUM_MAX 3
MUSIC_STATUS eMusic_Status = MUSIC_STATUS_OFF;

/* 数码管全亮/全灭控制变量 */
SEG_STATUS eSubModeSeg_Status = SEG_ALLOFF;

/* 数码管显示任务 */
#define SEG_TASK_PRIO 8
#define SEG_STK_SIZE  64
void seg_task(void * pvParameters);
TaskHandle_t SegTask_Handle;

/* 按键处理任务 */
#define KEY_TASK_PRIO 6	//6
#define KEY_STK_SIZE  64
void key_task(void * pvParameters);
TaskHandle_t KeyTask_Handle;

/* 状态迁移任务 */
#define STATUS_TASK_PRIO 19
#define STATUS_STK_SIZE  256
void status_task(void * pvParameters);
TaskHandle_t StatusTask_Handle;

/* 开机动画任务 */
#define STARTINGANIMATION_TASK_PRIO 22
#define STARTINGANIMATION_STK_SIZE  64
void startinganimation_task(void * pvParameters);
TaskHandle_t StartingAnimationTask_Handle;

/* 音乐播放任务 */
#define MUSIC_TASK_PRIO 1
#define MUSIC_STK_SIZE  64
void music_task(void * pvParameters);
TaskHandle_t MusicTask_Handle;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void Initial(void);																				/* 初期化函数											*/
void SetKeyCode2EventGroup(unsigned char ucKeyCode);		/* 将按键的押下弹起整理在事件集里 */
void ChangeKey2Evnet(void);																/* 从事件集总结出按键事件					*/

/* 状态迁移函数声明部分 */
void Func_1_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_1_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_1_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_1_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_1_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_1_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_1_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_1_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_1_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_2_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_2_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_2_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_2_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_2_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_2_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_2_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_2_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_2_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_3_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_3_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_3_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_3_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_3_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_3_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_3_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_3_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_3_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_4_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_4_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_4_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_4_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_4_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_4_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_4_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_4_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_4_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_5_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_5_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_5_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_5_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_5_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_5_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_5_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_5_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_5_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_6_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_6_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_6_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_6_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_6_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_6_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_6_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_6_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_6_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_7_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_7_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_7_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_7_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_7_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_7_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_7_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_7_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);
void Func_7_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode);

/* 根据状态迁移表做出来的状态迁移矩阵 */
FUNC_POINETR STATUS_FUNC_MATRIX[KEY_MAX][SUBMODE_MAX]=
{
	{NULL,			NULL,			NULL,			NULL,			NULL,			NULL,			NULL,			NULL,			NULL},
	{Func_1_0,	NULL,			Func_1_2,	Func_1_3,	Func_1_4,	Func_1_5,	Func_1_6,	Func_1_7,	Func_1_8},
	{Func_2_0,	NULL,			Func_2_2,	Func_2_3,	Func_2_4,	NULL,			NULL,			NULL,			NULL},
	{NULL,			NULL,			Func_3_2,	Func_3_3,	NULL,			Func_3_5,	Func_3_6,	NULL,			NULL},
	{NULL,			NULL,			Func_4_2,	NULL,			NULL,			Func_4_5,	Func_4_6,	NULL,			NULL},
	{NULL,			NULL,			Func_5_2,	Func_5_3,	NULL,			Func_5_5,	Func_5_6,	Func_5_7,	Func_5_8},
	{NULL,			NULL,			Func_6_2,	NULL,			NULL,			Func_6_5,	Func_6_6,	Func_6_7,	Func_6_8},
	{NULL,			NULL,			Func_7_2,	NULL,			NULL,			NULL,			NULL,			NULL,			NULL}	
};
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  HAL_TIM_Base_Start_IT(&htim2);			/* NEU_FUNCTION:BEEP */
	HAL_TIM_Base_Start_IT(&htim3);		  /* NEU_FUNCTION:SEG  */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 64);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
	/*进入临界段*/
	taskENTER_CRITICAL();
	/* 创建任务，队列，信号量等 */
	
	/* 创建消息队列Key_Queue */
	Key_Queue=xQueueCreate(KEYMSG_Q_NUM,sizeof(char));
  /* 创建事件标志组 */
	EventGroupHandler=xEventGroupCreate();		
	/* 创建二值号量	*/
	BinarySemaphore=xSemaphoreCreateBinary();

	/* 创建数码管显示任务 */
	xTaskCreate((TaskFunction_t) seg_task,
							(char*         ) "seg_task",		
							(configSTACK_DEPTH_TYPE) SEG_STK_SIZE,
							(void*         ) NULL,
							(UBaseType_t   ) SEG_TASK_PRIO,
							(TaskHandle_t* ) &SegTask_Handle);

	/* 创建按键处理任务 */
	xTaskCreate((TaskFunction_t) key_task,
							(char*         ) "key_task",		
							(configSTACK_DEPTH_TYPE) KEY_STK_SIZE,
							(void*         ) NULL,
							(UBaseType_t   ) KEY_TASK_PRIO,
							(TaskHandle_t* ) &KeyTask_Handle);

	/* 创建状态迁移任务 */
	xTaskCreate((TaskFunction_t) status_task,
							(char*         ) "status_task",		
							(configSTACK_DEPTH_TYPE) STATUS_STK_SIZE,
							(void*         ) NULL,
							(UBaseType_t   ) STATUS_TASK_PRIO,
							(TaskHandle_t* ) &StatusTask_Handle);


	/* 创建开机动画任务 */
	xTaskCreate((TaskFunction_t) startinganimation_task,
							(char*         ) "startinganimation_task",		
							(configSTACK_DEPTH_TYPE) STARTINGANIMATION_STK_SIZE,
							(void*         ) NULL,
							(UBaseType_t   ) STARTINGANIMATION_TASK_PRIO,
							(TaskHandle_t* ) &StartingAnimationTask_Handle);
							
	/* 音乐播放任务 */
	xTaskCreate((TaskFunction_t) music_task,
							(char*         ) "music_task",		
							(configSTACK_DEPTH_TYPE) MUSIC_STK_SIZE,
							(void*         ) NULL,
							(UBaseType_t   ) MUSIC_TASK_PRIO,
							(TaskHandle_t* ) &MusicTask_Handle);
							
	vTaskDelete(defaultTaskHandle);
	/*退出临界段*/
	taskEXIT_CRITICAL();
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* 数码管显示任务 */
void seg_task(void* argument)
{
  /* Infinite loop */
  for(;;)
  {

		if((Main_Mode == MAINMODE_SHUTDOWN)&& (Sub_Mode == SUBMODE_NULL)) //判断主子模式
		{
			LEDDisplayString("####", NO_DOT);						/* #:熄灭 */
		}else if(Main_Mode == MAINMODE_NOTDIAG)
		{
			switch( Sub_Mode )
			{
				case SUBMODE_INITIAL:
					break;
				case SUBMODE_AM:
					LEDDisplay2Half('A',Frequency,3);
					break;
				case SUBMODE_USB:
					LEDDisplay2Half('U',track_num,3);
					break;
				case SUBMODE_BTA:
						LEDDisplayString("B---", 3);
					break;
			}
		}else if(Main_Mode == MAINMODE_DIAG){
			switch(Sub_Mode)
			{
				case SUBMODE_LED:
					LEDDisplayString("1LED", 3);
					ALLOFF;
					break;
				case SUBMODE_SEG:
					if(SEG_ALLON == eSubModeSeg_Status)
					{
						LEDDisplayString("****", ALL_DOT);		/* *:全亮 */
					}else{
						LEDDisplayString("####", NO_DOT);			/* #:熄灭 */
					}
					break;
				case SUBMODE_TEMP:					
					HAL_ADC_Start_IT(&hadc1); // 开中断获取值
					LEDDisplayNumber(3000+AD_TempValue,3);
					break;
				case SUBMODE_BEEP:
					LEDDisplayString("4BPP", 3);
					break;
				default:
					break;					
			}
		}
    osDelay(100);
  }
}

/* 按键处理任务 */
void key_task(void* argument)
{
	unsigned char KeyCode;	/* 按键代码 */
	
	while(1)
	{
		bsp_KeyPro();
		KeyCode = bsp_GetKey();
		
		if (KeyCode > 0)
		{		
			SetKeyCode2EventGroup(KeyCode);  
			ChangeKey2Evnet();
		}

		osDelay(10);
	}
}

static void ChangeKey2Evnet()
{
	EventBits_t EventValue;
	unsigned char value = 0xFF;
	
	if(NULL != EventGroupHandler)
	{
		EventValue = xEventGroupGetBits(EventGroupHandler);
		switch(EventValue)
		{
			case ST2_KEY_SHORT:
				value = KEY_SRC_SHORT;
				xQueueSend(Key_Queue,&value,portMAX_DELAY);
				xEventGroupClearBits(EventGroupHandler,0x0FFF);
				break;
			case ST2_KEY_LONG:
				value = KEY_SRC_LONG;
				xQueueSend(Key_Queue,&value,portMAX_DELAY);
				xEventGroupClearBits(EventGroupHandler,0x0FFF);
				break;
			case ST3_KEY_SHORT:
				value = KEY_UP_SHORT;
				xQueueSend(Key_Queue,&value,portMAX_DELAY);
				xEventGroupClearBits(EventGroupHandler,0x0FFF);
				break;
			case ST3_KEY_LONG:
				value = KEY_UP_LONG;
				xQueueSend(Key_Queue,&value,portMAX_DELAY);
				xEventGroupClearBits(EventGroupHandler,0x0FFF);
				break;
			case ST4_KEY_SHORT:
				value = KEY_DOWN_SHORT;
				xQueueSend(Key_Queue,&value,portMAX_DELAY);
				xEventGroupClearBits(EventGroupHandler,0x0000);
				break;
			case ST4_KEY_LONG:
				value = KEY_DOWN_LONG;
				xQueueSend(Key_Queue,&value,portMAX_DELAY);
				xEventGroupClearBits(EventGroupHandler,0x0000);
				break;
			case ST2_ST3_GROUP:
				value = KEY_SRC_UP_GROUP;
				xQueueSend(Key_Queue,&value,portMAX_DELAY);
				xEventGroupClearBits(EventGroupHandler,0x0000);
				break;
			default:
				break;
		}
	}
}

static void SetKeyCode2EventGroup(unsigned char ucKeyCode)
{
	if(NULL != EventGroupHandler)
	{
		switch (ucKeyCode) {
			case KEY_DOWN_BTN1:
				xEventGroupSetBits(EventGroupHandler,ST2_BIT_SHORT);//SW2键按下
				break;
			case KEY_UP_BTN1:
				xEventGroupSetBits(EventGroupHandler,ST2_BIT_UP);//SW2键弹起
				break;
			case KEY_LONG_BTN1:
				xEventGroupSetBits(EventGroupHandler,ST2_BIT_LONG);//SW2键长按
				break;
			case KEY_DOWN_BTN2:
				xEventGroupSetBits(EventGroupHandler,ST3_BIT_SHORT);//SW3键按下
				break;
			case KEY_UP_BTN2:
				xEventGroupSetBits(EventGroupHandler,ST3_BIT_UP);//SW3键弹起
				break;
			case KEY_LONG_BTN2:
				xEventGroupSetBits(EventGroupHandler,ST3_BIT_LONG);//SW3键长按
				break;
			case KEY_DOWN_BTN3:
				xEventGroupSetBits(EventGroupHandler,ST4_BIT_SHORT);//SW4键按下
				break;
			case KEY_UP_BTN3:
				xEventGroupSetBits(EventGroupHandler,ST4_BIT_UP);//SW4键弹起
				break;
			case KEY_LONG_BTN3:
				xEventGroupSetBits(EventGroupHandler,ST4_BIT_LONG);//SW4键长按
				break;
			case KEY_DOWN_BTN1_BTN2:
				xEventGroupSetBits(EventGroupHandler,ST2_ST3_BIT_GROUP);//组合键，SW2键和SW3键同时按下
				break;
			default:
				break;
		}
	}
}

/* 状态迁移任务 */
void status_task(void* argument)
{
	unsigned char event;
	Initial();	
  /* Infinite loop */
  while(1)
  {
		if(Key_Queue!=NULL)
    {
			if(xQueueReceive(Key_Queue,&event,portMAX_DELAY))
			{
				if((event < KEY_MAX) || (Main_Mode < MAINMODE_MAX) || (Sub_Mode < SUBMODE_MAX))
				{
					if(STATUS_FUNC_MATRIX[event][Sub_Mode] != NULL)
					{
						(*STATUS_FUNC_MATRIX[event][Sub_Mode])(Main_Mode,Sub_Mode);
					}								
				}
			}
		}
		osDelay(50);
  }
}

/* 开机动画任务 */
void startinganimation_task(void* argument)
{
	BaseType_t state; 
	static unsigned char num = 0;
	unsigned int time = 500;					
	unsigned char times	= STARTINGANIMATION_TIME/time;		
	
  while(1)
  {
		if(NULL != BinarySemaphore)
		{
			state = xSemaphoreTake(BinarySemaphore,portMAX_DELAY);
			if(pdTRUE == state  )
			{
				while(1)
				{
					if(times == num)
					{
						break;
					}
					LEDDisplayString("$###", NO_DOT);
					osDelay(time);

					LEDDisplayString("#$##", NO_DOT);
					osDelay(time);

					LEDDisplayString("##$#", NO_DOT);
					osDelay(time);

					LEDDisplayString("###$", NO_DOT);
					osDelay(time);
					num++;

				}
			}
		}
  }
}

/* 音乐播放任务 */
void music_task(void* argument)
{
  while(1)
  {
		if(MUSIC_STATUS_ON == eMusic_Status)
		{
			switch(track_num)
			{
				case 1:
								HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
								BEEPON;
								osDelay(1);
								BEEPOFF;
								osDelay(1);
								HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_1);
								break;
				case 2:	
								HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
								BEEPON;
								osDelay(10);
								BEEPOFF;
								osDelay(10);
								HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_1);
								break;
				case 3:
								HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
								BEEPON;
								osDelay(100);
								BEEPOFF;
								osDelay(100);
								HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_1);
							break;
			}
		}
		osDelay(100);	
  }
}

void Initial()
{

	Main_Mode = MAINMODE_NOTDIAG;
	Sub_Mode = SUBMODE_INITIAL;
	
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);

	if(NULL != BinarySemaphore)
	{
		xSemaphoreGive(BinarySemaphore);//开启动画
	}
	osDelay(5000);
	vTaskDelete(StartingAnimationTask_Handle);
	
	/* 5S后进入AM子模式 */
	Main_Mode = MAINMODE_NOTDIAG;
	Sub_Mode = SUBMODE_AM;
	
	Frequency = 80;
	eMusic_Status = MUSIC_STATUS_OFF;
	AD_TempValue = 0;

}


void SetMainSubMode(MAINMODE_TYPE eMainMode, SUBMODE_TYPE eSubMode)
{
	Main_Mode = eMainMode;
	Sub_Mode = eSubMode;
}

void GetMainSubMode(MAINMODE_TYPE* peMainMode, SUBMODE_TYPE* peSubMode)
{
	*peMainMode = Main_Mode;
	*peSubMode = Sub_Mode;
}

/* 将ADC值转换成温度函数 */
//Rth = R25*exp[B*(1/T - 1/(273.15+25))]
//T = 1/(1/Beta*log(Rth/R25) + 1/(273.15+25))-273.15;
//c语言里面只有两个函数log和log10，
//其中函数 log(x) 表示是以e为底的自然对数，即 ln(x)函数。 
//log10(x) 以10为底的对数，即 lg(x)。
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	uint32_t value = 0;
	float temprature;
	float res;
	
	value = HAL_ADC_GetValue(hadc);
	res = (R1*value)/(uint32_t)(4095-value);	
	temprature = 1/(log(res/NTC_R0)/NTC_B+1/NTC_T0);
	AD_TempValueFromISR = temprature - K;
	
}


/* 状态迁移函数 */

void Func_1_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	//printf("FUNCTION=%s\r\n",__FUNCTION__);
	osDelay(5000);
	
	if((MAINMODE_NOTDIAG == Before_Main_Mode) && (SUBMODE_USB == Before_Sub_Mode))
	{
		eMusic_Status = MUSIC_STATUS_ON;
	}
	SetMainSubMode(Before_Main_Mode,Before_Sub_Mode);
}

void Func_1_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
	
void Func_1_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
	track_num = 1;
	eMusic_Status = MUSIC_STATUS_ON;
	SetMainSubMode(MAINMODE_NOTDIAG,SUBMODE_USB);
	
}
void Func_1_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{

	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
	eMusic_Status = MUSIC_STATUS_OFF;
	SetMainSubMode(MAINMODE_NOTDIAG,SUBMODE_BTA);
	
}
void Func_1_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
	SetMainSubMode(MAINMODE_NOTDIAG,SUBMODE_AM);
}
void Func_1_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
	SetMainSubMode(MAINMODE_DIAG,SUBMODE_SEG);
	eSubModeSeg_Status = SEG_ALLOFF;
}
void Func_1_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
	SetMainSubMode(MAINMODE_DIAG,SUBMODE_TEMP);
}
void Func_1_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
	SetMainSubMode(MAINMODE_DIAG,SUBMODE_BEEP);
}
void Func_1_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
	SetMainSubMode(MAINMODE_DIAG,SUBMODE_LED);
}
void Func_2_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	osDelay(5000);
	if((MAINMODE_NOTDIAG == Before_Main_Mode) && (SUBMODE_USB == Before_Sub_Mode))
	{
		eMusic_Status = MUSIC_STATUS_ON;
	}
	SetMainSubMode(Before_Main_Mode,Before_Sub_Mode);
}
void Func_2_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_2_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	/* 1.数码管全灭：数码管显示任务完成 */
	/* 2.LED灯灭 */
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
	
	Before_Main_Mode = Main_Mode;
	Before_Sub_Mode = Sub_Mode;
	
	SetMainSubMode(MAINMODE_SHUTDOWN,SUBMODE_NULL);
}
void Func_2_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	/* 1.数码管全灭：数码管显示任务完成 */
	/* 2.LED灯灭 */
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
	
	Before_Main_Mode = Main_Mode;
	Before_Sub_Mode = Sub_Mode;
	
	eMusic_Status = MUSIC_STATUS_OFF;
	
	SetMainSubMode(MAINMODE_SHUTDOWN,SUBMODE_NULL);
}
void Func_2_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	/* 1.数码管全灭：数码管显示任务完成 */
	/* 2.LED灯灭 */
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
	
	Before_Main_Mode = Main_Mode;
	Before_Sub_Mode = Sub_Mode;
	
	SetMainSubMode(MAINMODE_SHUTDOWN,SUBMODE_NULL);
	//printf("FUNCTION=%s\r\n",__FUNCTION__);
}
void Func_2_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_2_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_2_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_2_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_3_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_3_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_3_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	Frequency++;
	if(Frequency > 100)
	{
		Frequency = 0;
	}
}
void Func_3_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	if(track_num < TRACK_NUM_MAX)
	{
		track_num++;
	}else{
		track_num = 1;
	}	
}
void Func_3_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_3_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
}
void Func_3_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	eSubModeSeg_Status = SEG_ALLON;
}
void Func_3_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_3_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_4_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_4_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_4_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	Frequency++;
	if(Frequency > 100)
	{
		Frequency = 0;
	}
}
void Func_4_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_4_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_4_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
}
void Func_4_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	eSubModeSeg_Status = SEG_ALLON;
}
void Func_4_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_4_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_5_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_5_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_5_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	Frequency--;
	if(Frequency < 0)
	{
		Frequency = 100;
	}
}
void Func_5_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	if(track_num == 1)
	{
		track_num = 3;
	}else{
		track_num--;
	}	
}
void Func_5_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_5_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
		HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
}
void Func_5_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	eSubModeSeg_Status = SEG_ALLOFF;
}
void Func_5_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	AD_TempValue = AD_TempValueFromISR;
}
void Func_5_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
		eMusic_Status = MUSIC_STATUS_ON;
}
void Func_6_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_6_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_6_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	Frequency--;
	if(Frequency < 0)
	{
		Frequency = 100;
	}
}
void Func_6_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_6_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_6_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
		HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
}
void Func_6_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	eSubModeSeg_Status = SEG_ALLOFF;
}
void Func_6_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	AD_TempValue = AD_TempValueFromISR;
}
void Func_6_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	eMusic_Status = MUSIC_STATUS_OFF;
}
void Func_7_0(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_7_1(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_7_2(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
	SetMainSubMode(MAINMODE_DIAG,SUBMODE_LED);
}
void Func_7_3(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode)
{
	BEEPOFF;
}
void Func_7_4(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_7_5(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_7_6(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_7_7(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
void Func_7_8(MAINMODE_TYPE mainmode,SUBMODE_TYPE submode){}
/* USER CODE END Application */

