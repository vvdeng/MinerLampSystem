#include <reg52.h>
#include<intrins.h>
#include "insideStorage.h"

#define uchar unsigned char
#define uint unsigned int
#define  _Nop()  _nop_()
          
sfr   AUXR    = 0x8e;                  
sfr   S2CON   = 0x9a;                
sfr   S2BUF   = 0x9b;                 
sfr   BRT     = 0x9c;                  
sfr   IE2     = 0xaf;  //中断允许寄存器2 最低位为ES2            
#define   S2RI    0x01                 
#define   S2TI    0x02                
               

sbit PSB = P2^1;
sbit RES = P2^3;
sbit RS = P2 ^ 4;
sbit RW = P2 ^ 5;
sbit E  = P2^6;

sbit RW_485 =P2^7;

sbit LED0 =P1^0;
sbit LED1 =P1^1;

sbit k1=P1^4;
sbit k3=P1^5;
sbit k5=P1^6;
sbit k7=P1^7;


#define DataPort P0 


#define RECEIVE 0x01
#define SEND 0x02
#define SENTENCE_MAX_NUM 6
#define SENTENCE_LEN  32

#define CMD_UPDATE_STAFF_INFO 0xC0
#define CMD_REQ_UNIT_INFO 0x80

#define COMM_STATE_NOTHING 0
#define COMM_STATE_REQ_UNIT_INFO 1
#define COMM_STATE_REQ_UNIT_INFO_RECEIVE 2
#define COMM_STATE_UPDATE_STAFF 3
#define COMM_STATE_BROADCAST_MESSAGES 4
#define COMM_STATE_REQ_RACK_UNITS_INFO 5
#define COMM_STATE_UPDATE_RACK_STAFF 6


unsigned char localAddress,unitsNum, curSentenceIndex,changeMessageTimer=0,reqStateTimer=0,currentUnitIndex=1,commState1,commState2,reqUnitInfoInteval,oldCommState=COMM_STATE_NOTHING,staffBufIndex=0,oldTH0=0,oldTL0=0;
uint messagesBufIndex=0;
bit ack,changeMessageFlag=0,readStateFlag=0,receiveMessagesFlag=0,updateRackStaffFlag=0,inSetting=0,inAddrSetting=0,isInfraredAllowed=0;
uchar xdata unitStateArr[100];


#define LOCAL_ADDRESS_ADDR  0x0
#define UNITS_NUM_ADDR  0x2
#define MESSAGE_NUM_ADDR  0x4
#define MESSAGE_INTEVAL_ADDR  0x6
#define MESSAGE_LIST_ADDR  0x8

uchar xdata messageBuf[SENTENCE_MAX_NUM][SENTENCE_LEN+1]={'\0'};
uchar messageInteval,messageNum=0,tempMessageNum=0;
uchar* code menu[]={"地址设置","灯架数设置","红外设置"};
#define MENU_LEN 3;
bit tready,tready2,rready,rready2;
bit isSelected;

#define FIELD_COUNT 3
#define STAFF_INFO_FIELD_LENGTN 8
uchar xdata staff_buf[FIELD_COUNT][STAFF_INFO_FIELD_LENGTN];
uchar xdata messagesBuf[400];
uchar xdata staffBuf[60];

void displayCurrentLedMessage();
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

