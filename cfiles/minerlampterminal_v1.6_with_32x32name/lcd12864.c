/*
ע�⣡����������������������������������������������������������������������������������������11
���ܵ���CS1��CS2�Ǹߵ�ƽ��Ч������ʱʱ�侭ʵ����ѡΪ200us
�г��ϵ���CS1��CS2�ǵ͵�ƽ��Ч������ʱʱ�侭ʵ����ѡΪ100us
*/
#include "lcd12864.h"
#include "hanzi.h"
#include "vvspi.h"
#define ENABLE 1
#define DISENABLE 0
extern bit infoChanged;
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

/*unsigned char xdata half16x16[]={
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,  
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, };*/
unsigned char xdata empty16x16[]={ 
0x00,0x00,0x00,0x00,0xfc,0x04,0x07,0x07,0x07,0x07,0x04,0xfc,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xff,0x80,0x80,0x80,0x80,0x80,0x80,0xff,0x00,0x00,0x00,0x00,
};

unsigned char xdata error16x16[]={ 
0x00,0x00,0x00,0x00,0xfc,0x24,0x47,0x87,0x87,0x47,0x24,0xfc,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xff,0x84,0x82,0x81,0x81,0x82,0x84,0xff,0x00,0x00,0x00,0x00,
};
unsigned char xdata arrow16x16[]={ 
0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x90,0xA0,0xC0,0x80,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x02,0x01,0x00,
};
void delayus(uint n)
{
	uint i;
	for(i=0;i<n;i++);
}
void	LCD_WrCmd(uchar port,uchar  cmd)	
{  
delayus(DISPLAY_DELAY_TIME);
	EN=0;
   	if(port==1){
		CS1=ENABLE;
		delayus(DISPLAY_DELAY_TIME);
		CS2=DISENABLE;
		delayus(DISPLAY_DELAY_TIME);
	}
	else
	{
   		CS1=DISENABLE;
		delayus(DISPLAY_DELAY_TIME);
   		CS2=ENABLE;
		delayus(DISPLAY_DELAY_TIME);
   }
   DI=0;	
   RW=0;
   EN=1;
   LCD=cmd; 
   EN=0;
}

void  LCD_WrDat(uchar port ,uchar wrdata) 	
{  
delayus(DISPLAY_DELAY_TIME);
	EN=0;
	if(port==1){
		CS1=ENABLE;
		delayus(DISPLAY_DELAY_TIME);
		CS2=DISENABLE;
		delayus(DISPLAY_DELAY_TIME);
	}else{
   		CS1=DISENABLE;
		delayus(DISPLAY_DELAY_TIME);
   		CS2=ENABLE;
		delayus(DISPLAY_DELAY_TIME);
   }
   DI=1;	
   RW=0;
   EN=1;
   LCD=wrdata; 
   EN=0;
}

void  LCD_DispFill(uchar filldata) 
{  uchar  x, y;
   LCD_WrCmd(1,LCD_STARTROW);
   LCD_WrCmd(2,LCD_STARTROW);	
   for(y=0; y<8; y++)  
   {  LCD_WrCmd(1,LCD_ADDRSTRY+y);
      LCD_WrCmd(1,LCD_ADDRSTRX);
      LCD_WrCmd(2,LCD_ADDRSTRY+y);
      LCD_WrCmd(2,LCD_ADDRSTRX);
   
      for(x=0; x<64; x++)
      {  LCD_WrDat(1,filldata);
         LCD_WrDat(2,filldata);

      }	
   }
}

void  LCD_DispIni(void)			
{

   LCD_RST = 0;	
   delayus(1000);
   LCD_RST = 1;  
   delayus(1000);
   LCD_WrCmd(1,LCD_DISPON);	 
   LCD_WrCmd(1,LCD_STARTROW);
   delayus(1000);	
   LCD_WrCmd(2,LCD_DISPON);	
   LCD_WrCmd(2,LCD_STARTROW);		
   LCD_DispFill(0x00);
   LCD_WrCmd(1,LCD_ADDRSTRY+0);
   LCD_WrCmd(1,LCD_ADDRSTRX+0);
   LCD_WrCmd(2,LCD_ADDRSTRY+0);
   LCD_WrCmd(2,LCD_ADDRSTRX+0);
}
uchar xdata chrBuf[32];
void getBytesFormGB2312s(char* chr){
	uchar i=0, charSize=0,offsetLen=32;
	unsigned long temp_addr;
	
	
	    //��12x12����
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
   				chrBuf[2*i]=ReadByte(); //todo�˴��㷨�����
				chrBuf[2*i+1]=0x00; 
   			}
		
		}
		CS=1;	
}

