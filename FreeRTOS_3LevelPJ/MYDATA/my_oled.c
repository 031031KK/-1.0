#include "my_oled.h"
#include "my_delay.h"

//////////////////////////////////

// IIC 器件写地址
#define IIC_ADDR (0x78)

// IIC 时钟线 SCL
#define IIC_SCL_PORT IIC_SCL_GPIO_Port
#define IIC_SCL_PIN  IIC_SCL_Pin

 
// IIC 数据线 SDA
#define IIC_SDA_PORT IIC_SDA_GPIO_Port
#define IIC_SDA_PIN  IIC_SDA_Pin

/////////////////////////////////



//时钟线电平控制
static void IIC_SCL_Control( u8 c){
	 //输出高电平
   if(c==1) HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_SET);
	 //输出低电平
	 else HAL_GPIO_WritePin(IIC_SCL_PORT, IIC_SCL_PIN, GPIO_PIN_RESET);
}

//数据线电平控制
static void IIC_SDA_Control( u8 c){
	 //输出高电平
   if(c==1) HAL_GPIO_WritePin(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_PIN_SET);
	 //输出低电平
	 else HAL_GPIO_WritePin(IIC_SDA_PORT, IIC_SDA_PIN, GPIO_PIN_RESET);
}

//数据线电平读取
static u8 IIC_SDA_Read(){
	
	 return HAL_GPIO_ReadPin(IIC_SDA_PORT,IIC_SDA_PIN);
	
}

//数据线电平方向设置为输出
static void IIC_SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = IIC_SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IIC_SDA_PORT, &GPIO_InitStruct);
}

//数据线电平方向设置为输入
static void IIC_SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = IIC_SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(IIC_SDA_PORT, &GPIO_InitStruct);
}

//----------------------------------------------IIC基础-------------------------------

static void IIC_Delay(u32 us){
   //delay_us(us);
}

//IIC起始信号
static void IIC_Start(void)
{
	IIC_SDA_OUT(); 
	IIC_SDA_Control(1);	  	  
	IIC_SCL_Control(1);
	IIC_Delay(4);
 	IIC_SDA_Control(0);  
	IIC_Delay(4);
	IIC_SCL_Control(0); 
}	  

//IIC停止信号
static void IIC_Stop(void)
{
	IIC_SDA_OUT();  
	IIC_SCL_Control(0);
	IIC_SDA_Control(0);
 	IIC_Delay(4);
	IIC_SCL_Control(1); 
	IIC_SDA_Control(1);
	IIC_Delay(4);							   	
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
static u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	IIC_SDA_IN();
	IIC_SDA_Control(1);IIC_Delay(1);	   
	IIC_SCL_Control(1);IIC_Delay(1);	 
	while(IIC_SDA_Read())
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL_Control(0); 
	return 0;  
} 

//产生ACK应答
static void IIC_Ack(void)
{
	IIC_SCL_Control(0);
	IIC_SDA_OUT();
	IIC_SDA_Control(0);
	IIC_Delay(2);
	IIC_SCL_Control(1);
	IIC_Delay(2);
	IIC_SCL_Control(0);
}

//不产生ACK应答		    
static void IIC_NAck(void)
{
	IIC_SCL_Control(0);
	IIC_SDA_OUT();
	IIC_SDA_Control(1);
	IIC_Delay(2);
	IIC_SCL_Control(1);
	IIC_Delay(2);
	IIC_SCL_Control(0);
}		

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
static void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	IIC_SDA_OUT(); 	    
    IIC_SCL_Control(0);
    for(t=0;t<8;t++)
    {              
        IIC_SDA_Control((txd&0x80)>>7);
        txd<<=1; 	  
		IIC_Delay(2); 
		IIC_SCL_Control(1);
		IIC_Delay(2); 
		IIC_SCL_Control(0);	
		IIC_Delay(2);
    }	 
} 	

//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
static u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	IIC_SDA_IN();    
    for(i=0;i<8;i++ )
	{
        IIC_SCL_Control(0); 
        IIC_Delay(2);
		    IIC_SCL_Control(1);
        receive<<=1;
        if(IIC_SDA_Read())receive++;   
		IIC_Delay(1); 
    }					 
    if (!ack)
        IIC_NAck();    
    else
        IIC_Ack();  
    return receive;
}

//从指定地址读出一个数据
//ReadAddr:开始读数的地址  
//返回值  :读到的数据
static u8 IIC_Dev_Read_Byte(u8 devaddr,u8 addr)
{				  
	u8 temp=0;		  	    																 
	IIC_Start();  
	IIC_Send_Byte(devaddr);//器件写地址
	IIC_Wait_Ack(); 
	IIC_Send_Byte(addr);//寄存器地址
	IIC_Wait_Ack();	

	IIC_Start();  	 	   
	IIC_Send_Byte(devaddr|1); //读地址		   
	IIC_Wait_Ack();	 
	temp=IIC_Read_Byte(0);			   
	IIC_Stop();  
	return temp;
}

//连续读多个字节
//addr:起始地址
//rbuf:读数据缓存
//len:数据长度
static void IIC_Dev_Read_Bytes(u8 devaddr,u8 addr,u8 len,u8 *rbuf)
{
	int i=0;
	IIC_Start();  
	IIC_Send_Byte(devaddr);  
	IIC_Wait_Ack();	
	IIC_Send_Byte(addr);
	IIC_Wait_Ack();	
	IIC_Start();  	
	IIC_Send_Byte(devaddr|1);  	
	IIC_Wait_Ack();	
	for(i=0; i<len; i++)
	{
		if(i==len-1)
		{
			rbuf[i]=IIC_Read_Byte(0); 
		}
		else
			rbuf[i]=IIC_Read_Byte(1);
	}
	IIC_Stop( );	
}

//从指定地址写入一个数据
//WriteAddr :写入数据的目的地址    
//DataToWrite:要写入的数据
static void IIC_Dev_Write_Byte(u8 devaddr,u8 addr,u8 data)
{				   	  	    																 
	IIC_Start();  
	IIC_Send_Byte(devaddr); 
	IIC_Wait_Ack();	   
	IIC_Send_Byte(addr); 
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(data);			   
	IIC_Wait_Ack();  		    	   
	IIC_Stop();	
}

