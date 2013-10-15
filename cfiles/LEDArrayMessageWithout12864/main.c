
#include <reg52.h>  
#include<intrins.h> 
#include "insideStorage.h" 
#include "hanzi.h" 
    
#include "vvsimulspi.h"
//#include "vvspi.h"
#define FULL_SCREEN_HEIGHT 0 //16����ߵ���ֻʹ��T_I0 ������Ϊ��ȫ���ߣ�32����ߵ���ʹ��T_I0��T_IO2 ������Ϊ�ǰ����ߣ�������ʹ��ifndef�����ж�
#define HALF_SCREEN_HEIGHT 1
//�����ò���
#define LED_UNITS_COUNT 1  //��Ԫ������
#define WORDS_PER_MESSAGE 4//ÿ�仰��ʾ������
#define LED_UNITS_WIDTH 64  //led����� (����)
#define LED_UNITS_HIGHT 16  //led���߶� (����)
#define MESSAGE_BUF_LEN 600 //ram�б�������message��gb2312����
#define SENTENCE_BUF_LEN 150 //ram�� ����ÿ��ĵ������



#define CHARACTER_SIZE 16   //ÿ����ռ�õ����С

//���ý���
#define uchar unsigned char
#define uint unsigned int
#define  _Nop()  _nop_()
#define DATA_PRE 0x50 //0101 ����ǰ��






#define LOCAL_ADDRESS_ADDR  0x0

#define MESSAGE_NUM_ADDR  0x4
#define MESSAGE_INTEVAL_ADDR  0x6

#define MESSAGE_LIST_ADDR  0x100



#define FIELD_COUNT 3
#define STAFF_INFO_FIELD_LENGTN 8

#define CMD_UPDATE_STAFF 0xC0
#define CMD_REQ_UNIT_INFO 0x80
#define COMM_STATE_NOTHING 0
#define COMM_STATE_UPDATE_STAFF 1
#define COMM_STATE_REQ_UNIT_INFO 2
#define COMM_STATE_UPDATE_STAFF_WAIT_ADDRESS 3
#define COMM_STATE_REQ_UNIT_INFO_WAIT_ADDRESS 4
             
//ָʾ��

sbit LED_RXD1 =P3^5;
/*
//12864��ʾ��
#define DataPort P0 //12864��ʾ������ͨ��
sbit RS = P2 ^ 0;
sbit RW = P2 ^ 1;
sbit E  = P2^2;
sbit PSB = P2^3;
sbit RES = P2^4;
*/


//����1 485��д����
sbit RW_485=P3^7;



#define SENTENCE_MAX_NUM 6
#define SENTENCE_LEN  32

uint messagesBufIndex=0,changeMessageTimer;
uchar xdata  messageBuf[SENTENCE_MAX_NUM][SENTENCE_LEN+1]={'\0'};
uchar xdata messagesBuf[MESSAGE_BUF_LEN];
uchar messageNum,messageInteval,tempMessageNum,curSentenceIndex,commState1;

bit inSetting,receiveMessagesFlag=0,changeMessageFlag=0,lcdAllowed,isCommAndProc=0,halfOrFull=FULL_SCREEN_HEIGHT;


unsigned int speed,count=0,perCharSeq;





#define BYTE_LEN 8
unsigned char offsetCountPerRow=CHARACTER_SIZE/8,emptyCount=0;
unsigned char offsetLen=CHARACTER_SIZE*(CHARACTER_SIZE/8);
sfr P1M1=0x91;
sfr P1M0=0x92;
sfr P2M1=0x95;
sfr P2M0=0x96;

sbit T_CLK = P2^7;      //ʱ�� ��Ӧsk
sbit T_STR = P2^6;      //���� ��Ӧlt
sbit T_IO  = P2^5;       //����1
sbit T_IO2  = P2^4;       //����2
sbit EN=P3^6;