void lcd_disp_sz_char(uchar cx,uchar cy,uchar* chr)
{
	uchar *p,i,s,page;
	uchar port;

	getBytesFormGB2312s(chr);
	p=chrBuf;
	if(cx<4)
	{
		port=1;
		s=cx<<4;
	}
	else
	{
		port=2;
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
//	lcd_disp_sz_char(4,1,s+2);
	while(*s!='\0'&&wordCount<10)
	{
		charSize = GB_HZZF_len(s);
		lcd_disp_sz_char(cx,cy,s);
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

uchar xdata chrBuf_24[72];
void get24x24BytesFormGB2312s(char* chr,uchar * buf){
	uchar i=0, charSize=0,offsetLen=72;
	unsigned long temp_addr;
	
	

		temp_addr = GB_ADDR(chr,24);
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
   				buf[i]=ReadByte(); 
   			}
		} else 	if(charSize==1){
	

		 	for(i=0;i<(offsetLen/2);i++){
   				buf[2*i]=ReadByte(); //todo�˴��㷨�����
				buf[2*i+1]=0x00; 
   			}
		
		}
		CS=1;	
}


void lcd_disp_sz_char_24(uchar cx,uchar cy,uchar* chr,uchar* buf)
{
	uchar *p,i,s,page;
	uchar port;
	if(chr!=0)
	{
		get24x24BytesFormGB2312s(chr,buf);
	}
	p=chrBuf_24;
	if(cx<2)
	{
		port=1;
		s=cx*24;
	}
	else
	{
		port=2;
		s=((cx-2)*24);
			
	}
	
	for(page=0;page<3;page++)
	{
		LCD_WrCmd(port,0xb8+cy*3+page);
		delayus(100);
		LCD_WrCmd(port,0x40+s);
		delayus(100);
		for(i=0;i<24;i++)
		{
			LCD_WrDat(port,*p);
			delayus(10);
			p++;
		}
	}
}


void LCD_PutString_24(unsigned char cx,unsigned char cy,unsigned char  *s)
{
	uchar wordCount=0,charSize=0;

//	lcd_disp_sz_char(0,0,s);
//	lcd_disp_sz_char(4,1,s+2);
	while(*s!='\0'&&wordCount<10)
	{
		charSize = GB_HZZF_len(s);
		lcd_disp_sz_char_24(cx,cy,s,chrBuf_24);
		wordCount++;
		s++;
		if(charSize==2)
		{
		s++;
		}
		cx++;
		if(cx>4) {cx=0;cy++;}
	}
 
}
void lcd_disp_sz_char_24_original_x(uchar x,uchar cy,uchar* chr,uchar* buf)
{
	uchar *p,i,s,page;
	uchar port;
	if(chr!=0)
	{
		get24x24BytesFormGB2312s(chr,buf);
	}
	p=buf;
	if(x<64)
	{
		port=1;
		s=x;
	}
	else
	{
		port=2;
		s=x-64;
			
	}
	
	for(page=0;page<3;page++)
	{
		LCD_WrCmd(port,0xb8+cy*3+page);
		delayus(100);
		LCD_WrCmd(port,0x40+s);
		delayus(100);
		for(i=0;i<24;i++)
		{
			LCD_WrDat(port,*p);
			delayus(10);
			p++;
		}
	}
}

void LCD_PutString_24_cy(unsigned char cy,unsigned char  *s)
{
	uchar wordCount=0,charSize=0, *p=s;
	uchar x[4];
	//������������ȷ�����з�ʽ
	while(*p!='\0'){
		charSize= GB_HZZF_len(p);
		wordCount++;
		p++;
		if(charSize==2){
			p++;
		}
	}
	switch (wordCount){
	case 1:
		x[0]=39;
		break;
	case 2:
	    x[0]=29;
		x[1]=74;
		break;
	case 3:
		x[0]=38;
		x[1]=64;
		x[2]=86;
		break;
	default:
		x[0]=0;
		x[1]=34;
		x[2]=69;
		x[3]=103;
	}
	wordCount=0;
//	lcd_disp_sz_char(0,0,s);
//	lcd_disp_sz_char(4,1,s+2);
	while(*s!='\0'&&wordCount<4)
	{
		charSize = GB_HZZF_len(s);
		lcd_disp_sz_char_24_original_x(x[wordCount],cy,s,chrBuf_24);
		wordCount++;
		s++;
		if(charSize==2)
		{
		s++;
		}
		
	}
 
}
uchar xdata nameCharBuf[288];
void get24x24NameBytesFormGB2312s(char* chr,uint startIndex){
	uchar i=0, charSize=0,offsetLen=72;
	unsigned long temp_addr;
	
	

		temp_addr = GB_ADDR(chr,24);
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
   				nameCharBuf[i+startIndex]=ReadByte(); 
   			}
		} else 	if(charSize==1){
	

		 	for(i=0;i<(offsetLen/2);i++){
   				nameCharBuf[2*i+startIndex]=ReadByte(); //todo�˴��㷨�����
				nameCharBuf[2*i+1+startIndex]=0x00; 
   			}
		
		}
		CS=1;	
}
void arrangeNameArr(uchar wordCount){
	uint m;
	//����ȫ��Ӳ����
	switch(wordCount){
	case 1:
	   for (m=72;m<288;m++){
	     nameCharBuf[m]=0;
	   }
	   for (m=72;m<144;m++){
	   	if(m%24>11){
	     nameCharBuf[m]=nameCharBuf[m-72-12];
		}
	   }
	   for (m=144;m<216;m++){
	   	if(m%24<11){
	     nameCharBuf[m]=nameCharBuf[m-144+12];
	   
		 }
	  }
	  for (m=0;m<72;m++){
	     nameCharBuf[m]=0;
	   }

	break;
	case 2:
	 
	   for (m=287;m>215;m--){
	     nameCharBuf[m]=nameCharBuf[m-144];
	   }
	   for (m=72;m<216;m++){
	     nameCharBuf[m]=0;
	   }
	break;
	case 3:
	   for (m=216;m<288;m++){
	     nameCharBuf[m]=nameCharBuf[m-72];
	   }
	   for (m=216;m>143;m--){
	     if(m%24<11){
	         nameCharBuf[m]=nameCharBuf[m-72+12];
		 }
		 else{
		    nameCharBuf[m]=0;
		 }
	   }
	   for (m=143;m>71;m--){
	      if(m%24>11){
	         nameCharBuf[m]=nameCharBuf[m-0-12];
		  }
		  else{
		  	nameCharBuf[m]=0;
		  }
	   
	   }
	   
	break;
	}
}
void LCD_PutName_24_cy(unsigned char cy,unsigned char  *s){
	uchar m,wordCount=0,charSize=0, *p=s,offsetLen=72;
	
	if(infoChanged)
	{
        infoChanged=0;
		while(*s!='\0'&&wordCount<4)
		{
			charSize = GB_HZZF_len(s);
			get24x24NameBytesFormGB2312s(s,72*wordCount);
			wordCount++;
			s++;
			if(charSize==2)
			{
			s++;
			}
		
		}
		arrangeNameArr(wordCount);

	}
	for(m=0;m<4;m++){
		lcd_disp_sz_char_24_original_x(16+m*24,cy,0,nameCharBuf+offsetLen*m);
	}

}
void getBytesFormASCIIs(char* chr){
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
   				chrBuf[i]=ReadByte();
				chrBuf[16+i]=0x00; 
   			}
		
		}
		CS=1;	
}
void lcd_disp_sz_SingleBytechar(uchar cy,uchar cx,uchar* chr)
{
	uchar *p,i,s,page;
	uchar port;

	getBytesFormASCIIs(chr);
	p=chrBuf;
	if(cx<8)
	{
		port=1;
		s=cx<<3;
	}
	else
	{
		port=2;
		s=((cx-8)<<3);
			
	}
	
	for(page=0;page<2;page++)
	{
		LCD_WrCmd(port,0xb8+cy*2+page);
		delayus(100);
		LCD_WrCmd(port,0x40+s);
		delayus(100);
		for(i=0;i<8;i++)
		{
			LCD_WrDat(port,*p);
			delayus(10);
			p++;
		}
	}
}
void LCD_PutSingleByteString(unsigned char cx,unsigned char cy,unsigned char  *s)
{
	uchar wordCount=0,charSize=0;
	
//	lcd_disp_sz_char(0,0,s);
//	lcd_disp_sz_char(4,1,s+2);
	while(*s!='\0'&&wordCount<10)
	{
		charSize = GB_HZZF_len(s); //charsizeֻ����Ϊһ
		lcd_disp_sz_SingleBytechar(cy,cx,s);
		wordCount++;
		s++;
		if(charSize==2)
		{
		s++;
		}
		cx++;
		if(cx>15) {cx=0;cy++;}
	}
 
}
uchar charArr[2];
void LCD_PutChar(unsigned char cx,unsigned char cy,unsigned char  chr){
	uchar *p,i,s,page;
	uchar port;
	chr=0;//����warning �ñ������ã���Ϊ����֮ǰ�ӿ�
	cy--;

	p=arrow16x16;
	if(cx<4)
	{
		port=1;
		s=cx<<4;
	}
	else
	{
		port=2;
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
void NullCGRAM()
{ 
     lbatStateGraph=null16x16;
}
void DisplayCGRAM(unsigned char cx,unsigned char cy)
{
	
	uchar *p,i,s,page;
	uchar port;
	cy--;
	
	p=lbatStateGraph;
	if(cx<4)
	{
		port=1;
		s=cx<<4;
	}
	else
	{
		port=2;
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
void resetUnderline(){
	oldUnderlineX=0;
	oldUnderlineY=0;
}
void DisplayUnderline(unsigned char cx,unsigned char cy,bit display)
{
	uchar *p,i,s,page;
	uchar port;
	cy--;
	if(display==0){
	//��ȥ�»���
	}
	if(oldUnderlineX!=0||oldUnderlineY!=0){	
		p=null16x16;
		if(oldUnderlineX<4)
		{
			port=1;
			s=oldUnderlineX<<4;
		}
		else
		{
			port=2;
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
		port=1;
		s=cx<<4;
	}
	else
	{
		port=2;
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

void LCD_PutPosition(unsigned char x,unsigned char y)
{ 
  DisplayUnderline(x,y+1,1);
 
}
void ClrScreen()
{ 
   LCD_DispFill(0x00);
}