//连续写多个字节
//addr:起始地址
//wbuf:写数据缓存
//len:数据的长度
static void IIC_Dev_Write_Bytes(u8 devaddr,u8 addr,u8 len,u8 *wbuf)
{
	int i=0;
	IIC_Start();  
	IIC_Send_Byte(devaddr);  	
	IIC_Wait_Ack();	
	IIC_Send_Byte(addr);  //地址自增
	IIC_Wait_Ack();	
	for(i=0; i<len; i++)
	{
		IIC_Send_Byte(wbuf[i]);  
		IIC_Wait_Ack();		
	}
	IIC_Stop( );	
}

//----------------------------------------------应用---------------------------------
//字库声明
const unsigned char F6x8[][6];
const unsigned char F8X16[];
const unsigned char chinese_buf[][32];

//全局变量定义
#define OLED_CMD  0    //写命令
#define OLED_DATA 1    //写数据
#define Max_Column    128
#define Max_Row        64

//OLED写内容分类
void OLED_WR_Byte(unsigned dat,unsigned cmd)
{
    if(cmd)
    {
			//写命令
      IIC_Dev_Write_Byte(IIC_ADDR,0x40,dat);
    }
    else {
			//写数据
      IIC_Dev_Write_Byte(IIC_ADDR,0x00,dat);
    }
}
//坐标设置
void OLED_Set_Pos(unsigned char x, unsigned char y)
{     OLED_WR_Byte(0xb0+y,OLED_CMD);
      OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
      OLED_WR_Byte((x&0x0f),OLED_CMD);
}
//开启OLED显示
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
    OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
    OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//关闭OLED显示
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
    OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
    OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}
//清屏函数,清完屏,整个屏幕是黑色的
void OLED_Clear(void)
{
    u8 i,n;
    for(i=0;i<8;i++)
    {
        OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
        OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
        OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址
        for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA);
    } //更新显示
}
void OLED_On(void)
{
    u8 i,n;
    for(i=0;i<8;i++)
    {
        OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
        OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
        OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址
        for(n=0;n<128;n++)OLED_WR_Byte(1,OLED_DATA);
    } //更新显示
}
//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示
//size:选择字体 16/12
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 Char_Size)
{
			unsigned char c=0,i=0;
			c=chr-' ';//得到偏移后的值
			if(x>Max_Column-1){x=0;y=y+2;}
			if(Char_Size ==16)
			{
				OLED_Set_Pos(x,y);
				for(i=0;i<8;i++)
				OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
				OLED_Set_Pos(x,y+1);
				for(i=0;i<8;i++)
				OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
			}
			else {
				OLED_Set_Pos(x,y);
				for(i=0;i<6;i++)
				OLED_WR_Byte(F6x8[c][i],OLED_DATA);
			}
}
//m^n函数
u32 oled_pow(u8 m,u8 n)
{
    u32 result=1;
    while(n--)result*=m;
    return result;
}

//显示一个字符号串
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 Char_Size)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{        
		OLED_ShowChar(x,y,chr[j],Char_Size);
		x+=8;
		if(x>120){x=0;y+=2;}
		j++;
	}
}
void OLED_show_china(unsigned char x,unsigned char y,unsigned char datax)
{ 
		 unsigned char t,adder=0;
		 OLED_Set_Pos(x,y);    
		 for(t=0;t<16;t++)//每次只能写一行，所以纵为16需写两行
		 {
			 OLED_WR_Byte(chinese_buf[datax][t],1);
			 adder+=1;
		}    
		OLED_Set_Pos(x,y+1);    
		for(t=16;t<32;t++)
		 {    
			OLED_WR_Byte(chinese_buf[datax][t],1);
			adder+=1;
		}    
}

/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,const unsigned char BMP[])
{
 unsigned int j=0;
 unsigned char x,y;

  if(y1%8==0) y=y1/8;
  else y=y1/8+1;
    for(y=y0;y<y1;y++)
    {
        OLED_Set_Pos(x0,y);
    for(x=x0;x<x1;x++)
        {
            OLED_WR_Byte(BMP[j++],OLED_DATA);
        }
    }
}

/*
显示一个正矩形

sx:起始的x位置，区间为[0,127]
sy:起始的y位置，区间为[0,7]
ex:结束的x位置，区间为[0,127]
ey:结束的y位置，区间为[0,7]

*/
void my_oled_square(uint8_t sx,uint8_t sy,uint8_t ex,uint8_t ey)
{      	
	uint16_t i=0,j;
	for(i=sy;i<ey;i++)
	{
		OLED_Set_Pos(sx,i);
		for(j=sx;j<ex;j++){
		   OLED_WR_Byte(0xFF,OLED_DATA);//6X8字号
		}
	}
}

/*
显示一个正矩形,占据一行

sx:起始的x位置，区间为[0,127]
sy:起始的y位置，区间为[0,7]
ex:结束的x位置，区间为[0,127]
ey:结束的y位置，区间为[0,7]

*/
void my_oled_square_oneRow(uint8_t sx,uint8_t sy,uint8_t ex,uint8_t ey)
{      	
	uint16_t i=0,j;
	for(i=sy;i<ey;i++)
	{
		OLED_Set_Pos(sx,i);
		for(j=sx;j<ex;j++){
		   OLED_WR_Byte(0xFF,OLED_DATA);//6X8字号
		}
		for(j=ex;j<128;j++){
		   OLED_WR_Byte(0x00,OLED_DATA);//6X8字号
		}
	}
}

void my_oled_show_progress(u32 max,u32 now){
	  u8 barvalue=0;
	  if(now>=max) barvalue=128;
	  
	  else barvalue=now*(128.0/max);
	
    my_oled_square_oneRow(0,5,barvalue,7);
}


//表情包的数量

#define MEME_NUM 4


void OLED_meme_animation(){
	 static u8 activenum=0;
	 static u8 memeflg=0;
	
   OLED_DrawBMP(activenum,2,48+activenum,8,meme1[memeflg]);
	
	 if(activenum<80) activenum++;
	 else{
	    activenum=0;
		  if(memeflg<MEME_NUM-1)memeflg++;
		  else memeflg=0;
	 }
	
}	 


