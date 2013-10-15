#include <reg52.h>
#include<intrins.h>
#include "insideStorage.h"

#define uchar unsigned char
#define uint unsigned int
#define DATA_PRE 0x50 //二进制0101作为数据前导
#define DATA_PRE_UPDATE_RACK_STAFF 0x60 //0110 更新员工信息前导
#define  _Nop()  _nop_()

sfr WDT_CONTR = 0xc1; //看门狗地址
sfr   AUXR    = 0x8e;                  
sfr   S2CON   = 0x9a;                
sfr   S2BUF   = 0x9b;                 
sfr   BRT     = 0x9c;
sfr IPH    = 0xB7;                  
sfr   IE2     = 0xaf;  //中断允许寄存器2 最低位为ES2            
#define   S2RI    0x01                 
#define   S2TI    0x02                
               


//12864显示屏
sbit RS = P2 ^ 0;
sbit RW = P2 ^ 1;
sbit E  = P2^2;
sbit PSB = P2^3;
sbit RES = P2^4;

//sbit SEL_INH=P4^4;
//sbit LED4=P4^5;
//指示灯
sbit LED_TXD1 =P1^0;
sbit LED_RXD1 =P1^1;
sbit LED_TXD2 =P3^3;
sbit LED_RXD2 =P3^4;
sbit k0=P3^5;
//串口1 485读写控制
sbit RW_485_1=P3^6;
sbit RW_485_2=P3^7;

/*sbit k1=P1^4;
sbit k3=P1^5;
sbit k5=P1^6;
sbit k7=P1^7;
*/
sbit SEL_A=P1^4;
sbit SEL_B=P1^5;
sbit SEL_C=P1^6;
sbit SEL_D=P1^7;
sbit LED_TEST=P2^7;
sbit LED_TEST2=P2^7;
#define DataPort P0 


#define RECEIVE 0x01
#define SEND 0x02
#define SENTENCE_MAX_NUM 6
#define SENTENCE_LEN  32

#define CMD_UPDATE_STAFF_INFO 0xC0
#define CMD_REQ_UNIT_INFO 0x80
#define CMD_BRORDCAST 0xf0
#define CMD_SEND_STA 0x80
#define SYM_DATA_END 0xff

#define COMM_STATE_NOTHING 0
#define COMM_STATE_REQ_UNIT_INFO 1
#define COMM_STATE_REQ_UNIT_INFO_RECEIVE 2
#define COMM_STATE_UPDATE_STAFF 3
#define COMM_STATE_BROADCAST_MESSAGES 4
#define COMM_STATE_BROADCAST_MESSAGES_PREPARE_STEP_1 5
#define COMM_STATE_BROADCAST_MESSAGES_PREPARE_STEP_2 6
#define COMM_STATE_BROADCAST_MESSAGES_PREPARE_STEP_3 7
#define COMM_STATE_REQ_RACK_UNITS_INFO 8
#define COMM_STATE_UPDATE_RACK_STAFF 9
#define COMM_STATE_SEND_STA 10

#define COMM_STATE_BROADCAST_PREPARE_1 0x01
#define COMM_STATE_BROADCAST_PREPARE_2 0x03
#define COMM_STATE_BROADCAST_PREPARE_3 0x05

#define DISPLAY_MESSAGE 0
#define DISPLAY_STATISTICS 1

unsigned char localAddress,unitsNum, curSentenceIndex,changeMessageTimer=0,currentUnitIndex=1,currentUnitTotalIndex,commState1,commState2,reqUnitInfoInteval,oldCommState=COMM_STATE_NOTHING,staffBufIndex=0,oldTH0=0,oldTL0=0,timeoutThreshold=0,oldGroup=255,retryCount=0,reqStateTimeout=0;
uint messagesBufIndex=0,reqStateTimer=0;
bit ack,changeMessageFlag=0,readStateFlag=0,receiveMessagesFlag=0,updateRackStaffFlag=0,inSetting=0,inAddrSetting=0,isInfraredAllowed=0,totalRefreshed=0,showMode;
uchar xdata unitStateArr[101],tempUnitStateArr[101];


#define LOCAL_ADDRESS_ADDR  0x0
#define UNITS_NUM_ADDR  0x2
#define MESSAGE_NUM_ADDR  0x4
#define MESSAGE_INTEVAL_ADDR  0x6
#define TIMEOUT_THRESHOLD_ADDR  0x8
#define MESSAGE_LIST_ADDR  0x100

#define UNIT_STATE_NULL  0
#define UNIT_STATE_EMPTY  1
#define UNIT_STATE_CHARGING 2
#define UNIT_STATE_FULL  3
#define UNIT_STATE_ERROR  4

uchar xdata stateStat[6]={0};//索引0未用（巡检总数） ，1 表示矿灯取走 2表示矿灯充电 3表示矿灯充满 5表示断线

uchar xdata messageBuf[SENTENCE_MAX_NUM][SENTENCE_LEN+1]={'\0'};
uchar messageInteval,messageNum=0,tempMessageNum=0;
uchar* xdata menu[]={"地址设置","灯架设置","红外设置","采集间隔"};
#define MENU_LEN 4
bit tready,tready2,rready,rready2;
bit isSelected;

#define FIELD_COUNT 3
#define STAFF_INFO_FIELD_LENGTN 8
#define MESSAGE_BUF_LEN 400
uchar xdata staff_buf[FIELD_COUNT][STAFF_INFO_FIELD_LENGTN];
uchar xdata messagesBuf[MESSAGE_BUF_LEN]={'\0'};
uchar xdata staffBuf[60];

uchar queryStateInteval=5,queryUnitsNum=20,groupNum=0,groupIndex;
void displayCurrentLedMessage(bit isFirst);
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


/*
//串口2输出数据：
void OutByteS2(uchar k)
{
	S2BUF=k;
	while(!(S2CON&S2TI));
	S2CON&=~S2TI;
}
//串口2读取数据
uchar GetByteS2(void)
{
	while(!(S2CON&S2RI));
	S2CON&=~S2RI;
	return S2BUF;			
}
uchar GetByteS2Instance(void)
{	
	delayMs(50);
	if(S2CON&S2RI){
		S2CON&=~S2RI;
		return S2BUF;
	}
	else{
		return 4;//表示无响应
	}	
}*/
void switchIfNecessary(uchar lampNo){
	uchar group=(lampNo-1)/20;//每二十个一组 cd4067
//	if(group==oldGroup){
//		return;
//	}
	oldGroup=group;

	SEL_A=(group&0x01);
	SEL_B=((group>>1)&0x01);
	SEL_C=((group>>2)&0x01);
	SEL_D=((group>>3)&0x01);

	
}
void sendCmdAndAddrInstance(uchar cmd,uchar addr){
//	switchIfNecessary(addr);
	RW_485_2=1;

	
	S2BUF=cmd;
	while(!(S2CON&S2TI));
	S2CON&=~S2TI;

	S2BUF=addr;

	while(!(S2CON&S2TI));
	S2CON&=~S2TI;
	
	RW_485_2=0;
}

