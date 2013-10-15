#include<reg52.h>
#include "vvspi.h"
#include "insideStorage.h"
#include "hanzi.h"
#define DISPLAY_NAME 0
#define DISPLAY_TIME 1
#define FULL_TYPE 1
#define HALF_TYPE 2
#define LEFT_POS 1
#define CENTER_POS 2
#define RIGHT_POS 3
#define STR_BUF_LEN 128
#define WORD_COUNT 4
#define CHARACTER_SIZE 16
sbit T_CLK = P2^7;      //时钟 对应sk
sbit T_STR = P2^6;      //锁存 对应lt
sbit T_IO  = P2^5;       //数据1
sbit T_IO2  = P2^4;       //数据2
sbit OE=P3^6;
sfr P2M1=0x95;
sfr P2M0=0x96;

bit displayType=1,isCommAndProc=0;
unsigned char  address;
unsigned char tab[8];
unsigned char DIS[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
unsigned long timeElapsed=0;
unsigned char nameNum=10;
unsigned char xdata nameArr[32][10]={'\0'};
unsigned char  * code testName[10]={"测试灯零","测试灯一","测试灯二","测试灯叁","测试灯四","测试灯五","测试灯六","测试灯七","测试灯八","测试灯九",};
unsigned char nameSeq=0;
unsigned char code fixedChar[]={
/*--  文字:  区  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x00,0x00,0x7F,0xFC,0x40,0x20,0x40,0x30,0x48,0x20,0x46,0x40,0x41,0x40,0x40,0x80,
0x41,0x40,0x42,0x20,0x44,0x30,0x48,0x18,0x50,0x10,0x7F,0xFC,0x00,0x00,0x00,0x00,

/*--  文字:  架  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x08,0x00,0x08,0x00,0x7F,0x7C,0x09,0x44,0x09,0x44,0x11,0x44,0x15,0x7C,0x62,0x00,
0x01,0x00,0x7F,0xFC,0x03,0x80,0x05,0x40,0x09,0x30,0x31,0x0E,0xC1,0x04,0x01,0x00,
};
unsigned char code fixedNum[]={
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

/*--  文字:  -  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
unsigned char code fixedAlpha[]={
/*--  文字  空格 --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  A  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x10,0x10,0x18,0x28,0x28,0x24,0x3C,0x44,0x42,0x42,0xE7,0x00,0x00,

/*--  文字:  B  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xF8,0x44,0x44,0x44,0x78,0x44,0x42,0x42,0x42,0x44,0xF8,0x00,0x00,

/*--  文字:  C  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x3E,0x42,0x42,0x80,0x80,0x80,0x80,0x80,0x42,0x44,0x38,0x00,0x00,

/*--  文字:  D  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xF8,0x44,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x44,0xF8,0x00,0x00,

/*--  文字:  E  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xFC,0x42,0x48,0x48,0x78,0x48,0x48,0x40,0x42,0x42,0xFC,0x00,0x00,

/*--  文字:  F  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xFC,0x42,0x48,0x48,0x78,0x48,0x48,0x40,0x40,0x40,0xE0,0x00,0x00,

/*--  文字:  G  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x3C,0x44,0x44,0x80,0x80,0x80,0x8E,0x84,0x44,0x44,0x38,0x00,0x00,

/*--  文字:  H  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xE7,0x42,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0xE7,0x00,0x00,

/*--  文字:  I  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x7C,0x00,0x00,

/*--  文字:  J  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x3E,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x88,0xF0,

/*--  文字:  K  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xEE,0x44,0x48,0x50,0x70,0x50,0x48,0x48,0x44,0x44,0xEE,0x00,0x00,

/*--  文字:  L  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xE0,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x42,0xFE,0x00,0x00,

/*--  文字:  M  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xEE,0x6C,0x6C,0x6C,0x6C,0x54,0x54,0x54,0x54,0x54,0xD6,0x00,0x00,

/*--  文字:  N  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xC7,0x62,0x62,0x52,0x52,0x4A,0x4A,0x4A,0x46,0x46,0xE2,0x00,0x00,

/*--  文字:  O  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x38,0x44,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x44,0x38,0x00,0x00,

/*--  文字:  P  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xFC,0x42,0x42,0x42,0x42,0x7C,0x40,0x40,0x40,0x40,0xE0,0x00,0x00,

/*--  文字:  Q  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x38,0x44,0x82,0x82,0x82,0x82,0x82,0xB2,0xCA,0x4C,0x38,0x06,0x00,

/*--  文字:  R  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xFC,0x42,0x42,0x42,0x7C,0x48,0x48,0x44,0x44,0x42,0xE3,0x00,0x00,

/*--  文字:  S  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x3E,0x42,0x42,0x40,0x20,0x18,0x04,0x02,0x42,0x42,0x7C,0x00,0x00,

/*--  文字:  T  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xFE,0x92,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00,

/*--  文字:  U  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xE7,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,

/*--  文字:  V  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xE7,0x42,0x42,0x44,0x24,0x24,0x28,0x28,0x18,0x10,0x10,0x00,0x00,

/*--  文字:  W  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xD6,0x92,0x92,0x92,0x92,0xAA,0xAA,0x6C,0x44,0x44,0x44,0x00,0x00,

/*--  文字:  X  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xE7,0x42,0x24,0x24,0x18,0x18,0x18,0x24,0x24,0x42,0xE7,0x00,0x00,

/*--  文字:  Y  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0xEE,0x44,0x44,0x28,0x28,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00,

/*--  文字:  Z  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x7E,0x84,0x04,0x08,0x08,0x10,0x20,0x20,0x42,0x42,0xFC,0x00,0x00,


};
unsigned char xdata strBuffer[STR_BUF_LEN]={
0x00
};
/*void delayUs2x(unsigned char t) {
	while (--t)
		;
}

void delayMs(unsigned int t) {

	while (t--) {

		delayUs2x(245);
		delayUs2x(245);
	}
}
*/
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


void Init_Timer0(void)
{
	TMOD |= 0x01;			     
//1毫秒

	TH0=(65536-922)/256;	      
 	TL0=(65536-922)%256;
	EA=1;                    
	ET0=1;                     
	TR0=1;                   
}
void displayChar(unsigned char *charIndex,unsigned char x,unsigned char type,unsigned char pos){
	unsigned char i;

	if(type==FULL_TYPE){
		for(i=0;i<32;i++)
		{
			strBuffer[x*32+i]=charIndex[i];
		}
	}else
	{
		if(pos==LEFT_POS){
			for(i=0;i<16;i++)
			{
				strBuffer[x*32+2*i]=charIndex[i];
			//	strBuffer[x*32+2*i+1]=0x00;
			}
		}
		else if(pos==RIGHT_POS){
			for(i=0;i<16;i++)
			{
			//	strBuffer[x*32+2*i]=0x00;
				strBuffer[x*32+2*i+1]=charIndex[i];
				
			}
		}
		else{
			for(i=0;i<16;i++)
			{
				strBuffer[x*32+2*i]=(charIndex[i]>>4)&0x0f;
				strBuffer[x*32+2*i+1]=(charIndex[i]<<4)&0xf0;
				
			}
		}
	}
	
}
unsigned char hour=0,minute=0,second=0;
void getBytesFormGB2312s(char* charArr);
void createDisplay(){
	if(displayType==DISPLAY_TIME)
	{
		displayChar(&fixedNum[16*(hour/10)],0,HALF_TYPE,LEFT_POS);
		displayChar(&fixedNum[16*(hour%10)],0,HALF_TYPE,RIGHT_POS);
		displayChar(&fixedNum[16*(10)],1,HALF_TYPE,LEFT_POS);//显示 -
		displayChar(&fixedNum[16*(minute/10)],1,HALF_TYPE,RIGHT_POS);
		displayChar(&fixedNum[16*(minute%10)],2,HALF_TYPE,LEFT_POS);
		displayChar(&fixedNum[16*(10)],2,HALF_TYPE,RIGHT_POS);//显示 -
		displayChar(&fixedNum[16*(second/10)],3,HALF_TYPE,LEFT_POS);
		displayChar(&fixedNum[16*(second%10)],3,HALF_TYPE,RIGHT_POS);
	}
	else{
		getBytesFormGB2312s(nameArr[nameSeq]);
	}
}
bit timeStart=0,oldDisplayType=1;
bit inputChanged=1;
unsigned char oldNameSeq=255; //初始化为有效名称编号之外的值
void inputSelection(){

	displayType=(P0>>7)&0x01;
	if(displayType!=oldDisplayType){
		inputChanged=1;
		oldDisplayType=displayType;
	}
	if(displayType==DISPLAY_TIME){
		timeStart=P0&0x01;
	}
	else{
		hour=minute=second=0;
		nameSeq=P0&0x1f;
		if(nameSeq>=nameNum){
			nameSeq=0;
		}
		if(oldNameSeq!=nameSeq){
			inputChanged=1;
			oldNameSeq=nameSeq;
		}
	}
}
void getBytesFormGB2312s(char* charArr){
	unsigned char offsetLen=32, byteIndex=0,charSeq=0,charSize=0,tempByte;
	unsigned long temp_addr;
	unsigned int i,m;
	if(inputChanged==0){
		return;
	}
	inputChanged=0;
	for(m=0;m<STR_BUF_LEN;m++)
	{
		strBuffer[m]=0x0;
	}
	
	while(charSeq<WORD_COUNT&&charArr[byteIndex]!='\0')
	{
		temp_addr = GB_ADDR(charArr+byteIndex,CHARACTER_SIZE);
		charSize = GB_HZZF_len(charArr+byteIndex);
		byteIndex +=charSize;
		CS=0;

		WriteByte(0x03);
		WriteByte((temp_addr>>16)&0xff);
		WriteByte((temp_addr>>8)&0xff);
		WriteByte((temp_addr>>0)&0xff);
		if(charSize==2){
		 	for(i=0;i<offsetLen;i++){
				if(isCommAndProc==1){
					 return;
				}
   				strBuffer[offsetLen*charSeq+i]=ReadByte(); 
   			}
		} else 	if(charSize==1){
	

		 	for(i=0;i<(offsetLen/2);i++){
				if(isCommAndProc==1){
	 				return;
				}
				tempByte=ReadByte();
   				strBuffer[offsetLen*charSeq+2*i]=tempByte; 
				strBuffer[offsetLen*charSeq+2*i+1]=0x00; 
   			}
		
		}
		CS=1;
    	charSeq++;
	}
}
void init(){
	unsigned char i,j;
	P2M0=0x0f;	
	OE=0;
	for(i=0;i<nameNum;i++){
		for(j=0;j<9;j++){
	  		nameArr[i][j]=testName[i][j];
	  	}
	}
	Init_Timer0(); 
	//初始化spi
	init_spi();

}
main()
{
	
	inputSelection();
	init();

	
	while(1)
	{ 
		inputSelection();
		createDisplay();
	}
}
unsigned int milSecond=0;
void Timer0_isr(void) interrupt 1 using 1
{
	static unsigned char count=0,j;
	static unsigned char i;
	//1毫秒

	TH0=(65536-922)/256;	      
 	TL0=(65536-922)%256; 
	if(displayType==DISPLAY_TIME){
		if(timeStart==0){
			hour=minute=second=0;
		}
		else{
			milSecond++;
			if(milSecond==1000){
				second++;
				milSecond=0;
			}
			
			if(second==60){
				minute++;
				second=0;
			}
			if(minute==60){
				hour++;
				minute=0;
			}
			if(hour==99){
				hour=0;
			}
		}
	}
	for(j=0;j<4;j++) 
	{
		InputByte(strBuffer[j*32+i]);
		InputByte(strBuffer[j*32+i+1]);
	}
	T_STR=0; 
	T_STR=1; 
	P2=(P2&0xf0)|DIS[count];
	count++;
	i+=2;             
	if(count==16)
	count=0;
	if(i==32)
	{
		i=0;
	}
}


