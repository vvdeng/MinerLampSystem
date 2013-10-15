
#include <reg52.h>  
#include<intrins.h> 


//需配置部分
//#define LED_UNITS_COUNT 4 //单元板数量
#define WORDS_PER_MESSAGE 16//每句话显示文字数
#define LED_UNITS_WIDTH 64  //led屏宽度 (点数)
#define LED_UNITS_HIGHT 16  //led屏高度 (点数)
#define MESSAGES_DISPLAY_BUF_LEN 512 //ram中保存所有message的LED点阵编码


#define CHARACTER_SIZE 16   //每个字占用点阵大小

//配置结束
#define uchar unsigned char
#define uint unsigned int
#define  _Nop()  _nop_()
//串口1 485读写控制
sbit RW_485=P3^7;
uchar  code digitCode[10][16]={
/*--  文字:  0  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x18,0x24,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x18,0x00,0x00,

/*--  文字:  1  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x10,0x70,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x7C,0x00,0x00,

/*--  文字:  2  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x04,0x04,0x08,0x10,0x20,0x42,0x7E,0x00,0x00,

/*--  文字:  3  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x3C,0x42,0x42,0x04,0x18,0x04,0x02,0x02,0x42,0x44,0x38,0x00,0x00,

/*--  文字:  4  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x04,0x0C,0x14,0x24,0x24,0x44,0x44,0x7E,0x04,0x04,0x1E,0x00,0x00,

/*--  文字:  5  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x7E,0x40,0x40,0x40,0x58,0x64,0x02,0x02,0x42,0x44,0x38,0x00,0x00,

/*--  文字:  6  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x1C,0x24,0x40,0x40,0x58,0x64,0x42,0x42,0x42,0x24,0x18,0x00,0x00,

/*--  文字:  7  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x7E,0x44,0x44,0x08,0x08,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,

/*--  文字:  8  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x24,0x18,0x24,0x42,0x42,0x42,0x3C,0x00,0x00,

/*--  文字:  9  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x18,0x24,0x42,0x42,0x42,0x26,0x1A,0x02,0x02,0x24,0x38,0x00,0x00,
};
// （编号从零开始）灯架号位置：第2个16*16, 取走个数位置：第6个的右半和第7个 充电个数位置：第10个右半和第11个 充满个数位置：第14个右半和第15个 
uchar xdata dispBuf[MESSAGES_DISPLAY_BUF_LEN]={
/*--  文字:  灯  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x10,0x00,0x10,0x00,0x11,0xFE,0x10,0x10,0x54,0x10,0x54,0x10,0x58,0x10,0x90,0x10,
0x10,0x10,0x18,0x10,0x24,0x10,0x22,0x10,0x40,0x10,0x40,0x50,0x80,0x20,0x00,0x00,

/*--  文字:  架  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x08,0x00,0x08,0x00,0x7F,0x7C,0x09,0x44,0x09,0x44,0x11,0x44,0x15,0x7C,0x62,0x00,
0x01,0x00,0x7F,0xFC,0x03,0x80,0x05,0x40,0x09,0x30,0x31,0x0E,0xC1,0x04,0x01,0x00,

/*--  文字:    空格 --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:    空格 --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

/*--  文字:    空格 --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:    空格 --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

/*--  文字:  取  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x00,0x00,0xFF,0x80,0x22,0x7E,0x22,0x44,0x3E,0x44,0x22,0x44,0x22,0x44,0x3E,0x28,
0x22,0x28,0x22,0x90,0x27,0x10,0x3A,0x28,0xC2,0x46,0x02,0x84,0x02,0x00,0x02,0x00,

/*--  文字:  走  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x01,0x00,0x01,0x00,0x3F,0xFC,0x01,0x00,0x01,0x00,0x01,0x00,0xFF,0xFE,0x01,0x00,
0x11,0x00,0x11,0xF8,0x11,0x00,0x11,0x00,0x29,0x00,0x45,0x00,0x83,0xFE,0x00,0x00,

/*--  文字:  ：  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x30,0x00,0x30,0x00,0x00,0x00,0x30,0x00,0x30,0x00,0x00,0x00,0x00,0x00,

/*--  文字:    空格 --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

/*--  文字:    空格 --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,




/*--  文字:  充  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x02,0x00,0x01,0x00,0x7F,0xFC,0x02,0x00,0x07,0x40,0x0C,0x20,0x10,0x10,0x3F,0xF8,
0x04,0x90,0x04,0x80,0x04,0x80,0x08,0x80,0x08,0x84,0x10,0x84,0x20,0x7C,0x40,0x00,

/*--  文字:  电  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x01,0x00,0x01,0x00,0x01,0x00,0x3F,0xF8,0x21,0x08,0x21,0x08,0x3F,0xF8,0x21,0x08,
0x21,0x08,0x21,0x08,0x3F,0xF8,0x21,0x08,0x01,0x02,0x01,0x02,0x00,0xFE,0x00,0x00,

/*--  文字:  ：  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x30,0x00,0x30,0x00,0x00,0x00,0x30,0x00,0x30,0x00,0x00,0x00,0x00,0x00,

/*--  文字:    空格 --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

/*--  文字:    空格 --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,




/*--  文字:  充  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x02,0x00,0x01,0x00,0x7F,0xFC,0x02,0x00,0x07,0x40,0x0C,0x20,0x10,0x10,0x3F,0xF8,
0x04,0x90,0x04,0x80,0x04,0x80,0x08,0x80,0x08,0x84,0x10,0x84,0x20,0x7C,0x40,0x00,

/*--  文字:  满  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x41,0x10,0x21,0x10,0x2F,0xFE,0x01,0x10,0x80,0x00,0x57,0xFE,0x51,0x20,0x17,0xFC,
0x25,0x24,0x25,0x24,0x25,0xB4,0xC6,0x4C,0x44,0x84,0x44,0x04,0x44,0x14,0x44,0x08,

/*--  文字:  ：  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x30,0x00,0x30,0x00,0x00,0x00,0x30,0x00,0x30,0x00,0x00,0x00,0x00,0x00,

/*--  文字:    空格 --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

/*--  文字:    空格 --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,





};

uchar  commState1;

bit inSetting;








#define BYTE_LEN 8
unsigned char offsetCountPerRow=CHARACTER_SIZE/8,emptyCount=0;
unsigned char offsetLen=CHARACTER_SIZE*(CHARACTER_SIZE/8);
sfr P1M1=0x91;
sfr P1M0=0x92;
sfr P2M1=0x95;
sfr P2M0=0x96;

sbit T_CLK = P2^7;      //时钟 对应sk
sbit T_STR = P2^6;      //锁存 对应lt
sbit T_IO  = P2^5;       //数据1
sbit T_IO2  = P2^4;       //数据2
sbit EN=P3^6;

bit timeFlag,rowDisplayFinished=1,showMessageFlag=1;
unsigned char DIS[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

void InputByte(unsigned  char ucDa) 
{ 
	unsigned char i;
	for(i=8; i>0; i--)
	{
		T_IO = !((ucDa>>7)&0x01); 
		T_CLK =	0;                  
		T_CLK = 1;
		ucDa=ucDa<<1;
	} 
}
#define BUF_LEN 4

uchar bufIndex;
uchar xdata buf[BUF_LEN]={0};//0架号 1 取走数量 2充电数量 3充满数量
#define CMD_SEND_STA 0x80
#define COMM_STATE_NOTHING 0
#define COMM_STATE_WAITING_DATA 1
#define COMM_STATE_FINISHED 2
#define SYM_DATA_END 0xff
void ssio(void)
interrupt 4
{
          
	if(RI){
         RI=0;
		
	 	if(SBUF==CMD_SEND_STA){
			
			commState1=COMM_STATE_WAITING_DATA;
			bufIndex=0;
	
			return;
		}



		if((commState1==COMM_STATE_WAITING_DATA)){
			if(bufIndex>=BUF_LEN){
				if(SBUF==SYM_DATA_END){
					commState1=COMM_STATE_FINISHED;
				}
				else{
					commState1=COMM_STATE_NOTHING;
				}
			}
			else{
						
					buf[bufIndex++]=SBUF;
				
			}
		}
	
               
	}
	else if(TI){                       
		TI=0;
	}

}

bit isRefresh;
unsigned char timerCount=0,timerI,timerJ;
uint timeCount=0;
void Timer0_isr(void) interrupt 1 using 1
{
	//1毫秒

	TH0=(65536-922)/256;	      
 	TL0=(65536-922)%256;
    //0.05毫秒
//  TH0=(65536-46)/256;	      
//	TL0=(65536-46)%256;

	//0.025毫秒
//	TH0=0xFF;
//	TL0=0xE9;

	if(isRefresh==0){
	 	isRefresh=1;
	}



}
void timerAction(){
	if(showMessageFlag==0){
		 return;
	}
	if(isRefresh==0){
		return;
	}
	for(timerJ=0;timerJ<16;timerJ++) 
	{
		InputByte(dispBuf[timerJ*32+timerI]);
		InputByte(dispBuf[timerJ*32+timerI+1]);
	}
	T_STR=0; 
	T_STR=1; 
	P2=(P2&0xf0)|DIS[timerCount];
	timerCount++;
	timerI+=2;             
	if(timerCount==16)
	timerCount=0;
	if(timerI==32)
	{
		timerI=0;
	}
	isRefresh=0;
}




void init() {
	EN=0; //低电平有效
	RW_485=0; //485初始处于接收状态
  	
	//定时器0初始化
 	TMOD |= 0x01;	  		     
//  TH0=(65536-922)/256;	      
//	TL0=(65536-922)%256;
    //0.05毫秒
  	TH0=(65536-46)/256;	  
    TL0=(65536-46)%256;
	//0.025毫秒
//	TH0=0xFF;
//	TL0=0xE9;
	ET0=1;           
	TR0=1;  
  	
	//串口1初始化
    SCON  = 0x50;    
	TMOD |= 0x20;
	TH1 = 0xfd;
	TL1 = 0xfd;
	TR1 = 1;


	IP=0x10;//设定串口1为髙优先级
	ES=1; //开串口1中断
			
	//外部中断1 初始化
  	EX1=1;         
    IT1=1;  

	EA = 1;



}
#define LEFT 0
#define RIGHT 1
void updateDispBuf(uint n,uchar lorR,uchar *  digitPtr){
	uchar i;
	
	for(i=0;i<16;i++){
    	dispBuf[n*32+2*i+lorR]=digitPtr[i];	
	}
}
void refreshSta(){
	uchar i;
	//更新灯架号
	updateDispBuf(2,LEFT,digitCode[buf[0]/10]);
	updateDispBuf(2,RIGHT,digitCode[buf[0]%10]);
 /*   for(i=1;i<BUF_LEN;i++){
		updateDispBuf(4*i+2,RIGHT,digitCode[buf[i]/100]);
		updateDispBuf(4*i+3,LEFT,digitCode[(buf[i]/10)%10]);
		updateDispBuf(4*i+3,RIGHT,digitCode[buf[i]%10]);
	}*/
	updateDispBuf(6,RIGHT,digitCode[buf[1]/100]);
	updateDispBuf(7,LEFT,digitCode[(buf[1]/10)%10]);
	updateDispBuf(7,RIGHT,digitCode[buf[1]%10]);

	updateDispBuf(10,RIGHT,digitCode[buf[2]/100]);
	updateDispBuf(11,LEFT,digitCode[(buf[2]/10)%10]);
	updateDispBuf(11,RIGHT,digitCode[buf[2]%10]);

	updateDispBuf(14,RIGHT,digitCode[buf[3]/100]);
	updateDispBuf(15,LEFT,digitCode[(buf[3]/10)%10]);
	updateDispBuf(15,RIGHT,digitCode[buf[3]%10]);
}
void delayUs(uint t){
	while(t--);
}
void delayMs(uint t){
	while(t--)
	{
		delayUs(245);
		delayUs(245);
		delayUs(245);
		delayUs(245);
	}
}
void test(){
	
	if(commState1==COMM_STATE_FINISHED){
		
	}
}
void main()
{
	
	
 	P2M1=0x00;
	P2M0=0x0f;	

	
	init();



 	while(1){
		
	
		timerAction();
	
	 	if(commState1==COMM_STATE_FINISHED){
		
			refreshSta();
		
			commState1=COMM_STATE_NOTHING;
		}
	}
}