void sendDataInstance(uchar ldata,uchar pre){
	LED_TXD2=~LED_TXD2;
	RW_485_2=1;
		
	S2BUF=((ldata>>4)&0x0f)|pre;
	while(!(S2CON&S2TI));
	S2CON&=~S2TI;

	S2BUF=(ldata&0x0f)|pre;
	while(!(S2CON&S2TI));
	S2CON&=~S2TI;
	
	RW_485_2=0;
	
	
}
sendDataWithoutSplitInstance(uchar ldata){
		LED_TXD2=~LED_TXD2;
	RW_485_2=1;
		
	S2BUF=ldata;
	while(!(S2CON&S2TI));
	S2CON&=~S2TI;

	
	RW_485_2=0;
}
void sendCmdInstance(uchar ldata){
	sendDataWithoutSplitInstance(ldata);	
}
/*
void sendData(uchar ldata){
	RW_485_2=1;
	S2BUF=ldata;
}*/
/*
void sendCmdAndAddr(uchar cmd,uchar addr){
	uchar temp;


	RW_485_2=1;
//	temp=cmd;
	S2BUF=temp;
//	S2CON&=~S2TI;
	while(!(S2CON&S2TI));

	RW_485_2=1;
	temp=addr;
	if(temp==addr&&(addr==3)){
			LED_RXD1=~LED_RXD1;
		}
		if(temp==3){
			LED_TXD1=~LED_TXD1;
		}
	S2BUF=addr;

//	S2CON&=~S2TI;
	while(!(S2CON&S2TI));
	RW_485_2=0;
}
void sendData(uchar ldata){
	
	RW_485_2=1;
		
	S2BUF=(ldata>>4)&0x0f;
	S2CON&=~S2TI;
	while(!(S2CON&S2TI));
	S2BUF=ldata&0x0f;
	S2CON&=~S2TI;
	while(!(S2CON&S2TI));
	RW_485_2=0;
	
	
}*/
void reqCurrentUnitInfo(){
	/*	TR0=0;
		TH0=(65536-45872)/256;	      
 		TL0=(65536-45872)%256;
		TR0=1;*/
	if(commState2==COMM_STATE_UPDATE_STAFF){
		return;
	}
	if(currentUnitIndex<=queryUnitsNum){
		LED_TXD2=0;
		currentUnitTotalIndex=groupIndex*queryUnitsNum+currentUnitIndex;
		TR0=0;

		IE2 = 0x00;
		switchIfNecessary(currentUnitTotalIndex);
		sendCmdAndAddrInstance(CMD_REQ_UNIT_INFO,currentUnitTotalIndex);
	




		IE2 = 0x01;
		if(isInfraredAllowed==0){
			TH0=(65536-45872)/256;	      
 			TL0=(65536-45872)%256;
			
		}
		reqUnitInfoInteval=0;

		TR0=1;
	

	}
	else{
	//	readStateFlag=0;
		groupIndex++;
		if(groupIndex==groupNum){
			groupIndex=0;
		//	totalRefreshed=1;

		}
		//每结束20个就更新下充电状态
		totalRefreshed=1;
		commState2=COMM_STATE_NOTHING;
		currentUnitIndex=1;
		LED_TXD2=1;
		LED_RXD2=1;
		TR0=1;
	
	}
}
void reqNextUnitInfo(){
	currentUnitIndex++;
	reqCurrentUnitInfo();
}

void S2INT() interrupt 8
{
	uchar temp;		
	if(S2CON&S2RI)
	{	

		S2CON&=~S2RI;
		temp=S2BUF;
	
		if(commState2==COMM_STATE_REQ_UNIT_INFO)
		{
		//	LED_RXD2=~LED_RXD2; 
			LED_RXD2=0; //接收到数据改为常量，不闪烁
			if((temp&0xf0)!=DATA_PRE){
				if(retryCount<10)//最多尝试10次
				{
					retryCount++;
					reqCurrentUnitInfo();
					
				}
				else{
					//当前充电架状态保持不变，请求下一个状态
					retryCount=0;
					reqNextUnitInfo();
				}
			}
			else{
			reqStateTimeout=0;
			tempUnitStateArr[currentUnitTotalIndex]=(temp&0x0f);
						
			reqNextUnitInfo();
		   }
		}
		
	}
	if(S2CON&S2TI)
	{
		
/*		if(commState2==COMM_STATE_REQ_UNIT_INFO)
		{
			
			
		}
		else if(commState2==COMM_STATE_UPDATE_STAFF)
		{
		
		}
*/	
		S2CON&=~S2TI;
		RW_485_2=0;

	}

}



void makeInfo() {
	uchar m;
	unitStateArr[0]=localAddress;
	for (m = 1; m <= unitsNum; m++) {
	//	unitStateArr[m] = (unitStateArr[m]+1)%4;
		unitStateArr[m] = 0;
	}
}

//自定义命令格式：xxxd dddd  髙3位表示命令，后5位表示灯架地址 最多32位 0e0
// 接收命令：广播1000 0000 ；获取充电状态1010 00000；更新员工信息1100 0000；通信正常结束 1110 0000
//发送命令： 获取充电状态1000 0000 更新员工信息1100 0000
bit isBroadCast4Addr() {
	return (SBUF&0xe0)==0x80;
}
bit isReqUnitInfo() {
	return (SBUF&0xe0)==0xA0;
}

bit isUpdateStaffInfo() {
	return (SBUF&0xe0)==0xC0;
}



void sendUnitInfo(){
	uchar m;
	if(tready==0){ //发送忙碌
		RW_485_1=1;
		SBUF=0x80;
		while(TI!=1);
		TI=0; 
		RW_485_1=0;
	}
	else{
		tready=0 ;
	//	makeInfo();
	//	reqUnitInfo();
		{
			//发送地址,不加数据前缀
			RW_485_1=1;
			SBUF=(unitStateArr[0]);	
	    	while(TI!=1);
		    TI=0; 
			RW_485_1=0;
		}
	 	for(m=1;m<=unitsNum;m++) 
	 	{
			LED_TXD1=~LED_TXD1;
			RW_485_1=1;
			SBUF=(unitStateArr[m]|DATA_PRE);	
	    	while(TI!=1);
		    TI=0; 
			RW_485_1=0;
	 	}
		tready=1;
		LED_TXD1=1; //指示灯灭
	}
}
void prepareUpdateStaffInfo(void){
	RW_485_2=1 ;
	oldCommState=commState2;
	commState2=COMM_STATE_UPDATE_STAFF;	
}
void endUpdateStaffInfo(void){
	commState2=oldCommState;
	if(commState2==COMM_STATE_REQ_UNIT_INFO){
		reqCurrentUnitInfo();
	}
}

