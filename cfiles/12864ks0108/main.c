#include <reg52.h>
#include <intrins.h>
#include "hanzi.h" 
    
#include "vvspi.h"
#define uchar unsigned char
#define uint  unsigned int








sbit    CS1=P2^0;
sbit    CS2=P2^1;
sbit    LCD_RST= P2^2;
sbit    RW=P2^3;
sbit    DI=P2^4;
sbit    EN=P2^5;

sfr     LCD=0x80;  

#define	LCD_DISPON	0x3f
#define    LCD_STARTROW	0xc0	
#define	LCD_ADDRSTRY	0xb8	
#define	LCD_ADDRSTRX	0x40 
unsigned char xdata null16x16[]={ 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
unsigned char xdata underline16x16[]={ 
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

};	
unsigned char xdata full16x16[]={ 
0x00,0x00,0x00,0x00,0xfc,0xfc,0xff,0xff,0xff,0xff,0xfc,0xfc,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,

};
unsigned char xdata half16x16[]={ 
0x00,0x00,0x00,0x00,0xfc,0x04,0x07,0x07,0x07,0x07,0x04,0xfc,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,
};
unsigned char xdata empty16x16[]={ 
0x00,0x00,0x00,0x00,0xfc,0x04,0x07,0x07,0x07,0x07,0x04,0xfc,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xff,0x80,0x80,0x80,0x80,0x80,0x80,0xff,0x00,0x00,0x00,0x00,
};
unsigned char xdata error16x16[]={ 
0x00,0x00,0x00,0x00,0xfc,0x24,0x47,0x87,0x87,0x47,0x24,0xfc,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xff,0x84,0x82,0x81,0x81,0x82,0x84,0xff,0x00,0x00,0x00,0x00,
};

void delayus(uint n)
{
	uint i;
	for(i=0;i<n;i++);
}

void	LCD_WrCmd(bit port,uchar  cmd)	
{  
delayus(100);
	EN=0;
   	if(port==0){
		CS1=0;
		delayus(100);
		CS2=1;
		delayus(100);
	}else{
   		CS1=1;
		delayus(100);
   		CS2=0;
		delayus(100);
   }
   DI=0;	
   RW=0;
   EN=1;
   LCD=cmd; 
   EN=0;
}

void  LCD_WrDat(bit port ,uchar wrdata) 	
{  
delayus(100);
	EN=0;
	if(port==0){
	CS1=0;
	delayus(100);
	CS2=1;
	delayus(100);
	}else{
   		CS1=1;
		delayus(100);
   		CS2=0;
		delayus(100);
   }
   DI=1;	
   RW=0;
   EN=1;
   LCD=wrdata; 
   EN=0;
}

void  LCD_DispFill(uchar filldata) 
{  uchar  x, y;
   LCD_WrCmd(0,LCD_STARTROW);
   LCD_WrCmd(1,LCD_STARTROW);	
   for(y=0; y<8; y++)  
   {  LCD_WrCmd(0,LCD_ADDRSTRY+y);
      LCD_WrCmd(0,LCD_ADDRSTRX);
      LCD_WrCmd(1,LCD_ADDRSTRY+y);
      LCD_WrCmd(1,LCD_ADDRSTRX);
   
      for(x=0; x<64; x++)
      {  LCD_WrDat(0,filldata);
         LCD_WrDat(1,filldata);

      }	
   }
}

void  LCD_DispIni(void)			
{
   LCD_RST = 0;	
   delayus(5000);
   LCD_RST = 1;    
   LCD_WrCmd(0,LCD_DISPON);	 
   LCD_WrCmd(0,LCD_STARTROW);   	
   LCD_WrCmd(1,LCD_DISPON);	
   LCD_WrCmd(1,LCD_STARTROW);		
   LCD_DispFill(0x00);
   LCD_WrCmd(0,LCD_ADDRSTRY+0);
   LCD_WrCmd(0,LCD_ADDRSTRX+0);
   LCD_WrCmd(1,LCD_ADDRSTRY+0);
   LCD_WrCmd(1,LCD_ADDRSTRX+0);
}
uchar chrBuf[32];
void getBytesFormGB2312s(char* chr){
	uchar i=0, charSize=0,offsetLen=32;
	unsigned long temp_addr;
	
	

		temp_addr = GB_ADDR(chr,16);
		charSize = GB_HZZF_len(chr);

		CS=0;
	//	WriteByte(0x0b);
	//	WriteByte((temp_addr>>16)&0xff);
	//	WriteByte((temp_addr>>8)&0xff);
	//	WriteByte((temp_addr>>0)&0xff);
	//	WriteByte(0xFF);

		WriteByte(0x03);
		WriteByte((temp_addr>>16)&0xff);
		WriteByte((temp_addr>>8)&0xff);
		WriteByte((temp_addr>>0)&0xff);
		if(charSize==2){
		 	for(i=0;i<offsetLen;i++){
   				chrBuf[i]=ReadByte(); 
   			}
		} else 	if(charSize==1){
	

		 	for(i=0;i<(offsetLen/2);i++){
   				chrBuf[2*i]=ReadByte(); 
				chrBuf[2*i+1]=0x00; 
   			}
		
		}
		CS=1;
    
	
}
/*void getBytesFormGB2312s(char* charArr){
	uchar i=0, byteIndex=0,charSeq=0,charSize=0,offsetLen=32;
	unsigned long temp_addr;
	
	
	while(charSeq<WORDS_PER_MESSAGE)
	{
		temp_addr = GB_ADDR(charArr+byteIndex,CHARACTER_SIZE);
		charSize = GB_HZZF_len(charArr+byteIndex);
		byteIndex +=charSize;
		CS=0;
	//	WriteByte(0x0b);
	//	WriteByte((temp_addr>>16)&0xff);
	//	WriteByte((temp_addr>>8)&0xff);
	//	WriteByte((temp_addr>>0)&0xff);
	//	WriteByte(0xFF);

		WriteByte(0x03);
		WriteByte((temp_addr>>16)&0xff);
		WriteByte((temp_addr>>8)&0xff);
		WriteByte((temp_addr>>0)&0xff);
		if(charSize==2){
		 	for(i=0;i<offsetLen;i++){
   				hztest[offsetLen*charSeq+i]=ReadByte(); 
   			}
		} else 	if(charSize==1){
	

		 	for(i=0;i<(offsetLen/2);i++){
   				hztest[offsetLen*charSeq+2*i]=ReadByte(); 
				hztest[offsetLen*charSeq+2*i+1]=0x00; 
   			}
		
		}
		CS=1;
    	charSeq++;
	}
}*/	
void lcd_disp_sz_char(uchar cy,uchar cx,uchar* chr)
{
	uchar *p,i,s,page;
	bit port;

	getBytesFormGB2312s(chr);
	p=chrBuf;
	if(cx<4)
	{
		port=0;
		s=cx<<4;
	}
	else
	{
		port=1;
		s=((cx-4)<<4);
			
	}
	
	for(page=0;page<2;page++)
	{
		LCD_WrCmd(port,0xb8+cy*2+page);
		delayus(100);
		LCD_WrCmd(port,0x40+s);
		delayus(100);
		for(i=0;i<16;i++)
		{
			LCD_WrDat(port,*p);
			delayus(10);
			p++;
		}
	}
}
void LCD_PutString(unsigned char cx,unsigned char cy,unsigned char  *s)
{
	uchar wordCount=0,charSize=0;
//	lcd_disp_sz_char(0,0,s);
//	lcd_disp_sz_char(1,4,s+2);
	while(*s!='\0'&&wordCount<10)
	{
		charSize = GB_HZZF_len(s);
		lcd_disp_sz_char(cy,cx,s);
		wordCount++;
		s++;
		if(charSize==2)
		{
		s++;
		}
		cx++;
		if(cx>7) {cx=0;cy++;}
	}
 
}
uchar *lbatStateGraph=0;
void FullCGRAM()
{ 
     lbatStateGraph=full16x16;
}   
void HalfCGRAM()
{ 
     lbatStateGraph=half16x16;
}
void EmptyCGRAM()
{ 
     lbatStateGraph=empty16x16;
} 
void ErrorCGRAM()
{ 
     lbatStateGraph=error16x16;
}
void DisplayCGRAM(unsigned char cx,unsigned char cy)
{
	uchar *p,i,s,page;
	bit port;

	
	p=lbatStateGraph;
	if(cx<4)
	{
		port=0;
		s=cx<<4;
	}
	else
	{
		port=1;
		s=((cx-4)<<4);
			
	}
	
	for(page=0;page<2;page++)
	{
		LCD_WrCmd(port,0xb8+cy*2+page);
		delayus(100);
		LCD_WrCmd(port,0x40+s);
		delayus(100);
		for(i=0;i<16;i++)
		{
			LCD_WrDat(port,*p);
			delayus(10);
			p++;
		}
	}
} 
uchar oldUnderlineX=0,oldUnderlineY=0;
void DisplayUnderline(unsigned char cx,unsigned char cy,bit display)
{
	uchar *p,i,s,page;
	bit port;

	if(oldUnderlineX!=0||oldUnderlineY!=0){	
		p=null16x16;
		if(oldUnderlineX<4)
		{
			port=0;
			s=oldUnderlineX<<4;
		}
		else
		{
			port=1;
			s=((oldUnderlineX-4)<<4);
			
		}
	
		for(page=0;page<2;page++)
		{
			LCD_WrCmd(port,0xb8+oldUnderlineY*2+page);
			delayus(100);
			LCD_WrCmd(port,0x40+s);
			delayus(100);
			for(i=0;i<16;i++)
			{
				LCD_WrDat(port,*p);
				delayus(10);
				p++;
			}
		}
	}
	oldUnderlineX=cx;
	oldUnderlineY=cy;
	p=underline16x16;
	if(cx<4)
	{
		port=0;
		s=cx<<4;
	}
	else
	{
		port=1;
		s=((cx-4)<<4);
			
	}
	
	for(page=0;page<2;page++)
	{
		LCD_WrCmd(port,0xb8+cy*2+page);
		delayus(100);
		LCD_WrCmd(port,0x40+s);
		delayus(100);
		for(i=0;i<16;i++)
		{
			LCD_WrDat(port,*p);
			delayus(10);
			p++;
		}
	}


}   
#define WORDS_PER_MESSAGE 4//每句话显示文字数
#define CHARACTER_SIZE 16   //每个字占用点阵大小
#define SENTENCE_BUF_LEN 128 //ram中 保存每句的点阵编码
unsigned char xdata hztest[SENTENCE_BUF_LEN];


void  main(void)
{ 

	uchar xdata str[] = "阿埃",str2[] = "渭南光电科技";
		init_spi();
	 
	LCD_DispIni();
		LCD_PutString(0,0,str);
//		LCD_PutString(4,1,str2);
		FullCGRAM();
		DisplayCGRAM(1,3);
	while(1)
   {  
  //  delayus(10000);	
    //  lcd_disp_sz_str(0,0,str);
	
    
   }
}
