
#include <reg52.h>  
#include<intrins.h> 
#include "insideStorage.h" 
#include "hanzi.h"   
//#include "vvsimulspi.h"
#include "vvspi.h"

//�����ò���
//#define LED_UNITS_COUNT 4 //��Ԫ������
#define WORDS_PER_MESSAGE 16//ÿ�仰��ʾ������
#define LED_UNITS_WIDTH 64  //led����� (����)
#define LED_UNITS_HIGHT 16  //led���߶� (����)
#define MESSAGES_DISPLAY_BUF_LEN 600 //ram�б�������message��LED�������
#define TRANS_BUF_LEN 200   //���仺����
#define SENTENCE_BUF_LEN 150 //ram�� ����ÿ��ĵ������
#define CHARACTER_SIZE 16   //ÿ����ռ�õ����С

//���ý���
#define uchar unsigned char
#define uint unsigned int
#define  _Nop()  _nop_()
#define DATA_PRE 0x50 //0101 ����ǰ��






#define LOCAL_ADDRESS_ADDR  0x0

#define MESSAGE_NUM_ADDR  0x10
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


//����1 485��д����
sbit RW_485=P3^7;



#define SENTENCE_MAX_NUM 6
#define SENTENCE_LEN  32

uint transBufIndex=0,changeMessageTimer;
uchar xdata  messageArray[SENTENCE_MAX_NUM][SENTENCE_LEN+2]={'\0'};
uchar xdata messagesDisplayBuf[MESSAGES_DISPLAY_BUF_LEN];
uchar xdata transBuf[TRANS_BUF_LEN];
uchar xdata messageNum,messageInteval,tempMessageNum,curSentenceIndex,commState1;

bit inSetting,receiveMessagesFlag=0,changeMessageFlag=0,lcdAllowed,isCommAndProc=0;








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

int milSeconds,seconds;
bit isRefresh;
unsigned char timerCount=0,timerI,timerJ;

void Timer0_isr(void) interrupt 1 using 1
{
	//1����

	TH0=(65536-922)/256;	      
 	TL0=(65536-922)%256;
    //0.05����
//  TH0=(65536-46)/256;	      
//	TL0=(65536-46)%256;

	//0.025����
//	TH0=0xFF;
//	TL0=0xE9;
	if(changeMessageFlag==0){
		milSeconds++;
		if(milSeconds>=1000){
			milSeconds=0;
			seconds++;
		}
		if(seconds>=messageInteval){
			seconds=0;
			changeMessageFlag=1;
		}

	}
	if(isRefresh==0){
	 	isRefresh=1;
	}



}
void timerAction(){
	if(showMessageFlag==0||isCommAndProc==1){
		 return;
	}
	if(isRefresh==0){
		return;
	}
	for(timerJ=0;timerJ<16;timerJ++) 
	{
		InputByte(messagesDisplayBuf[timerJ*32+timerI]);
		InputByte(messagesDisplayBuf[timerJ*32+timerI+1]);
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
	//		transBufIndex=0;
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
				transBufIndex=0;
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
			transBuf[transBufIndex++]=SBUF;
			if(transBufIndex==2){
				tempMessageNum=(((transBuf[0]<<4)&0x0f)+transBuf[1]);

			}
			
			
			
			if(transBufIndex>=(tempMessageNum*SENTENCE_LEN+2)*2){
			
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
//  TH0=(65536-922)/256;	      
//	TL0=(65536-922)%256;
    //0.05����
  	TH0=(65536-46)/256;	  
    TL0=(65536-46)%256;
	//0.025����
//	TH0=0xFF;
//	TL0=0xE9;
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
			IapProgramByte(IAP_ADDRESS+MESSAGE_LIST_ADDR+m*SENTENCE_LEN+n, messageArray[m][n]);
		}
	}
}
void makeMessages(){
	uchar m,n,tempMessageInteval,tempBufIndex=2;//0,1�洢��Ϣ�����ѻ�ȡ
	void changeMessage(bit isFirst);
	
		messageNum=tempMessageNum;

		tempMessageInteval = (transBuf[tempBufIndex++] & 0x0f) << 4;
	
		tempMessageInteval+= (transBuf[tempBufIndex++]  & 0x0f);
	
		for (m = 0; m < messageNum; m++) {

			for (n = 0; n < SENTENCE_LEN; n++) {
				
				messageArray[m][n] = (transBuf[tempBufIndex++] & 0x0f) << 4;
	
				messageArray[m][n] += (transBuf[tempBufIndex++] & 0x0f);
			}

		}	
	 isCommAndProc=0;
	messageInteval=tempMessageInteval;
	storageBaseInfo();
//	displayCurrentLedMessage(1);
	changeMessageFlag=1;//��������յ������ݺ��������µ�������ʾ����
	changeMessage(1);
	
	
	

}
#define DEFAULT_MESSAGE_LEN 16
char data defaultMessage[]="��ӭ��ʹ�ÿ�Ƴ������ܹ���ϵͳ";
void retrieveBaseInfo(){
	uchar m,n;



	messageNum=IapReadByte(IAP_ADDRESS+MESSAGE_NUM_ADDR);
	if(messageNum==0xff){
		messageNum=1;
		messageInteval=10;
		for(n=0;n<DEFAULT_MESSAGE_LEN*2;n++){
			messageArray[0][n]=defaultMessage[n];
		}
	}
	else{
		messageInteval=IapReadByte(IAP_ADDRESS+MESSAGE_INTEVAL_ADDR);
		if(messageInteval==0xff){
			messageInteval=10;
		}

		for (m = 0; m < messageNum; m++) {
			for (n = 0; n < SENTENCE_LEN; n++) {
				messageArray[m][n]=IapReadByte(IAP_ADDRESS+MESSAGE_LIST_ADDR+m*SENTENCE_LEN+n);
			}
		}
	}
}



void getBytesFormGB2312s(char* charArr){
	uint byteIndex=0,charSeq=0,charSize=0;
	unsigned long temp_addr;
	uint i,m;
	for(m=0;m<MESSAGES_DISPLAY_BUF_LEN;m++)
	{
		messagesDisplayBuf[m]=0x0;
	}
	
	while(charSeq<WORDS_PER_MESSAGE&&charArr[byteIndex]!='\0')
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
   				messagesDisplayBuf[offsetLen*charSeq+i]=ReadByte(); 
   			}
		} else 	if(charSize==1){
	

		 	for(i=0;i<(offsetLen/2);i++){
				if(isCommAndProc==1){
	 				return;
				}
   				messagesDisplayBuf[offsetLen*charSeq+2*i]=ReadByte(); 
				messagesDisplayBuf[offsetLen*charSeq+2*i+1]=0x00; 
   			}
		
		}
		CS=1;
    	charSeq++;/*
		for(m=480;m<512;m++)
		{
			messagesDisplayBuf[m]=0x0;
		}*/
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
		getBytesFormGB2312s(messageArray[curSentenceIndex]);
		
	}
	showMessageFlag=1;
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
//		changeMessageTimer=0;//������displayCurrentLedMessage();֮�����
		getCurrentLedMessageBytes(isFirst);
//		displayCurrentLedMessage(0);
	}
}
void main()
{
	
	
 	P2M1=0x00;
	P2M0=0x0f;	
	retrieveBaseInfo();
	
	init();
	 

//	displayCurrentLedMessage(1);
		getCurrentLedMessageBytes(1);
 	while(1){
	  	timerAction();
	  	receiveMessage();
	 	changeMessage(0);		

	}
}