void makeUpdateStaff(){
	
	uchar m,n,lampNo,tempStaffBufIndex=0;

	

				lampNo = (staffBuf[tempStaffBufIndex++] & 0x0f) << 4;
				lampNo += (staffBuf[tempStaffBufIndex++] & 0x0f);
	
		for (m = 0; m < FIELD_COUNT; m++) {

			for (n = 0; n < STAFF_INFO_FIELD_LENGTN; n++) {

				staff_buf[m][n] = (staffBuf[tempStaffBufIndex++] & 0x0f) << 4;
				staff_buf[m][n] += (staffBuf[tempStaffBufIndex++] & 0x0f);
			}

		}	
		
		prepareUpdateStaffInfo();
	//	LED_TXD1=0;
		switchIfNecessary(lampNo);
		sendCmdAndAddrInstance(CMD_UPDATE_STAFF_INFO,lampNo);


		
		for (m = 0; m < FIELD_COUNT; m++) {

			for (n = 0; n < STAFF_INFO_FIELD_LENGTN; n++) {
				sendDataInstance(staff_buf[m][n],DATA_PRE_UPDATE_RACK_STAFF);
				
			}

		}
			
		
		endUpdateStaffInfo();
	//	LED_TEST=~LED_TEST;


}
void storageBaseInfo(){
	uchar m,n;
	IapEraseSector(IAP_ADDRESS); 
	IapProgramByte(IAP_ADDRESS+LOCAL_ADDRESS_ADDR, localAddress); 
	IapProgramByte(IAP_ADDRESS+UNITS_NUM_ADDR, unitsNum); 
    IapProgramByte(IAP_ADDRESS+MESSAGE_NUM_ADDR, messageNum);
	IapProgramByte(IAP_ADDRESS+MESSAGE_INTEVAL_ADDR, messageInteval);
	IapProgramByte(IAP_ADDRESS+TIMEOUT_THRESHOLD_ADDR, timeoutThreshold);
    for (m = 0; m < messageNum; m++) {
		for (n = 0; n < SENTENCE_LEN; n++) {
			IapProgramByte(IAP_ADDRESS+MESSAGE_LIST_ADDR+m*SENTENCE_LEN+n, messageBuf[m][n]);
		}
	}
}
void prepareSendMessage(void){
//	RW_485_2=1 ;
	oldCommState=commState2;
	commState2=COMM_STATE_BROADCAST_MESSAGES;	
}
void endSendMessage(void){
	commState2=oldCommState;
	if(commState2==COMM_STATE_REQ_UNIT_INFO){
		reqCurrentUnitInfo();
	}
}
void sendLedMessages(){
	uint m=0,n=0;
	TR0=0;
	IE2 = 0x00;
	ES=0; 				
	switchIfNecessary(101);//文字广播在第10组 //20个一组后为第5组 索引从0开始

	sendCmdInstance(0xf0);
	sendDataWithoutSplitInstance(COMM_STATE_BROADCAST_PREPARE_1);
	sendDataWithoutSplitInstance(COMM_STATE_BROADCAST_PREPARE_2);
	sendDataWithoutSplitInstance(COMM_STATE_BROADCAST_PREPARE_3);
	for (m = 0; m < MESSAGE_BUF_LEN; m++) {
		sendDataWithoutSplitInstance(messagesBuf[m]);
	}

	LED_TXD2=1;//指示灯灭
	TR0=1;
	IE2 = 0x01;
	ES=1; 

}
void makeMessages(){
	uchar m,n,tempMessageInteval,tempBufIndex=2;//0,1存储消息数，已获取
	
	
		messageNum=tempMessageNum;

		tempMessageInteval = (messagesBuf[tempBufIndex++] & 0x0f) << 4;
	
		tempMessageInteval+= (messagesBuf[tempBufIndex++]  & 0x0f);
	
		for (m = 0; m < messageNum; m++) {

			for (n = 0; n < SENTENCE_LEN; n++) {
				
				messageBuf[m][n] = (messagesBuf[tempBufIndex++] & 0x0f) << 4;
	
				messageBuf[m][n] += (messagesBuf[tempBufIndex++] & 0x0f);
			}

		}	
	messageInteval=tempMessageInteval;
    if(showMode==DISPLAY_MESSAGE)
	{
		displayCurrentLedMessage(1);
	}
	storageBaseInfo();
	prepareSendMessage();
	sendLedMessages();
	endSendMessage();
}


void ssio(void)
interrupt 4
{
          
	if(RI){
         RI=0;

	 	if(isBroadCast4Addr()){
			commState1=COMM_STATE_BROADCAST_MESSAGES_PREPARE_STEP_1;
			return;
		}

	 	if(isReqUnitInfo()&&((SBUF&0x1F)==localAddress))
		{
			commState1=COMM_STATE_REQ_RACK_UNITS_INFO;

			TR0=0;
			ES=0; 
	//		P1=255-4;
	//		SBUF=1;
	//		while(TI!=1);
	//	    	TI=0; 
	//		
			sendUnitInfo();
			commState1=COMM_STATE_NOTHING;
			
			TR0=1;
			ES=1; 
			return;
		}
	 	if(isUpdateStaffInfo()&&((SBUF&0x1F)==localAddress)){
			commState1=COMM_STATE_UPDATE_RACK_STAFF;
			LED_TEST2=~LED_TEST2;
			staffBufIndex=0;
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
				LED_RXD1=1;//指示灯灭
				receiveMessagesFlag=1;
			}
		
		}else if(commState1==COMM_STATE_UPDATE_RACK_STAFF){
			if((SBUF&0xf0)!=DATA_PRE_UPDATE_RACK_STAFF){
				commState1=COMM_STATE_NOTHING;
				return;
			}
			staffBuf[staffBufIndex++]=SBUF;
			
			if(staffBufIndex>=(FIELD_COUNT*STAFF_INFO_FIELD_LENGTN+1)*2){
				LED_TEST=~LED_TEST;
				TR0=0;
				IE2 = 0x00;
				ES=0; 		
				makeUpdateStaff();
				commState1=COMM_STATE_NOTHING;
				LED_TXD2=1;//指示灯灭
				TR0=1;
				IE2 = 0x01;
				ES=1; 
			}
		}

               
	}
	else if(TI){                       
		TI=0;
	}

}