void makeInfo();
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
void sendCmdAndAddrInstance(uchar cmd,uchar addr){

	RW_485=1;

	
	S2BUF=cmd;
	while(!(S2CON&S2TI));
	S2CON&=~S2TI;
	
	S2BUF=addr;
	while(!(S2CON&S2TI));
	S2CON&=~S2TI;
	
	RW_485=0;
}
void sendDataInstance(uchar ldata){
	
	RW_485=1;
		
	S2BUF=(ldata>>4)&0x0f;
	while(!(S2CON&S2TI));
	S2CON&=~S2TI;

	S2BUF=ldata&0x0f;
	while(!(S2CON&S2TI));
	S2CON&=~S2TI;
	
	RW_485=0;
	
	
}
/*
void sendData(uchar ldata){
	RW_485=1;
	S2BUF=ldata;
}*/
/*
void sendCmdAndAddr(uchar cmd,uchar addr){
	uchar temp;


	RW_485=1;
//	temp=cmd;
	S2BUF=temp;
//	S2CON&=~S2TI;
	while(!(S2CON&S2TI));

	RW_485=1;
	temp=addr;
	if(temp==addr&&(addr==3)){
			LED1=~LED1;
		}
		if(temp==3){
			LED0=~LED0;
		}
	S2BUF=addr;

//	S2CON&=~S2TI;
	while(!(S2CON&S2TI));
	RW_485=0;
}
void sendData(uchar ldata){
	
	RW_485=1;
		
	S2BUF=(ldata>>4)&0x0f;
	S2CON&=~S2TI;
	while(!(S2CON&S2TI));
	S2BUF=ldata&0x0f;
	S2CON&=~S2TI;
	while(!(S2CON&S2TI));
	RW_485=0;
	
	
}*/
void reqCurrentUnitInfo(){
	/*	TR0=0;
		TH0=(65536-45872)/256;	      
 		TL0=(65536-45872)%256;
		TR0=1;*/
		
	RW_485=1;
	if(currentUnitIndex<=5){
		
		TR0=0;
		IE2 = 0x00;
		sendCmdAndAddrInstance(CMD_REQ_UNIT_INFO,currentUnitIndex);
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
		TR0=1;
		commState2=COMM_STATE_NOTHING;
		currentUnitIndex=1;
		
		
	}
}
void reqNextUnitInfo(){
	currentUnitIndex++;
	reqCurrentUnitInfo();
}

void S2INT() interrupt 8
{
		
	if(S2CON&S2RI)
	{	

		S2CON&=~S2RI;
		if(commState2==COMM_STATE_REQ_UNIT_INFO)
		{
			unitStateArr[currentUnitIndex]=S2BUF;
			reqNextUnitInfo();
		}
		
	}
	if(S2CON&S2TI)
	{
		
		if(commState2==COMM_STATE_REQ_UNIT_INFO)
		{
			
			
		}
		else if(commState2==COMM_STATE_UPDATE_STAFF)
		{
		
		}
	
		S2CON&=~S2TI;
		RW_485=0;

	}

}



void makeInfo() {
	uchar m;
	unitStateArr[0]=localAddress;
	for (m = 1; m <= unitsNum; m++) {
		unitStateArr[m] = (unitStateArr[m]+1)%4;
//	unitStateArr[m]=3;
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
		SBUF=0x80;
		while(TI!=1);
		TI=0; 
	}
	else{
		tready=0 ;
	//	makeInfo();
	//	reqUnitInfo();
	 	for(m=0;m<=unitsNum;m++) 
	 	{
			SBUF=unitStateArr[m];	
	    	while(TI!=1);
		    TI=0; 
	 	}
		tready=1;
	}
}
void prepareUpdateStaffInfo(void){
	RW_485=1 ;
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
	//	LED0=0;
		sendCmdAndAddrInstance(CMD_UPDATE_STAFF_INFO,lampNo);
		
		for (m = 0; m < FIELD_COUNT; m++) {

			for (n = 0; n < STAFF_INFO_FIELD_LENGTN; n++) {
				sendDataInstance(staff_buf[m][n]);
				
			}

		}
			
		
		endUpdateStaffInfo();


}
void storageBaseInfo(){
	uchar m,n;
	IapEraseSector(IAP_ADDRESS); 
	IapProgramByte(IAP_ADDRESS+LOCAL_ADDRESS_ADDR, localAddress); 
	IapProgramByte(IAP_ADDRESS+UNITS_NUM_ADDR, unitsNum); 
    IapProgramByte(IAP_ADDRESS+MESSAGE_NUM_ADDR, messageNum);
	IapProgramByte(IAP_ADDRESS+MESSAGE_INTEVAL_ADDR, messageInteval);
    for (m = 0; m < messageNum; m++) {
		for (n = 0; n < SENTENCE_LEN; n++) {
			IapProgramByte(IAP_ADDRESS+MESSAGE_LIST_ADDR+m*SENTENCE_LEN+n, messageBuf[m][n]);
		}
	}
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
	curSentenceIndex=0;
	displayCurrentLedMessage();
	
	storageBaseInfo();
}