//初始化SSD1306
void OLED_Init(void)
{

    OLED_WR_Byte(0xAE,OLED_CMD);//--display off
    OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40,OLED_CMD);//--set start line address

    OLED_WR_Byte(0xB0,OLED_CMD);//--set page address

    OLED_WR_Byte(0x81,OLED_CMD); // contract control
    OLED_WR_Byte(0xFF,OLED_CMD);//--128
	
    OLED_WR_Byte(0xA1,OLED_CMD);//set segment remap


    OLED_WR_Byte(0xA6,OLED_CMD);//--normal / reverse
    OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3F,OLED_CMD);//--1/32 duty

    OLED_WR_Byte(0xC8,OLED_CMD);//Com scan direction
    OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset
    OLED_WR_Byte(0x00,OLED_CMD);//

    OLED_WR_Byte(0xD5,OLED_CMD);//set osc division
    OLED_WR_Byte(0x80,OLED_CMD);//

    OLED_WR_Byte(0xD8,OLED_CMD);//set area color mode off
    OLED_WR_Byte(0x05,OLED_CMD);//

    OLED_WR_Byte(0xD9,OLED_CMD);//Set Pre-Charge Period
    OLED_WR_Byte(0xF1,OLED_CMD);//

    OLED_WR_Byte(0xDA,OLED_CMD);//set com pin configuartion
    OLED_WR_Byte(0x12,OLED_CMD);//

    OLED_WR_Byte(0xDB,OLED_CMD);//set Vcomh
    OLED_WR_Byte(0x30,OLED_CMD);//

    OLED_WR_Byte(0x8D,OLED_CMD);//set charge pump enable
    OLED_WR_Byte(0x14,OLED_CMD);//

    OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel
}


//////////////////////////////////////字库////////////////////////////////////////////////\