//LCD液晶屏函数开始

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
//LCD液晶屏函数 结束
void showAddress(uchar x,uchar y){
	uchar localAddressArr[3];
	localAddress=localAddress%100;
	localAddressArr[0]='0'+(localAddress/10);
	localAddressArr[1]='0'+(localAddress%10);
	localAddressArr[2]='\0';
	LCD_PutString(x,y,localAddressArr);
}
char * itostr(uchar num,char * str, uchar strLen){
	uchar m;
 	for(m=0;m<strLen;m++){
		str[m]='0';
 	}
	while (num!=0){
	    str[--m]='0'+(num%10);
		num/=10;
	}
	str[strLen]='\0';
 	return str;
}
char  xdata emptyStat[4]={'\0'};
char  xdata chargingStat[4]={'\0'};
char  xdata fullStat[4]={'\0'};
void displayStatisticsMessage(){
    if(inSetting==1)
		return;
	

	LCD_PutString(0,1,"下井：");
	LCD_PutString(0,2,"充电：");
	LCD_PutString(0,3,"充满：");
	LCD_PutString(4,1,itostr(stateStat[1],emptyStat,3));
	LCD_PutString(4,2,itostr(stateStat[2],chargingStat,3));
	LCD_PutString(4,3,itostr(stateStat[3],fullStat,3));
//  LCD_PutString(0,1,itostr(stateStat[1],emptyStat,3));
//	LCD_PutString(0,2,itostr(stateStat[2],chargingStat,3));
//	LCD_PutString(0,3,itostr(stateStat[3],fullStat,3)); 
//	LCD_PutString(4,1,itostr(stateStat[5],chargingStat,3));
//	LCD_PutString(4,2,itostr(stateStat[0],fullStat,3));
}
void displayLEDMessage(uchar message[],uchar len){
	uchar m,indexi=0,indexj=0,count=0,isChinese=0,engliseAl=0,totalPrintCount=0;
	uchar xdata displayArray[4][10]={'\0'};

	for(m=0;m<len;m++){
		if((isChinese==0)&&(message[m]&0x80)==0x80){
			if(engliseAl%2!=0){
				displayArray[indexi][indexj++]=' ';
				count++;
				totalPrintCount++;
				engliseAl=0;
				m--;//重新读取该字节
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
		if(indexi>=4){ //最多显示4行
			break;
		}
		
	}
	for(m=0;m<4;m++){
		LCD_PutString(0,m+1,displayArray[m]);
	}
	/*		//调试
			{
			
				uchar firstState[4]={'\0'};
				firstState[0]='0'+(unitStateArr[1]/100);
				firstState[1]='0'+((unitStateArr[1]%100)/10);
				firstState[2]='0'+(unitStateArr[1]%10);
				firstState[3]='\0';
				LCD_PutString(0,4,firstState);
			}
	
	*/
}

void displayCurrentLedMessage(bit isFirst){
		//功能设置中
	if(inSetting==1){
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
	LCD_PutString(5,4,"灯架");
	showAddress(7,4);
}

#define MENU_STATE_NOP 0
#define MENU_STATE_IN_ADDR_SETTING 1
#define MENU_STATE_IN_ADDR_BIT_SETTING 2
#define MENU_STATE_IN_ADDR_SETTING_FINISHED 3
#define MENU_STATE_IN_UNITS_NUM_SETTING 4
#define MENU_STATE_IN_UNITS_NUM_BIT_SETTING 5
#define MENU_STATE_IN_UNITS_NUM_SETTING_FINISHED 6
#define MENU_STATE_IN_TIMEOUT_SETTING 7
#define MENU_STATE_IN_TIMEOUT_BIT_SETTING 8
#define MENU_STATE_IN_TIMEOUT_SETTING_FINISHED 9
uchar menuState=MENU_STATE_NOP;
uchar selAddrBit=0;
uchar selUnitsNumBit=0;
uchar selTimeoutBit=0;
uchar menuIndex=0;
uchar xdata tempAddressArr[5];
uchar xdata tempUnitsNumArr[7];
uchar xdata tempTimeoutArr[3];
void initTempAddressArr(){
	localAddress=localAddress%100;
	tempAddressArr[0]=' ';
	tempAddressArr[1]='0'+(localAddress/10);
	tempAddressArr[2]=' ';
	tempAddressArr[3]='0'+(localAddress%10);
	tempAddressArr[4]='\0';
}
void initTempUnitsNumArr(){
	tempUnitsNumArr[0]=' ';
	tempUnitsNumArr[1]='0'+(unitsNum/100);
	tempUnitsNumArr[2]=' ';
	tempUnitsNumArr[3]='0'+((unitsNum%100)/10);
	tempUnitsNumArr[4]=' ';
	tempUnitsNumArr[5]='0'+(unitsNum%10);
	tempUnitsNumArr[6]='\0';
}
void initTimeoutArr(){
	tempTimeoutArr[0]=' ';
	tempTimeoutArr[1]='0'+(timeoutThreshold%10);
	tempTimeoutArr[2]='\0';
}
void setInfraredFlag(){
	isInfraredAllowed=1;
	//外部中断0 初始化
	IT0 = 1;   
 	EX0 = 1;
	TR0=0;
	oldTH0=TH0;
	oldTL0=TL0;
	TH0=0xFF; 
  	TL0=0x00;
 	TR0=1;
}
void clearInfraredFlag(){
	isInfraredAllowed=0;
	EX0=0;
	TR0=0;
	//暂且不从oldTH0和oldTL0恢复
	TH0=(65536-45872)/256;	      
 	TL0=(65536-45872)%256;
 	TR0=1;
}
void clearInfraredFlagIfNeccesary(){
	if(isInfraredAllowed==1){
	clearInfraredFlag();
	}
}
void showMenuItem(uchar index,bit selected){
	ClrScreen();
	if(isInfraredAllowed){
		LCD_PutString(0,1,"红外");
	}
	LCD_PutString(3,1,"设置");
	LCD_PutString(2,2,menu[index]);
	if(selected){
		LCD_PutChar(1,2,26);//输出"->"
	}
}
void showMenu(){
	showMenuItem(menuIndex,1);
}

void menuItemSeleted(uchar index){
	switch(index){
	case 0: //设置地址
		menuState=MENU_STATE_IN_ADDR_SETTING;
		selAddrBit=0;
		showMenuItem(menuIndex,0);
		initTempAddressArr();
		LCD_PutString(3,3,tempAddressArr);
		LCD_PutChar(2,3,26);//输出"->"
		LCD_PutPosition(3+selAddrBit,3);
		Write_Cmd(0x0f);
		break;
	case 1: //设置灯架数
		menuState=MENU_STATE_IN_UNITS_NUM_SETTING;
		selUnitsNumBit=0;
		showMenuItem(menuIndex,0);
		initTempUnitsNumArr();
		LCD_PutString(3,3,tempUnitsNumArr);
		LCD_PutChar(2,3,26);//输出"->"
		LCD_PutPosition(3+selUnitsNumBit,3);
		Write_Cmd(0x0f);
		break;
	case 2: //允许红外设置
		menuState=MENU_STATE_NOP;
		isInfraredAllowed=~isInfraredAllowed;
		if(isInfraredAllowed==1)
		{
			setInfraredFlag();
		}else{
			clearInfraredFlag();
		}
		showMenu();
		break;
	case 3: //设置灯架数
		menuState=MENU_STATE_IN_TIMEOUT_SETTING;
		selTimeoutBit=0;
		showMenuItem(menuIndex,0);
		initTimeoutArr();
		LCD_PutString(3,3,tempTimeoutArr);
		LCD_PutChar(2,3,26);//输出"->"
		LCD_PutPosition(3+selTimeoutBit,3);
		Write_Cmd(0x0f);
		break;
	  default:
	  	break;
	}
}

void keyScan(){
	if(k0==0)
	{
		delayMs(10);
		if(k0==0)
		{
			while(!k0);
			if(inSetting==0){
				inSetting=1;
				setInfraredFlag();
				menuState=MENU_STATE_NOP;
				menuIndex=0;
				showMenu();
			}
			else {
				menuState=MENU_STATE_NOP;
				clearInfraredFlagIfNeccesary();
				inSetting=0;
				ClrScreen();//强行清屏	
				displayCurrentLedMessage(0);
				
			}
		}
	}
/*	if(k1==0)
	{
		delayMs(10);
		if(k1==0)
		{	
			while(!k1);	
			if(inSetting==0){
				inSetting=1;
				menuState=MENU_STATE_NOP;
				menuIndex=0;
				showMenu();
			}
			else{
				if(menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
					menuState=MENU_STATE_IN_ADDR_SETTING;
				}
				else if(menuState==MENU_STATE_IN_ADDR_SETTING){
					menuState=MENU_STATE_NOP;
					
					Write_Cmd(0x0C);//关闭游标及反白
					
				 	showMenu();
				}
				else if(menuState==MENU_STATE_IN_ADDR_SETTING_FINISHED){
					
					menuItemSeleted(menuIndex);
				}

				else if(menuState==MENU_STATE_IN_UNITS_NUM_BIT_SETTING){
					menuState=MENU_STATE_IN_UNITS_NUM_SETTING;
				}
				else if(menuState==MENU_STATE_IN_UNITS_NUM_SETTING){
					menuState=MENU_STATE_NOP;
					
					Write_Cmd(0x0C);//关闭游标及反白
					
				 	showMenu();
				}
				else if(menuState==MENU_STATE_IN_UNITS_NUM_SETTING_FINISHED){
					
					menuItemSeleted(menuIndex);
				}
				else if(menuState==MENU_STATE_IN_TIMEOUT_SETTING){
					menuState=MENU_STATE_NOP;
					
					Write_Cmd(0x0C);//关闭游标及反白
					
				 	showMenu();
				}
				else if(menuState==MENU_STATE_IN_TIMEOUT_SETTING_FINISHED){
					
					menuItemSeleted(menuIndex);
				}
				else{
				
					clearInfraredFlagIfNeccesary();
					inSetting=0;
					ClrScreen();//强行清屏	
					displayCurrentLedMessage(0);
				}
			}
			
			
		}
	}
	if(k3==0)
	{
		delayMs(10);
		if(k3==0)
		{
			while(!k3);
			if(menuState==MENU_STATE_NOP){
				menuItemSeleted(menuIndex);
			}
			else if(menuState==MENU_STATE_IN_ADDR_SETTING){
				menuState=MENU_STATE_IN_ADDR_BIT_SETTING;
			}
			else if(menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
				localAddress=(tempAddressArr[1]-'0')*10+(tempAddressArr[3]-'0');
				storageBaseInfo();
				Write_Cmd(0x0C);//关闭游标及反白
				LCD_PutString(2,4,"设置完毕!");
				menuState=MENU_STATE_IN_ADDR_SETTING_FINISHED;
					
			}
			else if(menuState==MENU_STATE_IN_UNITS_NUM_SETTING){
				menuState=MENU_STATE_IN_UNITS_NUM_BIT_SETTING;
			}
			else if(menuState==MENU_STATE_IN_UNITS_NUM_BIT_SETTING){
				unitsNum=(tempUnitsNumArr[1]-'0')*100+(tempUnitsNumArr[3]-'0')*10+(tempUnitsNumArr[5]-'0');
				storageBaseInfo();
				Write_Cmd(0x0C);//关闭游标及反白
				LCD_PutString(2,4,"设置完毕!");
				menuState=MENU_STATE_IN_UNITS_NUM_SETTING_FINISHED;
					
			}
			else if(menuState==MENU_STATE_IN_TIMEOUT_SETTING){
				menuState=MENU_STATE_IN_TIMEOUT_BIT_SETTING;
			}
			else if(menuState==MENU_STATE_IN_TIMEOUT_BIT_SETTING){
				timeoutThreshold=(tempTimeoutArr[1]);
				storageBaseInfo();
				Write_Cmd(0x0C);//关闭游标及反白
				LCD_PutString(2,4,"设置完毕!");
				menuState=MENU_STATE_IN_TIMEOUT_SETTING_FINISHED;
					
			}			
		}
	}
	if(k5==0)
	{
		delayMs(10);
		if(k5==0)
		{
			while(!k5);	
			if(inSetting){
				if(menuState==MENU_STATE_NOP){
					menuIndex=(menuIndex-1+MENU_LEN)%MENU_LEN;
					showMenuItem(menuIndex,1);
				}else if(menuState==MENU_STATE_IN_ADDR_SETTING){
					selAddrBit=(selAddrBit-1+2)%2;
					LCD_PutPosition(3+selAddrBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
					tempAddressArr[2*selAddrBit+1]='0'+((tempAddressArr[2*selAddrBit+1]-'0'-1+10)%10);
					LCD_PutString(3,3,tempAddressArr);;
					LCD_PutPosition(3+selAddrBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_UNITS_NUM_SETTING){
					selUnitsNumBit=(selUnitsNumBit-1+3)%3;
					LCD_PutPosition(3+selUnitsNumBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_UNITS_NUM_BIT_SETTING){
					tempUnitsNumArr[2*selUnitsNumBit+1]='0'+((tempUnitsNumArr[2*selUnitsNumBit+1]-'0'-1+10)%10);
					LCD_PutString(3,3,tempUnitsNumArr);;
					LCD_PutPosition(3+selUnitsNumBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_TIMEOUT_SETTING){
					selTimeoutBit=(selTimeoutBit-1+1)%1;
					LCD_PutPosition(3+selTimeoutBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_TIMEOUT_BIT_SETTING){

					tempTimeoutArr[2*selTimeoutBit+1]='0'+((tempTimeoutArr[2*selTimeoutBit+1]-'0'-1+10)%10);
					LCD_PutString(3,3,tempTimeoutArr);;
					LCD_PutPosition(3+selTimeoutBit,3);
					Write_Cmd(0x0f);
				}
			}
		}
	}
	if(k7==0)
	{
		delayMs(10);
		if(k7==0)
		{
			while(!k7);
			if(inSetting){
				if(menuState==MENU_STATE_NOP)
				{
					menuIndex=(menuIndex+1)%MENU_LEN;
					showMenuItem(menuIndex,1);
				}
				else if(menuState==MENU_STATE_IN_ADDR_SETTING){
					selAddrBit=(selAddrBit+1)%2;
					LCD_PutPosition(3+selAddrBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
					tempAddressArr[2*selAddrBit+1]='0'+((tempAddressArr[2*selAddrBit+1]-'0'+1)%10);
					LCD_PutString(3,3,tempAddressArr);
					LCD_PutPosition(3+selAddrBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_UNITS_NUM_SETTING){
					selUnitsNumBit=(selUnitsNumBit+1)%3;
					LCD_PutPosition(3+selUnitsNumBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_UNITS_NUM_BIT_SETTING){
					tempUnitsNumArr[2*selUnitsNumBit+1]='0'+((tempUnitsNumArr[2*selUnitsNumBit+1]-'0'+1)%10);
					LCD_PutString(3,3,tempUnitsNumArr);
					LCD_PutPosition(3+selUnitsNumBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_TIMEOUT_SETTING){
					selTimeoutBit=(selTimeoutBit+1)%1;
					LCD_PutPosition(3+selTimeoutBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_TIMEOUT_BIT_SETTING){
					tempTimeoutArr[2*selTimeoutBit+1]='0'+((tempTimeoutArr[2*selTimeoutBit+1]-'0'+1)%10);
					LCD_PutString(3,3,tempTimeoutArr);
					LCD_PutPosition(3+selTimeoutBit,3);
					Write_Cmd(0x0f);
				}						
			}
		}
	}
*/
}
sbit IR=P3^2; 
char code Tab[16]="0123456789ABCDEF";
//每个按键编码为4字节，在此为方便起见，选用第3字节作为对应字符的编码
#define IR_CHANNEL_MINUS 0x45
#define IR_CHANNEL 0x46
#define IR_CHANNEL_ADD 0x47
#define IR_PREV 0x44
#define IR_NEXT 0x40
#define IR_PLAY 0x43
#define IR_VOL_ADD 0x07
#define IR_VOL_MINUS 0x15
#define IR_EQ 0x09
#define IR_ZERO 0x16
#define IR_ONE 0x0C
#define IR_TWO 0x18
#define IR_THREE 0x5E
#define IR_FOUR 0x08
#define IR_FIVE 0x1C
#define IR_SIX 0x5A
#define IR_SEVEN 0x42
#define IR_EIGHT 0x52
#define IR_NINE 0x4A
#define IP_100 0x19
#define IP_200 0x0D
unsigned char  irtime;
bit irpro_ok,irok;
unsigned char IRcord[4];
unsigned char irdata[33];

unsigned char TempData[16];
uchar changeNum(uchar irKey){
	switch (irKey){
	case IR_ZERO:
		return 0;
	case IR_ONE:
		return 1;
	case IR_TWO:
		return 2;
	case IR_THREE:
		return 3;
	case IR_FOUR:
		return 4;
	case IR_FIVE:
		return 5;
	case IR_SIX:
		return 6;
	case IR_SEVEN:
		return 7;
	case IR_EIGHT:
		return 8;
	case IR_NINE:
		return 9;
	}
}
void Ir_work(void);
void Ircordpro(void);
void EX0_ISR (void) interrupt 0 
{
  static unsigned char  i;             
  static bit startflag;               

if(startflag)                         
   {
    if(irtime<63&&irtime>=33)
                        i=0;
    		irdata[i]=irtime;
    		irtime=0;
    		i++;
   			 if(i==33)
      			{
	  			 irok=1;
				 i=0;
	  			}
          }
  	 else
		{
		irtime=0;
		startflag=1;
		}

}
void Ir_work(void)
{	
	if(isInfraredAllowed==0){
	 return;
	}
	switch( IRcord[2]){
	case IR_PREV:
	{
		if(menuState==MENU_STATE_NOP)
		{
			menuIndex=(menuIndex-1+MENU_LEN)%MENU_LEN;
			showMenuItem(menuIndex,1);
		}
		else if(menuState==MENU_STATE_IN_ADDR_SETTING||menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
				selAddrBit=(selAddrBit-1+2)%2;
				LCD_PutPosition(3+selAddrBit,3);
				Write_Cmd(0x0f);
		}

		else if(menuState==MENU_STATE_IN_UNITS_NUM_SETTING||menuState==MENU_STATE_IN_UNITS_NUM_BIT_SETTING){
			selUnitsNumBit=(selUnitsNumBit-1+3)%3;
			LCD_PutPosition(3+selUnitsNumBit,3);
			Write_Cmd(0x0f);
		}
		else if(menuState==MENU_STATE_IN_TIMEOUT_SETTING||menuState==MENU_STATE_IN_TIMEOUT_BIT_SETTING){
			selTimeoutBit=(selTimeoutBit-1+1)%1;
			LCD_PutPosition(3+selTimeoutBit,3);
			Write_Cmd(0x0f);
		}
	}
	break;
	
	case IR_NEXT:
	{
		if(menuState==MENU_STATE_NOP)
		{
			menuIndex=(menuIndex+1)%MENU_LEN;
			showMenuItem(menuIndex,1);
		}
		else if(menuState==MENU_STATE_IN_ADDR_SETTING||menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
				selAddrBit=(selAddrBit+1)%2;
				LCD_PutPosition(3+selAddrBit,3);
				Write_Cmd(0x0f);
		}

		else if(menuState==MENU_STATE_IN_UNITS_NUM_SETTING||menuState==MENU_STATE_IN_UNITS_NUM_BIT_SETTING){
			selUnitsNumBit=(selUnitsNumBit+1)%3;
			LCD_PutPosition(3+selUnitsNumBit,3);
			Write_Cmd(0x0f);
		}
		else if(menuState==MENU_STATE_IN_TIMEOUT_SETTING||menuState==MENU_STATE_IN_TIMEOUT_BIT_SETTING){
			selTimeoutBit=(selTimeoutBit+1)%1;
			LCD_PutPosition(3+selTimeoutBit,3);
			Write_Cmd(0x0f);
		}
	}
	break;
	case IR_VOL_MINUS:
	{
		if(menuState==MENU_STATE_NOP)
		{
			menuIndex=(menuIndex-1+MENU_LEN)%MENU_LEN;
			showMenuItem(menuIndex,1);
		}
		else if(menuState==MENU_STATE_IN_ADDR_SETTING||menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
				tempAddressArr[2*selAddrBit+1]='0'+((tempAddressArr[2*selAddrBit+1]-'0'-1+10)%10);
				LCD_PutString(3,3,tempAddressArr);
				LCD_PutPosition(3+selAddrBit,3);
				Write_Cmd(0x0f);
		}

		else if(menuState==MENU_STATE_IN_UNITS_NUM_SETTING||menuState==MENU_STATE_IN_UNITS_NUM_BIT_SETTING){
				tempUnitsNumArr[2*selUnitsNumBit+1]='0'+((tempUnitsNumArr[2*selUnitsNumBit+1]-'0'-1+10)%10);
				LCD_PutString(3,3,tempUnitsNumArr);
				LCD_PutPosition(3+selUnitsNumBit,3);
				Write_Cmd(0x0f);
		}
		else if(menuState==MENU_STATE_IN_TIMEOUT_SETTING||menuState==MENU_STATE_IN_TIMEOUT_BIT_SETTING){
				tempTimeoutArr[2*selTimeoutBit+1]='0'+((tempTimeoutArr[2*selTimeoutBit+1]-'0'-1+10)%10);
				LCD_PutString(3,3,tempTimeoutArr);
				LCD_PutPosition(3+selTimeoutBit,3);
				Write_Cmd(0x0f);
		}
	}
	break;
	
	case IR_VOL_ADD:
	{
		if(menuState==MENU_STATE_NOP)
		{
			menuIndex=(menuIndex+1)%MENU_LEN;
			showMenuItem(menuIndex,1);
		}
		else if(menuState==MENU_STATE_IN_ADDR_SETTING||menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
				tempAddressArr[2*selAddrBit+1]='0'+((tempAddressArr[2*selAddrBit+1]-'0'+1)%10);
				LCD_PutString(3,3,tempAddressArr);
				LCD_PutPosition(3+selAddrBit,3);
				Write_Cmd(0x0f);
		}

		else if(menuState==MENU_STATE_IN_UNITS_NUM_SETTING||menuState==MENU_STATE_IN_UNITS_NUM_BIT_SETTING){
				tempUnitsNumArr[2*selUnitsNumBit+1]='0'+((tempUnitsNumArr[2*selUnitsNumBit+1]-'0'+1)%10);
				LCD_PutString(3,3,tempUnitsNumArr);
				LCD_PutPosition(3+selUnitsNumBit,3);
				Write_Cmd(0x0f);
		}
		else if(menuState==MENU_STATE_IN_TIMEOUT_SETTING||menuState==MENU_STATE_IN_TIMEOUT_BIT_SETTING){
				tempTimeoutArr[2*selTimeoutBit+1]='0'+((tempTimeoutArr[2*selTimeoutBit+1]-'0'+1)%10);
				LCD_PutString(3,3,tempTimeoutArr);
				LCD_PutPosition(3+selTimeoutBit,3);
				Write_Cmd(0x0f);
		}
	}
	break;
	
	case IR_CHANNEL_MINUS:
	case IR_CHANNEL:
	case IR_CHANNEL_ADD:
	{
				if(menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
					menuState=MENU_STATE_IN_ADDR_SETTING;
				}
				else if(menuState==MENU_STATE_IN_ADDR_SETTING){
					menuState=MENU_STATE_NOP;
					
					Write_Cmd(0x0C);//关闭游标及反白
					
				 	showMenu();
				}
				else if(menuState==MENU_STATE_IN_ADDR_SETTING_FINISHED){
					
					menuItemSeleted(menuIndex);
				}

				else if(menuState==MENU_STATE_IN_UNITS_NUM_BIT_SETTING){
					menuState=MENU_STATE_IN_UNITS_NUM_SETTING;
				}
				else if(menuState==MENU_STATE_IN_UNITS_NUM_SETTING){
					menuState=MENU_STATE_NOP;
					
					Write_Cmd(0x0C);//关闭游标及反白
					
				 	showMenu();
				}
				else if(menuState==MENU_STATE_IN_UNITS_NUM_SETTING_FINISHED){
					
					menuItemSeleted(menuIndex);
				}
				else if(menuState==MENU_STATE_IN_TIMEOUT_BIT_SETTING){
					menuState=MENU_STATE_IN_TIMEOUT_SETTING;
				}
				else if(menuState==MENU_STATE_IN_TIMEOUT_SETTING){
					menuState=MENU_STATE_NOP;
					
					Write_Cmd(0x0C);//关闭游标及反白
					
				 	showMenu();
				}
				else if(menuState==MENU_STATE_IN_TIMEOUT_SETTING_FINISHED){
					
					menuItemSeleted(menuIndex);
				}
				else{
					menuState=MENU_STATE_NOP;
					clearInfraredFlagIfNeccesary();
					inSetting=0;
					displayCurrentLedMessage(0);
				}
	}
	break;
	case IR_EQ:
	{
		if(menuState==MENU_STATE_NOP){
			menuItemSeleted(menuIndex);
		}
		else if(menuState==MENU_STATE_IN_ADDR_SETTING||menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
			localAddress=(tempAddressArr[1]-'0')*10+(tempAddressArr[3]-'0');	
			unitStateArr[0]=localAddress; //需重新设置返回给上位机的本机地址编码
			storageBaseInfo();
			Write_Cmd(0x0C);//关闭游标及反白
			LCD_PutString(2,4,"设置完毕!");
			menuState=MENU_STATE_IN_ADDR_SETTING_FINISHED;			
		}
		else if(menuState==MENU_STATE_IN_UNITS_NUM_SETTING||menuState==MENU_STATE_IN_UNITS_NUM_BIT_SETTING){
			unitsNum=(tempUnitsNumArr[1]-'0')*100+(tempUnitsNumArr[3]-'0')*10+(tempUnitsNumArr[5]-'0');
			storageBaseInfo();
			Write_Cmd(0x0C);//关闭游标及反白
			LCD_PutString(2,4,"设置完毕!");
			menuState=MENU_STATE_IN_UNITS_NUM_SETTING_FINISHED;		
		}
		else if(menuState==MENU_STATE_IN_TIMEOUT_SETTING||menuState==MENU_STATE_IN_TIMEOUT_BIT_SETTING){
			timeoutThreshold=(tempTimeoutArr[1]-'0');
			storageBaseInfo();
			Write_Cmd(0x0C);//关闭游标及反白
			LCD_PutString(2,4,"设置完毕!");
			menuState=MENU_STATE_IN_TIMEOUT_SETTING_FINISHED;		
		}
	}
	break;
	
	case IR_ZERO:
	case IR_ONE:
	case IR_TWO:
	case IR_THREE:
	case IR_FOUR:
	case IR_FIVE:
	case IR_SIX:
	case IR_SEVEN:
	case IR_EIGHT:
	case IR_NINE:
	{
		if(menuState==MENU_STATE_IN_ADDR_SETTING||menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
			tempAddressArr[2*selAddrBit+1]='0'+changeNum(IRcord[2]);
			LCD_PutString(3,3,tempAddressArr);;
			LCD_PutPosition(3+selAddrBit,3);
			Write_Cmd(0x0f);
		}else if(menuState==MENU_STATE_IN_UNITS_NUM_SETTING||menuState==MENU_STATE_IN_UNITS_NUM_BIT_SETTING){
			tempUnitsNumArr[2*selUnitsNumBit+1]='0'+changeNum(IRcord[2]);
			LCD_PutString(3,3,tempUnitsNumArr);;
			LCD_PutPosition(3+selUnitsNumBit,3);
			Write_Cmd(0x0f);
		}
		else if(menuState==MENU_STATE_IN_TIMEOUT_SETTING||menuState==MENU_STATE_IN_TIMEOUT_BIT_SETTING){
			tempTimeoutArr[2*selTimeoutBit+1]='0'+changeNum(IRcord[2]);
			LCD_PutString(3,3,tempTimeoutArr);;
			LCD_PutPosition(3+selTimeoutBit,3);
			Write_Cmd(0x0f);
		}
	}
	break;
	}
      
	  
}

void Ircordpro(void)
{ 
  unsigned char i, j, k;
  unsigned char cord,value;

  k=1;
  for(i=0;i<4;i++)      
     {
      for(j=1;j<=8;j++) 
         {
          cord=irdata[k];
          if(cord>7)
             value|=0x80;
          if(j<8)
		    {
			 value>>=1;
			}
           k++;
         }
     IRcord[i]=value;
     value=0;     
     } 
	 irpro_ok=1;
}
#define UNIT_STATE_EMPTY  1
#define UNIT_STATE_CHARGING 2
#define UNIT_STATE_FULL  3
#define UNIT_STATE_ERROR  4
#define UNIT_STATE_OFFLINE 5
//定时器1 函数
void Timer0_isr(void) interrupt 1 using 1
{
	//50毫秒
	if(isInfraredAllowed==1){
		TH0=0xFF; 
  		TL0=0x00;
		irtime++;
		return;
	}
	TH0=(65536-45872)/256;	      
 	TL0=(65536-45872)%256;

	if(changeMessageFlag==0){
		changeMessageTimer++;
		if(changeMessageTimer>=20*messageInteval){
		    changeMessageTimer=0;
			changeMessageFlag=1;
		}
	}

	if(commState2==COMM_STATE_REQ_UNIT_INFO){
		reqUnitInfoInteval++;
		if(reqUnitInfoInteval==timeoutThreshold){
		    reqStateTimeout++;
			if(reqStateTimeout>4){
			   tempUnitStateArr[currentUnitTotalIndex]=UNIT_STATE_OFFLINE;
			   reqNextUnitInfo();
			}
			else{
			   reqCurrentUnitInfo();
			}
		}
	}
//	else if(readStateFlag==0)//注意！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！1 考虑是否else
	else if(commState2==COMM_STATE_NOTHING)
	{	
		reqStateTimer++;
		if(reqStateTimer>=2*queryStateInteval){	
			reqStateTimer=0;
			readStateFlag=1;
			currentUnitIndex=1;
		}
/*		else{
			changeMessageTimer++;
			if(changeMessageTimer==20*messageInteval){
				if(messageNum>0){
					curSentenceIndex=(++curSentenceIndex)%messageNum;
					changeMessageFlag=1;
					changeMessageTimer=0;
				}
			}
		}  
*/
	}

  
}
void init() {
	RW_485_1=0 ;
 	RW_485_2=1 ;
 	//定时器0 方式1 ，50ms。
//	SBUF=0x0;
	TMOD |= 0x01;	  		     
	TH0=(65536-45872)/256;	      
	TL0=(65536-45872)%256;
	ET0=1;           
	TR0=1;   

 	//串口1 方式1 9600		  			
//	SM0 = 0;
//	SM1 = 1;
//	REN = 1;
	SCON  = 0x50;    
	TMOD |= 0x20;
	TH1 = 0xfd;
	TL1 = 0xfd;
	TR1 = 1;

	//串口2 9600bps@11.0592MHz
	AUXR &= 0xf7;		//波特率不倍速
	S2CON = 0x50;		//8位数据,可变波特率
	BRT = 0xDC;		//设定独立波特率发生器重装值
	AUXR |= 0x04;		//独立波特率发生器时钟为Fosc,即1T
	AUXR |= 0x10;		//启动独立波特率发生器
	IE2 = 0x01;     //最低位ES2置0，关闭串口2中断
	

	//12864 初始化
	{  
   	delayMs(40);           //大于40MS的延时程序
   	PSB=1;                 //设置为8BIT并口工作模式
	delayMs(1);            //延时
  	RES=0;                 //复位
  	delayMs(1);            //延时
  	RES=1;                 //复位置高
  	delayMs(10);
  	Write_Cmd(0x30);       //选择基本指令集
  	DelayUs2x(50);         //延时大于100us
  	Write_Cmd(0x30);       //选择8bit数据流
  	DelayUs2x(20);         //延时大于37us
  	Write_Cmd(0x0c);       //开显示(无游标、不反白)
  	DelayUs2x(50);         //延时大于100us
  	Write_Cmd(0x01);       //清除显示，并且设定地址指针为00H
  	delayMs(15);           //延时大于10ms
  	Write_Cmd(0x06);       //指定在资料的读取及写入时，设定游标的移动方向及指定显示的移位，光标从右向左加1位移动
  	DelayUs2x(50);         //延时大于100us
	}
	

 	IPH=0x10;
	IP=0x10;//设置串口1中断优先级为髙优先级
	ES = 1;
	EA = 1;
	
	isSelected=0;
    
                                      
}
void afterInitAndRetrieveSetting(){
	 groupNum=unitsNum/queryUnitsNum; 
	 groupIndex=0;
}
void retrieveBaseInfo(){
	uchar m,n;

	localAddress=IapReadByte(IAP_ADDRESS+LOCAL_ADDRESS_ADDR);
	if(localAddress==0xff){
		localAddress=0;
	}
	
	unitsNum=IapReadByte(IAP_ADDRESS+UNITS_NUM_ADDR);
	if(unitsNum==0xff){
		unitsNum = 100;
	}

	messageNum=IapReadByte(IAP_ADDRESS+MESSAGE_NUM_ADDR);
	if(messageNum==0xff){
		messageNum=0;
	}
	messageInteval=IapReadByte(IAP_ADDRESS+MESSAGE_INTEVAL_ADDR);
	if(messageInteval==0xff){
		messageInteval=10;
	}
	timeoutThreshold=IapReadByte(IAP_ADDRESS+TIMEOUT_THRESHOLD_ADDR);
	if(timeoutThreshold==0xff){
		timeoutThreshold=1;
	}
	for (m = 0; m < messageNum; m++) {
		for (n = 0; n < SENTENCE_LEN; n++) {
			messageBuf[m][n]=IapReadByte(IAP_ADDRESS+MESSAGE_LIST_ADDR+m*SENTENCE_LEN+n);
		}
	}
}
uchar xdata oldStateStat[6];
bit send2LEDFlag=0;
void refreshUnitsInfo(){
	uchar m,val;
	//保存采集之前的取走、充电、充满状态，如果采集前后没有变化，则不用更新点阵屏显示内容
	for(m=1;m<4;m++){
		oldStateStat[m]=stateStat[m];
	}
	stateStat[0]=stateStat[1]=stateStat[2]=stateStat[3]=stateStat[4]=stateStat[5]=0;
	for(m=1;m<=unitsNum;m++){
		val=tempUnitStateArr[m];
		unitStateArr[m]=val;
		stateStat[val]++;
	}
	stateStat[0]=m;
	if((oldStateStat[1]==stateStat[1])&&(oldStateStat[2]==stateStat[2])&&(oldStateStat[3]==stateStat[3])){
		send2LEDFlag=0;
	}
	else{
		send2LEDFlag=1;
	}
}
char testNum=0;
void sendSta2LEDIfNecessary(){

	if(send2LEDFlag==1){
	   commState2=COMM_STATE_SEND_STA;
	   	TR0=0;

		IE2 = 0x00;
		switchIfNecessary(121); //20个一组切换到第6组 索引从0开始
		sendCmdInstance(CMD_SEND_STA);
		sendDataWithoutSplitInstance(localAddress);
		sendDataWithoutSplitInstance(stateStat[1]);
		sendDataWithoutSplitInstance(stateStat[2]);
		sendDataWithoutSplitInstance(stateStat[3]);
		sendCmdInstance(SYM_DATA_END);
		IE2 = 0x01;

		TR0=1;
		commState2=COMM_STATE_NOTHING;
	}
}
void main() {
	//showMode=DISPLAY_MESSAGE;
	showMode=DISPLAY_STATISTICS;
	init();
	retrieveBaseInfo();
	displayCurrentLedMessage(1);
	afterInitAndRetrieveSetting();

	makeInfo();
	WDT_CONTR = 0x3f;//设置看门狗，溢出时间9.1022s
	if(showMode==DISPLAY_STATISTICS){
		ClrScreen();
	}
	while (1) {
		
		tready = 1;
		rready = 1;
		tready2 = 1;
		rready2 = 1;
		if(readStateFlag){
	//		LED_TEST=0;
			readStateFlag=0;
			tready2=0 ;
			commState2=COMM_STATE_REQ_UNIT_INFO;
	//		makeInfo();
			reqCurrentUnitInfo();			
			tready2=1;
	
		}

		if(totalRefreshed==1){
			totalRefreshed=0;
			refreshUnitsInfo();
			sendSta2LEDIfNecessary();

	
		}
		if(receiveMessagesFlag==1){
			receiveMessagesFlag=0;
			commState1=COMM_STATE_NOTHING;
			makeMessages();
		}
		if(showMode==DISPLAY_MESSAGE)
		{
			if(changeMessageFlag==1){
				changeMessageFlag=0;
			//	changeMessageTimer=0;//考虑在displayCurrentLedMessage();之后调用
				displayCurrentLedMessage(0);
			}
		}
		else{
			 displayStatisticsMessage();
		}
		WDT_CONTR = 0x3f;//设置看门狗，溢出时间9.1022s
		if(irok)                        
	 	{   
	   		Ircordpro();
 	  		irok=0;
	  	}

    	if(irpro_ok)                  
	  	{
	  		Ir_work();
			irpro_ok=0;
  	  	}
		keyScan();
		
	}

}