void ssio(void)
interrupt 4
{
          
	if(RI){
         RI=0;

	 	if(isBroadCast4Addr()){
			commState1=COMM_STATE_BROADCAST_MESSAGES;
			messagesBufIndex=0;
			tempMessageNum=0;
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
			staffBufIndex=0;
			return;
		}
		if(commState1==COMM_STATE_BROADCAST_MESSAGES){
			messagesBuf[messagesBufIndex++]=SBUF;
			if(messagesBufIndex==2){
				tempMessageNum=(((messagesBuf[0]<<4)&0x0f)+messagesBuf[1]);

			}
			if(messagesBufIndex>=(tempMessageNum*SENTENCE_LEN+2)*2){
			
				receiveMessagesFlag=1;
			}
		
		}else if(commState1==COMM_STATE_UPDATE_RACK_STAFF){
			staffBuf[staffBufIndex++]=SBUF;
			
			if(staffBufIndex>=(FIELD_COUNT*STAFF_INFO_FIELD_LENGTN+1)*2){
				TR0=0;
				IE2 = 0x00;
				ES=0; 		
				makeUpdateStaff();
				commState1=COMM_STATE_NOTHING;
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
	
	
}
void displayCurrentLedMessage(){
		//功能设置中
	if(inSetting==1){
		return;
	}
	
	ClrScreen(); 
	if(messageNum>0){
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
uchar menuState=MENU_STATE_NOP;
uchar selAddrBit=0;
uchar selUnitsNumBit=0;
uchar menuIndex=0;
uchar tempAddressArr[5];
uchar tempUnitsNumArr[7];
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
	  default:
	  	break;
	}
}

void keyScan(){
	if(k1==0)
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
				else{
					menuState=MENU_STATE_NOP;
					clearInfraredFlagIfNeccesary();
					inSetting=0;
					displayCurrentLedMessage();
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
			if(inSetting==0){
				inSetting=1;
				setInfraredFlag();
				menuState=MENU_STATE_NOP;
				menuIndex=0;
				showMenu();
			}
			else{
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
					menuIndex=(menuIndex-1+3)%3;
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
					menuIndex=(menuIndex+1)%3;
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
			}
		}
	}

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
			menuIndex=(menuIndex-1+3)%3;
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
	}
	break;
	
	case IR_NEXT:
	{
		if(menuState==MENU_STATE_NOP)
		{
			menuIndex=(menuIndex+1)%3;
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
	}
	break;
	case IR_VOL_MINUS:
	{
		if(menuState==MENU_STATE_NOP)
		{
			menuIndex=(menuIndex-1+3)%3;
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
	}
	break;
	
	case IR_VOL_ADD:
	{
		if(menuState==MENU_STATE_NOP)
		{
			menuIndex=(menuIndex+1)%3;
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
				else{
					menuState=MENU_STATE_NOP;
					clearInfraredFlagIfNeccesary();
					inSetting=0;
					displayCurrentLedMessage();
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
	if(commState2==COMM_STATE_REQ_UNIT_INFO){
		reqUnitInfoInteval++;
		if(reqUnitInfoInteval==4){//200毫秒
			unitStateArr[currentUnitIndex]=4;
			reqNextUnitInfo();
		}
	}
	else if(readStateFlag==0)
	{	
		reqStateTimer=(reqStateTimer+1)%200;
		if(reqStateTimer==0){		
			readStateFlag=1;
			currentUnitIndex=1;
		}
		else{
			changeMessageTimer++;
			if(changeMessageTimer==20*messageInteval){
				if(messageNum>0){
					curSentenceIndex=(++curSentenceIndex)%messageNum;
					changeMessageFlag=1;
					changeMessageTimer=0;
				}
			}
		}  
	}

  
}
void init() {

 	RW_485=1 ;
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
	

 
	IP=0x10;//设置串口1中断优先级为髙优先级
	ES = 1;
	EA = 1;
	
	isSelected=0;                                           
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
	for (m = 0; m < messageNum; m++) {
		for (n = 0; n < SENTENCE_LEN; n++) {
			messageBuf[m][n]=IapReadByte(IAP_ADDRESS+MESSAGE_LIST_ADDR+m*SENTENCE_LEN+n);
		}
	}
}
void main() {
	init();
	retrieveBaseInfo();
	displayCurrentLedMessage();

	
	while (1) {
		tready = 1;
		rready = 1;
		tready2 = 1;
		rready2 = 1;
		if(readStateFlag){
			readStateFlag=0;
			tready2=0 ;
			commState2=COMM_STATE_REQ_UNIT_INFO;
			makeInfo();
			reqCurrentUnitInfo();			
			tready2=1;
	
		}
	
		if(receiveMessagesFlag==1){
			receiveMessagesFlag=0;
			commState1=COMM_STATE_NOTHING;
			makeMessages();
		}
		if(changeMessageFlag==1){
			changeMessageFlag=0;
			displayCurrentLedMessage();
		}
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