bit timeFlag,rowDisplayFinished=1,showMessageFlag=1;
unsigned char DIS[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
//unsigned char xdata hztest[SENTENCE_BUF_LEN];

unsigned char xdata hztest1[]=   
{
/*--  ����18;  �������¶�Ӧ�ĵ���Ϊ����x��=24x24   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x10,0x00,0x00,0x10,0x00,0x00,
0x10,0x00,0x10,0x10,0x18,0x1F,0xFF,0xF8,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x1F,0xFF,0xF0,0x10,0x10,0x10,0x00,0x10,0x10,
0x00,0x10,0x00,0x00,0x10,0x00,0x00,0x10,0x00,0x00,0x10,0x00,0x00,0x10,0x00,0x00,
0x10,0x00,0x00,0x10,0x00,0x00,0x00,0x00,

/*--  ����:  ��  --*/
/*--  ����18;  �������¶�Ӧ�ĵ���Ϊ����x��=24x24   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x18,0x1F,0xFF,0xF8,0x18,0x00,0x18,0x18,
0x00,0xD8,0x19,0xFF,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x19,
0x98,0x18,0xFE,0x18,0x18,0x1A,0x18,0x18,0x19,0x18,0x18,0x19,0x98,0x18,0x18,0x98,
0x18,0x18,0x58,0x1F,0xFF,0xF8,0x18,0x00,0x18,0x18,0x00,0x18,0x1F,0xFF,0xF8,0x18,
0x00,0x18,0x18,0x00,0x10,0x00,0x00,0x00,
};


/*
void DelayUs2x(unsigned char t) {
	while (--t)
		;
}

void delayMs(unsigned char t) {

	while (t--) {

		DelayUs2x(245);
		DelayUs2x(245);
	}
}
*/
/*
void Check_Busy()
{  
    RS=0;
    RW=1;
    E=1;
    DataPort=0xff;
    while((DataPort&0x80)==0x80);
    E=0;
}

void Write_Cmd(unsigned char Cmd)
{
	Check_Busy();

	RS=0;
	RW=0;
	E=1;
	DataPort=Cmd;
	DelayUs2x(5);
	E=0;
	DelayUs2x(5);
}

void Write_Data(unsigned char Data)
{
	Check_Busy();
	RS=1;
	RW=0;
	E=1;
	DataPort=Data;
	DelayUs2x(5);
	E=0;
	DelayUs2x(5);
}






void LCD_PutString(unsigned char x,unsigned char y,unsigned char  *s)
{ 
 switch(y)
     {
	  case 1: Write_Cmd(0x80+x);break;
	  case 2: Write_Cmd(0x90+x);break;
	  case 3: Write_Cmd(0x88+x);break;
	  case 4: Write_Cmd(0x98+x);break;
      default:break;
	 }
 while(*s>0)
   { 
      Write_Data(*s);
      s++;
      DelayUs2x(50);
   }
}
void LCD_PutChar(unsigned char x,unsigned char y,unsigned char c)
{ 
 switch(y)
     {
	  case 1: Write_Cmd(0x80+x);break;
	  case 2: Write_Cmd(0x90+x);break;
	  case 3: Write_Cmd(0x88+x);break;
	  case 4: Write_Cmd(0x98+x);break;
      default:break;
	 } 
      Write_Data(c);
    
      DelayUs2x(50);
 
}
void LCD_PutPosition(unsigned char x,unsigned char y)
{ 
 switch(y)
     {
	  case 1: Write_Cmd(0x80+x);break;
	  case 2: Write_Cmd(0x90+x);break;
	  case 3: Write_Cmd(0x88+x);break;
	  case 4: Write_Cmd(0x98+x);break;
      default:break;
	 } 
 
}

void ClrScreen()
{ 
   Write_Cmd(0x01);
   delayMs(15);
}
   	
*/

//�Զ��������ʽ��xxdd dddd  �{2λ��ʾ�����6λ��ʾ�Ƽܵ�ַ ���64λ 0xC0
//����Ա����Ϣ 1100 0000 �������� 1000 0000 
/*bit isUpdateInfoCmd() {
	return (SBUF&0xC0)==0xC0;
}
bit isReqUnitInfo() {
	return (SBUF&0xC0)==0x80;
}
*/
/*
void displayLEDMessage(uchar message[],uchar len){
	uchar m,indexi=0,indexj=0,count=0,isChinese=0,engliseAl=0,totalPrintCount=0;
	uchar xdata displayArray[4][10]={'\0'};
	if(lcdAllowed==0){
		return;
	}
	for(m=0;m<len;m++){
		if((isChinese==0)&&(message[m]&0x80)==0x80){
			if(engliseAl%2!=0){
				displayArray[indexi][indexj++]=' ';
				count++;
				totalPrintCount++;
				engliseAl=0;
				m--;//���¶�ȡ���ֽ�
			}
			else
			{
				displayArray[indexi][indexj++]=message[m];
				isChinese=1;
			}
		}
		else{
			if(isChinese==0){
				engliseAl++;
			}
			displayArray[indexi][indexj++]=message[m];
			count++;
			isChinese=0;
			totalPrintCount++;
		}
		if(totalPrintCount==len){
			displayArray[indexi][indexj]='\0';
		}
		if(count==4){
			displayArray[indexi][indexj]='\0';
			indexi++;
			indexj=0;
			count=0;
		}
		if(indexi>=4){ //�����ʾ4��
			break;
		}
		
	}
	for(m=0;m<4;m++){
		LCD_PutString(0,m+1,displayArray[m]);
	}
	
	
}
void displayCurrentLedMessage(bit isFirst){
		//����������
	if(inSetting==1||lcdAllowed==0){
		return;
	}
	
	ClrScreen(); 
	if(messageNum>0){
		if(isFirst==1){
			curSentenceIndex=0;
		}else{
			curSentenceIndex=(++curSentenceIndex)%messageNum;
		}
		displayLEDMessage(messageBuf[curSentenceIndex],SENTENCE_LEN);
	}

}
*/


/*void sendData(uchar c)
{	
	RW_485=1;
    SBUF = c;
	TI = 0;
    while(!TI); 
}
*/
void Input2Bytes(unsigned  char ucDa1,unsigned  char ucDa2) 
{ 
	unsigned  char i;
	if(isCommAndProc==1){
		return;
	}
	for(i=8; i>0; i--)
	{
 
	 	T_IO = !((ucDa1>>7)&0x01);
	  	T_IO2 = !((ucDa2>>7)&0x01);
  		T_CLK =0;                  
		T_CLK = 1;
 		ucDa1 = ucDa1 << 1; 
 		ucDa2 = ucDa2 << 1; 
	} 
}
void InputBytes(unsigned  char ucDa1) 
{ 
	unsigned  char i;
	if(isCommAndProc==1){
		return;
	}
	for(i=8; i>0; i--)
	{
 
	 	T_IO = !((ucDa1>>7)&0x01); 	
  		T_CLK =0;                  
		T_CLK = 1;
 		ucDa1 = ucDa1 << 1; 

	} 
}
void printRowBytes(unsigned char cIndex,unsigned char row){
	//void Input2Byte(unsigned  char ucDa1,unsigned  char ucDa2) 
	unsigned char m;

	unsigned int arrIndex=cIndex*offsetLen;
	for(m=0;m<offsetCountPerRow;m++){
		if(halfOrFull==HALF_SCREEN_HEIGHT)
		{
			if(row<((LED_UNITS_HIGHT-CHARACTER_SIZE)/2)){
				Input2Bytes(0,messagesBuf[arrIndex+offsetLen/2+offsetCountPerRow*row+m]);
			
			}else if(row>=(CHARACTER_SIZE/2)){
				Input2Bytes(messagesBuf[arrIndex+offsetCountPerRow*(row-(LED_UNITS_HIGHT-CHARACTER_SIZE)/2)+m],0);
			}
			else{
				Input2Bytes(messagesBuf[arrIndex+offsetCountPerRow*(row-(LED_UNITS_HIGHT-CHARACTER_SIZE)/2)+m],messagesBuf[arrIndex+offsetLen/2+offsetCountPerRow*row+m]);
			}
		}
		else
		{
			InputBytes(messagesBuf[arrIndex+offsetCountPerRow*row+m]);
		}
		
	}
}
void Timer0_isr(void) interrupt 1 using 1
{
	//1����

	TH0=(65536-918)/256;	      
 	TL0=(65536-918)%256;

	if(changeMessageFlag==0){
		changeMessageTimer++;
		if(changeMessageTimer==1000*messageInteval){
			changeMessageFlag=1;
		}
	}
	if(rowDisplayFinished==1)
	{
		rowDisplayFinished=0;
		timeFlag=1;
		
	}


}
void timerAction(){

 	unsigned char m;
	if(showMessageFlag==0||isCommAndProc==1){
	 return;
	}

 	for(m=0;m< WORDS_PER_MESSAGE;m++) 
  	{
		printRowBytes(m,count);

	}
	for(m=0;m<emptyCount;m++){
		Input2Bytes(0x0,0x0);
	} 


	
	T_STR=0; 
	T_STR=1;  
//	P1=(P1&0xf0)|DIS[count];
	P2=(P2&0xf0)|DIS[count];
//	P2=DIS[count]|0xf0;
		count++;
		
		
		if(halfOrFull==HALF_SCREEN_HEIGHT)
		{
			if(count>=(LED_UNITS_HIGHT/2))
 			{
				count=0;
			}
			perCharSeq+=offsetCountPerRow; 
	 		if(perCharSeq==(CHARACTER_SIZE*offsetCountPerRow/2))
 			{
				perCharSeq=0;
			}
     	}
		else	
		{
			if(count>=(LED_UNITS_HIGHT))
 			{
				count=0;
			}
			perCharSeq+=offsetCountPerRow;
			if(perCharSeq==(CHARACTER_SIZE*offsetCountPerRow))
 			{
				perCharSeq=0;
			} 
		}
}
#define COMM_STATE_BROADCAST_MESSAGES 4
#define COMM_STATE_BROADCAST_MESSAGES_PREPARE_STEP_1 5
#define COMM_STATE_BROADCAST_MESSAGES_PREPARE_STEP_2 6
#define COMM_STATE_BROADCAST_MESSAGES_PREPARE_STEP_3 7
#define COMM_STATE_BROADCAST_PREPARE_1 0x01
#define COMM_STATE_BROADCAST_PREPARE_2 0x03
#define COMM_STATE_BROADCAST_PREPARE_3 0x05
bit isBroadCast4Addr() {
	return SBUF==0xf0;
}
void ssio(void)
interrupt 4
{
          
	if(RI){
         RI=0;
		
	 	if(isBroadCast4Addr()){
			
			commState1=COMM_STATE_BROADCAST_MESSAGES_PREPARE_STEP_1;
	//		commState1=COMM_STATE_BROADCAST_MESSAGES;
	//		messagesBufIndex=0;
	//		tempMessageNum=0;
			return;
		}



		if((commState1==COMM_STATE_BROADCAST_MESSAGES_PREPARE_STEP_1)){
			if(SBUF==COMM_STATE_BROADCAST_PREPARE_1){
			
				commState1=COMM_STATE_BROADCAST_MESSAGES_PREPARE_STEP_2;
			}
			else{
				commState1=COMM_STATE_NOTHING;
			}
			return;
		}
		if((commState1==COMM_STATE_BROADCAST_MESSAGES_PREPARE_STEP_2)){
			if(SBUF==COMM_STATE_BROADCAST_PREPARE_2){
				commState1=COMM_STATE_BROADCAST_MESSAGES_PREPARE_STEP_3;
			}
			else{
				commState1=COMM_STATE_NOTHING;
			}
			return;
		}
		if((commState1==COMM_STATE_BROADCAST_MESSAGES_PREPARE_STEP_3)){
			if(SBUF==COMM_STATE_BROADCAST_PREPARE_3){
				commState1=COMM_STATE_BROADCAST_MESSAGES;
				messagesBufIndex=0;
				tempMessageNum=0;
				isCommAndProc=1;
			}
			else{
				commState1=COMM_STATE_NOTHING;
			}
			return;
	
		}
		if(commState1==COMM_STATE_BROADCAST_MESSAGES){
			LED_RXD1=~LED_RXD1;
			messagesBuf[messagesBufIndex++]=SBUF;
			if(messagesBufIndex==2){
				tempMessageNum=(((messagesBuf[0]<<4)&0x0f)+messagesBuf[1]);

			}
			
			
			
			if(messagesBufIndex>=(tempMessageNum*SENTENCE_LEN+2)*2){
			
		//		showMessageFlag=~showMessageFlag;
				LED_RXD1=1;//ָʾ����
				receiveMessagesFlag=1;
			}
		
		}

               
	}
	else if(TI){                       
		TI=0;
	}

}


void init() {
	EN=0; //�͵�ƽ��Ч
	RW_485=0; //485��ʼ���ڽ���״̬
  	
	//��ʱ��0��ʼ��
 	TMOD |= 0x01;	  		     
    TH0=(65536-918)/256;	      
	TL0=(65536-918)%256; 
	ET0=1;           
	TR0=1;  
  	
	//����1��ʼ��
    SCON  = 0x50;    
	TMOD |= 0x20;
	TH1 = 0xfd;
	TL1 = 0xfd;
	TR1 = 1;


	IP=0x10;//�趨����1Ϊ�{���ȼ�
	ES=1; //������1�ж�
			
	//�ⲿ�ж�1 ��ʼ��
  	EX1=1;         
    IT1=1;  

	EA = 1;
	//��ʼ��spi
	init_spi();

	lcdAllowed=0;
/*	if(lcdAllowed==1)	
	{  
	delayMs(40);           //����40MS����ʱ����
   	PSB=1;                 //����Ϊ8BIT���ڹ���ģʽ
   	delayMs(1);            //��ʱ
   	RES=0;                 //��λ
   	delayMs(1);            //��ʱ
   	RES=1;                 //��λ�ø�
   	delayMs(10);
	
   	Write_Cmd(0x30);       //ѡ�����ָ�
   	DelayUs2x(50);         //��ʱ����100us
   	Write_Cmd(0x30);       //ѡ��8bit������
   	DelayUs2x(20);         //��ʱ����37us
   	Write_Cmd(0x0c);       //����ʾ(���αꡢ������)
   	DelayUs2x(50);         //��ʱ����100us
   	Write_Cmd(0x01);       //�����ʾ�������趨��ַָ��Ϊ00H
   	delayMs(15);           //��ʱ����10ms
   	Write_Cmd(0x06);       //ָ�������ϵĶ�ȡ��д��ʱ���趨�α���ƶ�����ָ����ʾ����λ�������������1λ�ƶ�
   	DelayUs2x(50);         //��ʱ����100us
	}
*/
}
void storageBaseInfo(){
	uchar m,n;
	IapEraseSector(IAP_ADDRESS); 
	
    IapProgramByte(IAP_ADDRESS+MESSAGE_NUM_ADDR, messageNum);
	IapProgramByte(IAP_ADDRESS+MESSAGE_INTEVAL_ADDR, messageInteval);

    for (m = 0; m < messageNum; m++) {
		for (n = 0; n < SENTENCE_LEN; n++) {
			IapProgramByte(IAP_ADDRESS+MESSAGE_LIST_ADDR+m*SENTENCE_LEN+n, messageBuf[m][n]);
		}
	}
}
void makeMessages(){
	uchar m,n,tempMessageInteval,tempBufIndex=2;//0,1�洢��Ϣ�����ѻ�ȡ
	void changeMessage(bit isFirst);
	
		messageNum=tempMessageNum;

		tempMessageInteval = (messagesBuf[tempBufIndex++] & 0x0f) << 4;
	
		tempMessageInteval+= (messagesBuf[tempBufIndex++]  & 0x0f);
	
		for (m = 0; m < messageNum; m++) {

			for (n = 0; n < SENTENCE_LEN; n++) {
				
				messageBuf[m][n] = (messagesBuf[tempBufIndex++] & 0x0f) << 4;
	
				messageBuf[m][n] += (messagesBuf[tempBufIndex++] & 0x0f);
			}

		}	
	 isCommAndProc=0;
	messageInteval=tempMessageInteval;
//	displayCurrentLedMessage(1);
	
	storageBaseInfo();
	
	changeMessage(1);

}
#define DEFAULT_MESSAGE_LEN=30
char data defaultMessage[]="��ӭʹ�ÿ�Ƴ������ܹ���ϵͳ";
void retrieveBaseInfo(){
	uchar m,n;



	messageNum=IapReadByte(IAP_ADDRESS+MESSAGE_NUM_ADDR);
	if(messageNum==0xff){
		messageNum=1;
		messageInteval=10;
		for(n=0;n<DEFAULT_MESSAGE_LEN;n++){
			messageBuf[0][n]=defaultMessage[n];
		}
	}
	else{
		messageInteval=IapReadByte(IAP_ADDRESS+MESSAGE_INTEVAL_ADDR);
		if(messageInteval==0xff){
			messageInteval=10;
		}

		for (m = 0; m < messageNum; m++) {
			for (n = 0; n < SENTENCE_LEN; n++) {
				messageBuf[m][n]=IapReadByte(IAP_ADDRESS+MESSAGE_LIST_ADDR+m*SENTENCE_LEN+n);
			}
		}
	}
}



void getBytesFormGB2312s(char* charArr){
	uchar i=0, byteIndex=0,charSeq=0,charSize=0;
	unsigned long temp_addr;
	
	
	while(charSeq<WORDS_PER_MESSAGE)
	{
		temp_addr = GB_ADDR(charArr+byteIndex,CHARACTER_SIZE);
		charSize = GB_HZZF_len(charArr+byteIndex);
		byteIndex +=charSize;
		CS=0;
	/*	WriteByte(0x0b);
		WriteByte((temp_addr>>16)&0xff);
		WriteByte((temp_addr>>8)&0xff);
		WriteByte((temp_addr>>0)&0xff);
		WriteByte(0xFF);
	*/
		WriteByte(0x03);
		WriteByte((temp_addr>>16)&0xff);
		WriteByte((temp_addr>>8)&0xff);
		WriteByte((temp_addr>>0)&0xff);
		if(charSize==2){
		 	for(i=0;i<offsetLen;i++){
				if(isCommAndProc==1){
					 return;
				}
   				messagesBuf[offsetLen*charSeq+i]=ReadByte(); 
   			}
		} else 	if(charSize==1){
	

		 	for(i=0;i<(offsetLen/2);i++){
				if(isCommAndProc==1){
	 				return;
				}
   				messagesBuf[offsetLen*charSeq+2*i]=ReadByte(); 
				messagesBuf[offsetLen*charSeq+2*i+1]=0x00; 
   			}
		
		}
		CS=1;
    	charSeq++;
	}
 //������
/*	for(i=0;i<144;i++){
		hztest[i]=hztest1[i];
	}
*/
}

void getCurrentLedMessageBytes(bit isFirst){
		//����������	 	ch
	if(inSetting==1){
		return;
	}
	if(isCommAndProc==1){
		return;
	}
	showMessageFlag=0;

	if(messageNum>0){
		if(isFirst==1){
			curSentenceIndex=0;
		}else{
			curSentenceIndex=(++curSentenceIndex)%messageNum;
		}
		getBytesFormGB2312s(messageBuf[curSentenceIndex]);
	}
	showMessageFlag=1;
}
void refreshDisplay(){
	if(timeFlag){
  		timeFlag=0;
		timerAction();
		rowDisplayFinished=1;
   	}
}
void receiveMessage(){
	if(receiveMessagesFlag==1){
		receiveMessagesFlag=0;
		commState1=COMM_STATE_NOTHING;
		makeMessages();
	}
}
void changeMessage(bit isFirst){
	if(changeMessageFlag==1){
		changeMessageFlag=0;
		changeMessageTimer=0;//������displayCurrentLedMessage();֮�����
		getCurrentLedMessageBytes(isFirst);
//		displayCurrentLedMessage(0);
	}
}
void main()
{
	unsigned int ledTotalWidth=	LED_UNITS_WIDTH*LED_UNITS_COUNT;
	unsigned char emptyWidth=ledTotalWidth-CHARACTER_SIZE* WORDS_PER_MESSAGE;

	emptyCount=emptyWidth/BYTE_LEN;
	
 	P2M1=0x00;
	P2M0=0x0f;	
	retrieveBaseInfo();
	
	init();
	 

//	displayCurrentLedMessage(1);
		getCurrentLedMessageBytes(1);
 	while(1){
	  	refreshDisplay();
	  	receiveMessage();
	 	changeMessage(0);		

	}
}