const unsigned char meme1[][8*36] =                  // 数据表
{
//----眼镜----
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x80,0xC0,0xC0,0xE0,0x60,0x70,
      0x30,0x30,0x38,0x18,0x18,0x18,0x18,0x18,
      0x18,0x18,0x18,0x18,0x30,0x30,0x30,0x60,
      0x60,0xE0,0xC0,0x80,0x80,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x80,0xE0,0x70,0x3C,0x0E,
      0x07,0x03,0x01,0x01,0x80,0x80,0x80,0x80,
      0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,
      0x80,0x80,0x80,0x01,0x03,0x07,0x0E,0x1C,
      0x38,0xF0,0xC0,0x80,0x00,0x00,0x00,0x00,
      0x00,0xF8,0xFE,0x07,0x01,0x00,0x00,0x00,
      0x18,0xFC,0xFE,0x07,0x03,0x31,0x79,0x79,
      0x31,0x01,0x03,0xCF,0xFE,0x7C,0x1C,0x0C,
      0x1C,0x38,0xFC,0xFE,0x07,0x03,0x31,0x79,
      0x79,0x31,0x01,0x03,0xCF,0xFE,0x78,0x18,
      0x00,0x00,0x03,0x1F,0xFC,0xE0,0x00,0x00,
      0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x03,0x07,0x06,0x8E,0x8C,0x8C,
      0x8C,0x86,0x87,0x83,0x81,0x80,0x80,0x80,
      0x80,0x80,0x80,0x83,0x87,0x86,0x8E,0x8C,
      0x8C,0x8C,0x86,0x07,0x03,0x01,0x00,0x00,
      0x00,0x00,0x00,0xC0,0xFF,0x7F,0x00,0x00,
      0x00,0x01,0x07,0x1F,0x78,0xF0,0xC0,0x80,
      0x00,0x00,0x00,0x00,0x00,0x07,0x1F,0x3F,
      0x7F,0xFF,0xCF,0x87,0x83,0x83,0x01,0x01,
      0x01,0x01,0x83,0x83,0xC7,0xFF,0x7F,0x7F,
      0x1F,0x0F,0x03,0x00,0x00,0x00,0x00,0x80,
      0xE0,0x78,0x3E,0x0F,0x03,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,
      0x07,0x0E,0x1C,0x18,0x38,0x30,0x70,0x60,
      0x60,0xC0,0xC0,0xC1,0xC1,0xC1,0x81,0x83,
      0xC3,0xC1,0xC1,0xC1,0xC0,0xC0,0x60,0x60,
      0x30,0x30,0x38,0x1C,0x0C,0x06,0x07,0x03,
      0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			
//----笑脸-----
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x80,0x80,0xC0,0xE0,0x60,
      0x60,0x30,0x30,0x30,0x38,0x18,0x18,0x18,
      0x18,0x18,0x18,0x18,0x30,0x30,0x30,0x60,
      0x60,0xE0,0xC0,0xC0,0x80,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x80,0xE0,0x70,0x38,
      0x1C,0x06,0x03,0x03,0x01,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x01,0x01,0x03,0x06,0x0E,
      0x3C,0x70,0xE0,0x80,0x00,0x00,0x00,0x00,
      0x00,0x00,0xF0,0xFE,0x0F,0x01,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x1E,
      0x9E,0x9C,0x80,0x80,0x80,0x80,0x80,0x80,
      0x80,0x80,0x80,0x80,0x80,0x80,0x8C,0x9E,
      0x9E,0x8C,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x01,0x0F,0xFE,0xF8,0x00,0x00,
      0x00,0x00,0x7F,0xFF,0x80,0x00,0x00,0x00,
      0x1E,0xFF,0xE7,0xC3,0xC3,0xC3,0xE1,0xE1,
      0xE1,0xE1,0xE1,0xE1,0xE1,0xE1,0xE1,0xE1,
      0xE1,0xE1,0xE1,0xE1,0xE1,0xE1,0xE1,0xE1,
      0xE1,0xE1,0xE1,0xC3,0xC3,0xC3,0xE3,0xFF,
      0x3E,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
      0x00,0x00,0x00,0x03,0x0F,0x3C,0x70,0xE0,
      0x80,0x00,0x03,0x0F,0x1F,0x3F,0x7F,0xFF,
      0xFF,0xFF,0x9F,0x0F,0x07,0x07,0x03,0x03,
      0x03,0x03,0x03,0x03,0x07,0x0F,0x9F,0xFF,
      0xFF,0xFF,0x7F,0x3F,0x1F,0x0F,0x07,0x81,
      0xE0,0xF0,0x3C,0x1F,0x07,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
      0x03,0x03,0x06,0x0E,0x1C,0x18,0x38,0x30,
      0x70,0x61,0x61,0x63,0xC3,0xC3,0xC3,0xC3,
      0xC2,0xC3,0xC3,0xC3,0x63,0x63,0x61,0x71,
      0x30,0x30,0x18,0x1C,0x0C,0x06,0x07,0x03,
      0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			
//----眨眼---
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x80,0xC0,0x60,0x60,0x30,0x10,
      0x18,0x18,0x08,0x08,0x0C,0x0C,0x0C,0x04,
      0x04,0x0C,0x0C,0x0C,0x0C,0x08,0x18,0x18,
      0x18,0x30,0x30,0x60,0xC0,0xC0,0x80,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0xC0,0x70,0x38,0x0C,
      0x07,0x03,0x01,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,
      0x06,0x1C,0x38,0xE0,0x80,0x00,0x00,0x00,
      0x00,0x00,0xF8,0xFF,0x03,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,
      0x18,0xB0,0xF0,0xE0,0x60,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x40,0x60,0xE0,0xF0,
      0x10,0x18,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x07,0xFE,0xC0,0x00,
      0x00,0x00,0x1F,0xFF,0xC0,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x03,0x01,0x00,0x80,0x00,0x00,0x00,0x00,
      0x80,0x80,0x00,0x00,0x00,0x80,0x80,0x01,
      0x01,0x03,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0xE0,0x7F,0x01,0x00,
      0x00,0x00,0x00,0x00,0x07,0x0E,0x38,0x70,
      0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x03,0x07,0x06,0x06,0x06,
      0x03,0x03,0x06,0x06,0x06,0x03,0x01,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,
      0xE0,0x70,0x1C,0x0F,0x03,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x01,0x03,0x06,0x06,0x0C,0x08,0x18,
      0x18,0x30,0x30,0x30,0x20,0x20,0x60,0x60,
      0x60,0x60,0x20,0x20,0x20,0x30,0x30,0x10,
      0x18,0x18,0x0C,0x0C,0x06,0x03,0x03,0x01,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			
//----音乐----
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,
      0x80,0x80,0x80,0x80,0xC0,0x80,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x80,0xC0,0xE0,0x60,0x30,0x30,0x18,0x18,
      0x18,0x08,0x08,0x0C,0x0C,0x08,0x08,0x08,
      0x18,0x18,0x18,0x30,0x30,0x60,0x60,0xC0,
      0x80,0x00,0x00,0x60,0x70,0x78,0x3F,0x07,
      0x05,0x70,0x70,0x7D,0x1F,0x03,0x00,0x00,
      0x00,0x00,0x80,0xE0,0x78,0x1C,0x0E,0x03,
      0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x03,0x07,0x0E,0x3C,0xF0,0xC0,0x00,0x00,
      0x80,0xC0,0xC0,0xF0,0x3C,0x1C,0x78,0x30,
      0x00,0x00,0xFF,0x87,0x00,0x00,0x00,0x00,
      0x00,0x00,0x08,0x0C,0x06,0x06,0x06,0x06,
      0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x0C,0x0E,0x06,0x06,0x06,0x0E,0x0C,0x00,
      0x00,0x00,0x00,0x00,0x00,0xFF,0xFE,0x00,
      0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x07,0x1F,0x78,0xE0,0x80,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x78,
      0xFC,0xBE,0x1E,0x1E,0x0F,0x1E,0x1E,0xBE,
      0xFC,0x78,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0xC0,0xF0,0x3C,0x1F,0x01,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x07,
      0x06,0x0C,0x18,0x10,0x30,0x60,0x60,0x40,
      0x40,0xC1,0xC3,0xC3,0xC2,0xC3,0xC3,0xC1,
      0x40,0x60,0x60,0x60,0x30,0x30,0x18,0x0C,
      0x06,0x03,0x01,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

const unsigned char chinese_buf[][32] =          // 数据表
{
/*"主", */0x00,0x08,0x08,0x08,0x08,0x08,0x09,0xFE,
      0x08,0x08,0x08,0x88,0x0C,0x08,0x00,0x00,
      0x40,0x40,0x41,0x41,0x41,0x41,0x41,0x7F,
      0x41,0x41,0x41,0x41,0x41,0x60,0x40,0x00,

/*"页", */0x02,0x02,0x02,0xF2,0x12,0x1A,0x16,0xD2,
      0x12,0x12,0x12,0xFA,0x12,0x03,0x02,0x00,
      0x00,0x00,0x80,0x9F,0x40,0x20,0x10,0x0F,
      0x10,0x20,0x20,0x5F,0xC0,0x00,0x00,0x00,

/*"两", */0x02,0xF2,0x12,0x12,0x12,0xFE,0x12,0x12,
      0x12,0xFE,0x12,0x12,0x12,0xFB,0x12,0x00,
      0x00,0xFF,0x10,0x08,0x04,0x03,0x14,0x08,
      0x04,0x03,0x04,0x48,0x98,0x7F,0x00,0x00,

/*"只", */0x00,0x00,0x00,0xFC,0x04,0x04,0x04,0x04,
      0x04,0x04,0x04,0xFE,0x04,0x00,0x00,0x00,
      0x00,0x80,0x40,0x23,0x11,0x19,0x01,0x01,
      0x01,0x09,0x11,0x23,0x60,0xC0,0x00,0x00,

/*"老", */0x40,0x40,0x48,0x48,0x48,0x48,0x7F,0xC8,
      0x4C,0x68,0x50,0x48,0x4C,0x60,0x40,0x00,
      0x40,0x20,0x10,0x08,0x04,0x7E,0x91,0x90,
      0x88,0x88,0x84,0x86,0x80,0xE0,0x00,0x00,

/*"虎", */0x00,0x00,0xF8,0x48,0x48,0x48,0xF8,0x4F,
      0x4A,0x2A,0x2B,0x0A,0xA8,0x18,0x00,0x00,
      0x80,0x40,0x3F,0x80,0x40,0x3C,0x04,0x05,
      0x05,0x7F,0x85,0x81,0x81,0xE0,0x00,0x00,

/*"春", */0x20,0x22,0x2A,0x2A,0xAA,0x6A,0x3A,0x2F,
      0x2A,0x6A,0xAA,0x2A,0x2A,0x22,0x20,0x00,
      0x04,0x04,0x02,0x01,0xFF,0x49,0x49,0x49,
      0x49,0x49,0xFF,0x01,0x02,0x06,0x02,0x00,

/*"节", */0x04,0x24,0x24,0x24,0x24,0x3F,0xE4,0x24,
      0x24,0x3F,0x24,0x24,0xF4,0x26,0x04,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,
      0x00,0x04,0x08,0x10,0x0F,0x00,0x00,0x00,

/*"序", */0x00,0x00,0xFC,0x04,0x14,0x14,0x14,0x55,
      0x96,0x54,0x34,0x14,0x16,0x04,0x00,0x00,
      0x40,0x30,0x0F,0x00,0x01,0x01,0x41,0x81,
      0x7F,0x01,0x01,0x05,0x03,0x01,0x00,0x00,

/*"曲", */0x00,0xF0,0x10,0x10,0x10,0xFF,0x10,0x10,
      0x10,0xFF,0x10,0x10,0x10,0xF8,0x10,0x00,
      0x00,0xFF,0x42,0x42,0x42,0x7F,0x42,0x42,
      0x42,0x7F,0x42,0x42,0x42,0xFF,0x00,0x00,

/*"难", */0x04,0x34,0xC4,0x04,0xC4,0x3E,0x44,0x20,
      0xF8,0x4F,0x49,0xFA,0x48,0x4C,0x08,0x00,
      0x20,0x10,0x0C,0x03,0x04,0x18,0x00,0x00,
      0xFF,0x22,0x22,0x3F,0x22,0x32,0x20,0x00,

/*"忘", */0x04,0x04,0x04,0xFC,0x84,0x84,0x85,0x86,
      0x84,0x84,0x84,0xC4,0x84,0x06,0x04,0x00,
      0x20,0x18,0x00,0x3C,0x40,0x40,0x42,0x44,
      0x58,0x40,0x40,0x70,0x00,0x08,0x30,0x00,

/*"今", */0x80,0x80,0x40,0x20,0x10,0x08,0x24,0x43,
      0xC4,0x08,0x10,0x20,0x40,0xC0,0x40,0x00,
      0x00,0x00,0x00,0x02,0x02,0x02,0x02,0x82,
      0x42,0x22,0x1A,0x06,0x00,0x00,0x00,0x00,

/*"宵", */0x10,0x0C,0x04,0xC4,0x54,0x64,0x45,0x7E,
      0x44,0x64,0x54,0xE4,0x44,0x14,0x0C,0x00,
      0x00,0x00,0x00,0xFF,0x12,0x12,0x12,0x12,
      0x12,0x52,0x92,0x7F,0x00,0x00,0x00,0x00
};

const unsigned char Start_BMP[] =                  // 数据表
{
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x10,0x10,0x10,0xF0,0x78,0x3C,0x94,
      0x10,0x10,0x10,0x30,0x10,0x00,0x00,0x00,
      0x00,0x00,0x00,0x80,0xC0,0x00,0x00,0xF8,
      0x00,0x00,0xC0,0x80,0x00,0x00,0x00,0x00,
      0x00,0x00,0x10,0x10,0x10,0xF0,0x78,0x3C,
      0x94,0x10,0x10,0x10,0x30,0x10,0x00,0x00,
      0x00,0x00,0xF8,0xF8,0xC0,0xC0,0xC0,0xC0,
      0x80,0x00,0x00,0x00,0x00,0x00,0xD8,0xD8,
      0x00,0x00,0x00,0x00,0x00,0xF8,0xF8,0x00,
      0x00,0x00,0x00,0x00,0xD8,0xD8,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x40,0x62,0x37,0x1B,0x42,0xC2,0xFF,
      0x02,0x0A,0x1A,0x72,0x20,0x00,0x00,0x00,
      0x00,0x00,0x0C,0x07,0x01,0x60,0x60,0x7F,
      0x00,0x00,0x01,0x07,0x0E,0x00,0x00,0x00,
      0x00,0x00,0x40,0x62,0x37,0x1B,0x42,0xC2,
      0xFF,0x02,0x0A,0x1A,0x72,0x20,0x00,0x00,
      0x00,0x00,0x3F,0x3F,0x31,0x20,0x30,0x3F,
      0x1F,0x00,0x00,0x00,0x00,0x00,0x3F,0x3F,
      0x00,0x00,0x00,0x00,0x00,0x3F,0x3F,0x00,
      0x00,0x00,0x00,0x00,0x3F,0x3F,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x10,0x78,0xF8,
      0xF0,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,
      0xE0,0xF0,0x78,0x38,0x10,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0xE0,0xF0,0xF8,0xFC,0xFC,0x7C,
      0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,
      0x3D,0x3F,0x3F,0x3F,0x3F,0x3E,0x3C,0x3C,
      0x3C,0x3C,0x3C,0x3C,0x3E,0x3F,0x3F,0x3F,
      0x3F,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,
      0x3C,0x7C,0xFC,0xFC,0xFC,0xF8,0xF0,0xC0,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
      0x00,0x00,0x80,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xE0,0xE0,0xC0,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0xE0,
      0xE0,0xE0,0xC0,0xC0,0xC0,0xC0,0x80,0x80,
      0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
      0x00,0x00,0x03,0x03,0x03,0x03,0x03,0x01,
      0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,
      0x80,0x00,0x00,0x00,0x00,0x00,0x01,0x01,
      0x01,0x01,0x01,0x03,0x03,0x03,0x03,0x01,
      0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0x80,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x03,0x07,0x06,0x06,0x07,
      0x03,0x07,0x06,0x06,0x06,0x07,0x02,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x80,0xFF,0xFF,0xFF,0xFF,0xFF,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x01,0x07,0x0F,0x0F,0x1F,0x1F,
      0x1F,0x1F,0x1F,0x7F,0x7F,0x7F,0x7F,0x3F,
      0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,
      0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,
      0x1F,0x1F,0x3F,0x7F,0x7F,0x7F,0x3F,0x1F,
      0x1F,0x1F,0x1F,0x0F,0x0F,0x07,0x03,0x01,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
/************************************6*8的点阵************************************/

const unsigned char F6x8[][6] = 
{

0x00, 0x00, 0x00, 0x00, 0x00, 0x00,// sp

0x00, 0x00, 0x00, 0x2f, 0x00, 0x00,// !

0x00, 0x00, 0x07, 0x00, 0x07, 0x00,// "

0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14,// #

0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12,// $

0x00, 0x62, 0x64, 0x08, 0x13, 0x23,// %

0x00, 0x36, 0x49, 0x55, 0x22, 0x50,// &

0x00, 0x00, 0x05, 0x03, 0x00, 0x00,// '

0x00, 0x00, 0x1c, 0x22, 0x41, 0x00,// (

0x00, 0x00, 0x41, 0x22, 0x1c, 0x00,// )

0x00, 0x14, 0x08, 0x3E, 0x08, 0x14,// *

0x00, 0x08, 0x08, 0x3E, 0x08, 0x08,// +

0x00, 0x00, 0x00, 0xA0, 0x60, 0x00,// ,

0x00, 0x08, 0x08, 0x08, 0x08, 0x08,// -

0x00, 0x00, 0x60, 0x60, 0x00, 0x00,// .

0x00, 0x20, 0x10, 0x08, 0x04, 0x02,// /

0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E,// 0

0x00, 0x00, 0x42, 0x7F, 0x40, 0x00,// 1

0x00, 0x42, 0x61, 0x51, 0x49, 0x46,// 2

0x00, 0x21, 0x41, 0x45, 0x4B, 0x31,// 3

0x00, 0x18, 0x14, 0x12, 0x7F, 0x10,// 4

0x00, 0x27, 0x45, 0x45, 0x45, 0x39,// 5

0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30,// 6

0x00, 0x01, 0x71, 0x09, 0x05, 0x03,// 7

0x00, 0x36, 0x49, 0x49, 0x49, 0x36,// 8

0x00, 0x06, 0x49, 0x49, 0x29, 0x1E,// 9

0x00, 0x00, 0x36, 0x36, 0x00, 0x00,// :

0x00, 0x00, 0x56, 0x36, 0x00, 0x00,// ;

0x00, 0x08, 0x14, 0x22, 0x41, 0x00,// <

0x00, 0x14, 0x14, 0x14, 0x14, 0x14,// =

0x00, 0x00, 0x41, 0x22, 0x14, 0x08,// >

0x00, 0x02, 0x01, 0x51, 0x09, 0x06,// ?

0x00, 0x32, 0x49, 0x59, 0x51, 0x3E,// @

0x00, 0x7C, 0x12, 0x11, 0x12, 0x7C,// A

0x00, 0x7F, 0x49, 0x49, 0x49, 0x36,// B

0x00, 0x3E, 0x41, 0x41, 0x41, 0x22,// C

0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C,// D

0x00, 0x7F, 0x49, 0x49, 0x49, 0x41,// E

0x00, 0x7F, 0x09, 0x09, 0x09, 0x01,// F

0x00, 0x3E, 0x41, 0x49, 0x49, 0x7A,// G

0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F,// H

0x00, 0x00, 0x41, 0x7F, 0x41, 0x00,// I

0x00, 0x20, 0x40, 0x41, 0x3F, 0x01,// J

0x00, 0x7F, 0x08, 0x14, 0x22, 0x41,// K

0x00, 0x7F, 0x40, 0x40, 0x40, 0x40,// L

0x00, 0x7F, 0x02, 0x0C, 0x02, 0x7F,// M

0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F,// N

0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E,// O

0x00, 0x7F, 0x09, 0x09, 0x09, 0x06,// P

0x00, 0x3E, 0x41, 0x51, 0x21, 0x5E,// Q

0x00, 0x7F, 0x09, 0x19, 0x29, 0x46,// R

0x00, 0x46, 0x49, 0x49, 0x49, 0x31,// S

0x00, 0x01, 0x01, 0x7F, 0x01, 0x01,// T

0x00, 0x3F, 0x40, 0x40, 0x40, 0x3F,// U

0x00, 0x1F, 0x20, 0x40, 0x20, 0x1F,// V

0x00, 0x3F, 0x40, 0x38, 0x40, 0x3F,// W

0x00, 0x63, 0x14, 0x08, 0x14, 0x63,// X

0x00, 0x07, 0x08, 0x70, 0x08, 0x07,// Y

0x00, 0x61, 0x51, 0x49, 0x45, 0x43,// Z

0x00, 0x00, 0x7F, 0x41, 0x41, 0x00,// [

0x00, 0x55, 0x2A, 0x55, 0x2A, 0x55,// 55

0x00, 0x00, 0x41, 0x41, 0x7F, 0x00,// ]

0x00, 0x04, 0x02, 0x01, 0x02, 0x04,// ^

0x00, 0x40, 0x40, 0x40, 0x40, 0x40,// _

0x00, 0x00, 0x01, 0x02, 0x04, 0x00,// '

0x00, 0x20, 0x54, 0x54, 0x54, 0x78,// a

0x00, 0x7F, 0x48, 0x44, 0x44, 0x38,// b

0x00, 0x38, 0x44, 0x44, 0x44, 0x20,// c

0x00, 0x38, 0x44, 0x44, 0x48, 0x7F,// d

0x00, 0x38, 0x54, 0x54, 0x54, 0x18,// e

0x00, 0x08, 0x7E, 0x09, 0x01, 0x02,// f

0x00, 0x18, 0xA4, 0xA4, 0xA4, 0x7C,// g

0x00, 0x7F, 0x08, 0x04, 0x04, 0x78,// h

0x00, 0x00, 0x44, 0x7D, 0x40, 0x00,// i

0x00, 0x40, 0x80, 0x84, 0x7D, 0x00,// j

0x00, 0x7F, 0x10, 0x28, 0x44, 0x00,// k

0x00, 0x00, 0x41, 0x7F, 0x40, 0x00,// l

0x00, 0x7C, 0x04, 0x18, 0x04, 0x78,// m

0x00, 0x7C, 0x08, 0x04, 0x04, 0x78,// n

0x00, 0x38, 0x44, 0x44, 0x44, 0x38,// o

0x00, 0xFC, 0x24, 0x24, 0x24, 0x18,// p

0x00, 0x18, 0x24, 0x24, 0x18, 0xFC,// q

0x00, 0x7C, 0x08, 0x04, 0x04, 0x08,// r

0x00, 0x48, 0x54, 0x54, 0x54, 0x20,// s

0x00, 0x04, 0x3F, 0x44, 0x40, 0x20,// t

0x00, 0x3C, 0x40, 0x40, 0x20, 0x7C,// u

0x00, 0x1C, 0x20, 0x40, 0x20, 0x1C,// v

0x00, 0x3C, 0x40, 0x30, 0x40, 0x3C,// w

0x00, 0x44, 0x28, 0x10, 0x28, 0x44,// x

0x00, 0x1C, 0xA0, 0xA0, 0xA0, 0x7C,// y

0x00, 0x44, 0x64, 0x54, 0x4C, 0x44,// z

0x14, 0x14, 0x14, 0x14, 0x14, 0x14,// horiz lines

};

/****************************************8*16的点阵************************************/

const unsigned char F8X16[]= 

{

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,// 0

0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x33,0x30,0x00,0x00,0x00,//! 1

0x00,0x10,0x0C,0x06,0x10,0x0C,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//" 2

0x40,0xC0,0x78,0x40,0xC0,0x78,0x40,0x00,0x04,0x3F,0x04,0x04,0x3F,0x04,0x04,0x00,//# 3

0x00,0x70,0x88,0xFC,0x08,0x30,0x00,0x00,0x00,0x18,0x20,0xFF,0x21,0x1E,0x00,0x00,//$ 4

0xF0,0x08,0xF0,0x00,0xE0,0x18,0x00,0x00,0x00,0x21,0x1C,0x03,0x1E,0x21,0x1E,0x00,//% 5

0x00,0xF0,0x08,0x88,0x70,0x00,0x00,0x00,0x1E,0x21,0x23,0x24,0x19,0x27,0x21,0x10,//& 6

0x10,0x16,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//' 7

0x00,0x00,0x00,0xE0,0x18,0x04,0x02,0x00,0x00,0x00,0x00,0x07,0x18,0x20,0x40,0x00,//( 8

0x00,0x02,0x04,0x18,0xE0,0x00,0x00,0x00,0x00,0x40,0x20,0x18,0x07,0x00,0x00,0x00,//) 9

0x40,0x40,0x80,0xF0,0x80,0x40,0x40,0x00,0x02,0x02,0x01,0x0F,0x01,0x02,0x02,0x00,//* 10

0x00,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x1F,0x01,0x01,0x01,0x00,//+ 11

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xB0,0x70,0x00,0x00,0x00,0x00,0x00,//, 12

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,//- 13

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,0x00,0x00,//. 14

0x00,0x00,0x00,0x00,0x80,0x60,0x18,0x04,0x00,0x60,0x18,0x06,0x01,0x00,0x00,0x00,/// 15

0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x0F,0x10,0x20,0x20,0x10,0x0F,0x00,//0 16

0x00,0x10,0x10,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,//1 17

0x00,0x70,0x08,0x08,0x08,0x88,0x70,0x00,0x00,0x30,0x28,0x24,0x22,0x21,0x30,0x00,//2 18

0x00,0x30,0x08,0x88,0x88,0x48,0x30,0x00,0x00,0x18,0x20,0x20,0x20,0x11,0x0E,0x00,//3 19

0x00,0x00,0xC0,0x20,0x10,0xF8,0x00,0x00,0x00,0x07,0x04,0x24,0x24,0x3F,0x24,0x00,//4 20

0x00,0xF8,0x08,0x88,0x88,0x08,0x08,0x00,0x00,0x19,0x21,0x20,0x20,0x11,0x0E,0x00,//5 21

0x00,0xE0,0x10,0x88,0x88,0x18,0x00,0x00,0x00,0x0F,0x11,0x20,0x20,0x11,0x0E,0x00,//6 22

0x00,0x38,0x08,0x08,0xC8,0x38,0x08,0x00,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00,//7 23

0x00,0x70,0x88,0x08,0x08,0x88,0x70,0x00,0x00,0x1C,0x22,0x21,0x21,0x22,0x1C,0x00,//8 24

0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x00,0x31,0x22,0x22,0x11,0x0F,0x00,//9 25

0x00,0x00,0x00,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,//: 26

0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x60,0x00,0x00,0x00,0x00,//; 27

0x00,0x00,0x80,0x40,0x20,0x10,0x08,0x00,0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x00,//< 28

0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00,//= 29

0x00,0x08,0x10,0x20,0x40,0x80,0x00,0x00,0x00,0x20,0x10,0x08,0x04,0x02,0x01,0x00,//> 30

0x00,0x70,0x48,0x08,0x08,0x08,0xF0,0x00,0x00,0x00,0x00,0x30,0x36,0x01,0x00,0x00,//? 31

0xC0,0x30,0xC8,0x28,0xE8,0x10,0xE0,0x00,0x07,0x18,0x27,0x24,0x23,0x14,0x0B,0x00,//@ 32

0x00,0x00,0xC0,0x38,0xE0,0x00,0x00,0x00,0x20,0x3C,0x23,0x02,0x02,0x27,0x38,0x20,//A 33

0x08,0xF8,0x88,0x88,0x88,0x70,0x00,0x00,0x20,0x3F,0x20,0x20,0x20,0x11,0x0E,0x00,//B 34

0xC0,0x30,0x08,0x08,0x08,0x08,0x38,0x00,0x07,0x18,0x20,0x20,0x20,0x10,0x08,0x00,//C 35

0x08,0xF8,0x08,0x08,0x08,0x10,0xE0,0x00,0x20,0x3F,0x20,0x20,0x20,0x10,0x0F,0x00,//D 36

0x08,0xF8,0x88,0x88,0xE8,0x08,0x10,0x00,0x20,0x3F,0x20,0x20,0x23,0x20,0x18,0x00,//E 37

0x08,0xF8,0x88,0x88,0xE8,0x08,0x10,0x00,0x20,0x3F,0x20,0x00,0x03,0x00,0x00,0x00,//F 38

0xC0,0x30,0x08,0x08,0x08,0x38,0x00,0x00,0x07,0x18,0x20,0x20,0x22,0x1E,0x02,0x00,//G 39

0x08,0xF8,0x08,0x00,0x00,0x08,0xF8,0x08,0x20,0x3F,0x21,0x01,0x01,0x21,0x3F,0x20,//H 40

0x00,0x08,0x08,0xF8,0x08,0x08,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,//I 41

0x00,0x00,0x08,0x08,0xF8,0x08,0x08,0x00,0xC0,0x80,0x80,0x80,0x7F,0x00,0x00,0x00,//J 42

0x08,0xF8,0x88,0xC0,0x28,0x18,0x08,0x00,0x20,0x3F,0x20,0x01,0x26,0x38,0x20,0x00,//K 43

0x08,0xF8,0x08,0x00,0x00,0x00,0x00,0x00,0x20,0x3F,0x20,0x20,0x20,0x20,0x30,0x00,//L 44

0x08,0xF8,0xF8,0x00,0xF8,0xF8,0x08,0x00,0x20,0x3F,0x00,0x3F,0x00,0x3F,0x20,0x00,//M 45

0x08,0xF8,0x30,0xC0,0x00,0x08,0xF8,0x08,0x20,0x3F,0x20,0x00,0x07,0x18,0x3F,0x00,//N 46

0xE0,0x10,0x08,0x08,0x08,0x10,0xE0,0x00,0x0F,0x10,0x20,0x20,0x20,0x10,0x0F,0x00,//O 47

0x08,0xF8,0x08,0x08,0x08,0x08,0xF0,0x00,0x20,0x3F,0x21,0x01,0x01,0x01,0x00,0x00,//P 48

0xE0,0x10,0x08,0x08,0x08,0x10,0xE0,0x00,0x0F,0x18,0x24,0x24,0x38,0x50,0x4F,0x00,//Q 49

0x08,0xF8,0x88,0x88,0x88,0x88,0x70,0x00,0x20,0x3F,0x20,0x00,0x03,0x0C,0x30,0x20,//R 50

0x00,0x70,0x88,0x08,0x08,0x08,0x38,0x00,0x00,0x38,0x20,0x21,0x21,0x22,0x1C,0x00,//S 51

0x18,0x08,0x08,0xF8,0x08,0x08,0x18,0x00,0x00,0x00,0x20,0x3F,0x20,0x00,0x00,0x00,//T 52

0x08,0xF8,0x08,0x00,0x00,0x08,0xF8,0x08,0x00,0x1F,0x20,0x20,0x20,0x20,0x1F,0x00,//U 53

0x08,0x78,0x88,0x00,0x00,0xC8,0x38,0x08,0x00,0x00,0x07,0x38,0x0E,0x01,0x00,0x00,//V 54

0xF8,0x08,0x00,0xF8,0x00,0x08,0xF8,0x00,0x03,0x3C,0x07,0x00,0x07,0x3C,0x03,0x00,//W 55

0x08,0x18,0x68,0x80,0x80,0x68,0x18,0x08,0x20,0x30,0x2C,0x03,0x03,0x2C,0x30,0x20,//X 56

0x08,0x38,0xC8,0x00,0xC8,0x38,0x08,0x00,0x00,0x00,0x20,0x3F,0x20,0x00,0x00,0x00,//Y 57

0x10,0x08,0x08,0x08,0xC8,0x38,0x08,0x00,0x20,0x38,0x26,0x21,0x20,0x20,0x18,0x00,//Z 58

0x00,0x00,0x00,0xFE,0x02,0x02,0x02,0x00,0x00,0x00,0x00,0x7F,0x40,0x40,0x40,0x00,//[ 59

0x00,0x0C,0x30,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x06,0x38,0xC0,0x00,//\ 60

0x00,0x02,0x02,0x02,0xFE,0x00,0x00,0x00,0x00,0x40,0x40,0x40,0x7F,0x00,0x00,0x00,//] 61

0x00,0x00,0x04,0x02,0x02,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//^ 62

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,//_ 63

0x00,0x02,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//` 64

0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x19,0x24,0x22,0x22,0x22,0x3F,0x20,//a 65

0x08,0xF8,0x00,0x80,0x80,0x00,0x00,0x00,0x00,0x3F,0x11,0x20,0x20,0x11,0x0E,0x00,//b 66

0x00,0x00,0x00,0x80,0x80,0x80,0x00,0x00,0x00,0x0E,0x11,0x20,0x20,0x20,0x11,0x00,//c 67

0x00,0x00,0x00,0x80,0x80,0x88,0xF8,0x00,0x00,0x0E,0x11,0x20,0x20,0x10,0x3F,0x20,//d 68

0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x1F,0x22,0x22,0x22,0x22,0x13,0x00,//e 69

0x00,0x80,0x80,0xF0,0x88,0x88,0x88,0x18,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,//f 70

0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x6B,0x94,0x94,0x94,0x93,0x60,0x00,//g 71

0x08,0xF8,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x3F,0x21,0x00,0x00,0x20,0x3F,0x20,//h 72

0x00,0x80,0x98,0x98,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,//i 73

0x00,0x00,0x00,0x80,0x98,0x98,0x00,0x00,0x00,0xC0,0x80,0x80,0x80,0x7F,0x00,0x00,//j 74

0x08,0xF8,0x00,0x00,0x80,0x80,0x80,0x00,0x20,0x3F,0x24,0x02,0x2D,0x30,0x20,0x00,//k 75

0x00,0x08,0x08,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,//l 76

0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x20,0x3F,0x20,0x00,0x3F,0x20,0x00,0x3F,//m 77

0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x3F,0x21,0x00,0x00,0x20,0x3F,0x20,//n 78

0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x1F,0x20,0x20,0x20,0x20,0x1F,0x00,//o 79

0x80,0x80,0x00,0x80,0x80,0x00,0x00,0x00,0x80,0xFF,0xA1,0x20,0x20,0x11,0x0E,0x00,//p 80

0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x0E,0x11,0x20,0x20,0xA0,0xFF,0x80,//q 81

0x80,0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x20,0x20,0x3F,0x21,0x20,0x00,0x01,0x00,//r 82

0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x33,0x24,0x24,0x24,0x24,0x19,0x00,//s 83

0x00,0x80,0x80,0xE0,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x1F,0x20,0x20,0x00,0x00,//t 84

0x80,0x80,0x00,0x00,0x00,0x80,0x80,0x00,0x00,0x1F,0x20,0x20,0x20,0x10,0x3F,0x20,//u 85

0x80,0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x00,0x01,0x0E,0x30,0x08,0x06,0x01,0x00,//v 86

0x80,0x80,0x00,0x80,0x00,0x80,0x80,0x80,0x0F,0x30,0x0C,0x03,0x0C,0x30,0x0F,0x00,//w 87

0x00,0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x31,0x2E,0x0E,0x31,0x20,0x00,//x 88

0x80,0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x80,0x81,0x8E,0x70,0x18,0x06,0x01,0x00,//y 89

0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x21,0x30,0x2C,0x22,0x21,0x30,0x00,//z 90

0x00,0x00,0x00,0x00,0x80,0x7C,0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x3F,0x40,0x40,//{ 91

0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,//| 92

0x00,0x02,0x02,0x7C,0x80,0x00,0x00,0x00,0x00,0x40,0x40,0x3F,0x00,0x00,0x00,0x00,//} 93

0x00,0x06,0x01,0x01,0x02,0x02,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//~ 94

};