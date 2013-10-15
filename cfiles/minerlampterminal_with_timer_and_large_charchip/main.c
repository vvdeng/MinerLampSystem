
#include <reg52.h>  
#include<intrins.h> 
#include "insideStorage.h"    
#include "lcd12864.h" 
#include "vvspi.h"         
#define uchar unsigned char
#define  _Nop()  _nop_()
#define DATA_PRE 0x50 //0101 数据前导
#define DATA_PRE_UPDATE_STAFF 0x60 //0110 更新员工信息数据前导
sfr   ADC_CONTR      =   0xBC; 
sfr   ADC_RES       =   0xBD;                 
sfr   ADC_LOW2     =   0xBE;                
sfr   P1ASF         =   0x9D; 
sbit  EADC =IE^5;
#define   ADC_POWER      0x80                 
#define   ADC_FLAG      0x10                  
#define   ADC_START    0x08                 
#define   ADC_SPEEDLL   0x00                  
#define   ADC_SPEEDL     0x20                 
#define   ADC_SPEEDH     0x40                  
#define   ADC_SPEEDHH   0x60

#define ADC_STATE_CHANNEL 0 //状态ad通道选用P1.0  
#define ADC_BRIGHT_CHANNEL 1 //光线亮度ad通道选用P1.1  


#define LOCAL_ADDRESS_ADDR  0x0
#define BRIGHT_THRESHOLD_ADDR  0x02
#define IS_SETTED_ADDR  0x04
#define STAFF_INFO_ADDR 0x8



#define FIELD_COUNT 3
#define STAFF_INFO_FIELD_LENGTN 8

#define CMD_UPDATE_STAFF 0xC0
#define CMD_REQ_UNIT_INFO 0x80
#define COMM_STATE_NOTHING 0
#define COMM_STATE_UPDATE_STAFF 1
#define COMM_STATE_REQ_UNIT_INFO 2
#define COMM_STATE_UPDATE_STAFF_WAIT_ADDRESS 3
#define COMM_STATE_REQ_UNIT_INFO_WAIT_ADDRESS 4
             
#define BAT_VAL_ARR_LEN 10
sbit ledControl=P2^7;
sbit IR=P3^2; 
sbit ledTest=P3^4;
sbit k0=P3^5;
//串口1 485读写控制
sbit RW_485=P3^7;
//按键
/*sbit k1=P1^4;
sbit k3=P1^5;
sbit k5=P1^6;
sbit k7=P1^7;*/

sbit LED_TXD=P1^2;//2;
sbit LED_RXD=P1^4;//4;
sbit isLedDisplay=P2^7;


uchar* xdata fieldName[]={"姓名：","部门：","工种："};
uchar* xdata menu[]={"地址设置","亮度设置","调试设置"};
uchar xdata staffInfoBuf[FIELD_COUNT][STAFF_INFO_FIELD_LENGTN+1]={'\0'};
uchar xdata buf[2*FIELD_COUNT*STAFF_INFO_FIELD_LENGTN];
uchar* xdata batStateDesc[]={"检测状态","矿灯取走","矿灯充电","矿灯充满","发生故障"};
int xdata batAdValArr[BAT_VAL_ARR_LEN];

uchar tready,rready,batState=0,brightValue=0,commState=0,bufIndex=0,updateStaffInteval,localAddress=0,curSentenceIndex,currentChannel=0,brightThreshold,isSetted,isInfoSetted=0,batAdValArrIndex=0;
unsigned int stateValue=0,msCount=0,mCount=0;
bit isSelected=0,updateStaffFlag=0,stateAdFlag=0,brightAdFlag=0,inSetting=0,inAddrSetting=0,isInfraredAllowed=0,isdebug=1,timeChanged=0;

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


   	


//自定义命令格式：xxdd dddd  髙2位表示命令，后6位表示灯架地址 最多64位 0xC0
//更新员工信息 1100 0000 请求数据 1000 0000 
/*bit isUpdateInfoCmd() {
	return (SBUF&0xC0)==0xC0;
}
bit isReqUnitInfo() {
	return (SBUF&0xC0)==0x80;
}
*/
void showAddress(uchar x,uchar y){
	uchar localAddressArr[4];
	localAddress=localAddress%1000;
	localAddressArr[0]='0'+localAddress/100;
	localAddressArr[1]='0'+((localAddress%100)/10);
	localAddressArr[2]='0'+(localAddress%10);
	localAddressArr[3]='\0';
	LCD_PutSingleByteString(x,y,localAddressArr);
}
void displayStaffInfo(){
	uchar m,indexi=0,indexj=0,count=0,isChinese=0;

	if(inSetting){
		return;	
	}
	ClrScreen();
		for(m=0;m<FIELD_COUNT;m++){
			LCD_PutString(0,m+1,fieldName[m]);							
			LCD_PutString(3,m+1,staffInfoBuf[m]);
		}
	
}
void redisplayStaffInfoIfNecessary(bit instanceFlag){
	if(inSetting==1){
		return;
	}
	
	if(isSetted==1){
		if((instanceFlag==1)){
			displayStaffInfo();
		}
	}
	showAddress(13,1);


}
void storageBaseInfo(){
	uchar m,n;
	IapEraseSector(IAP_ADDRESS); 
	IapProgramByte(IAP_ADDRESS+LOCAL_ADDRESS_ADDR, localAddress);
    IapProgramByte(IAP_ADDRESS+BRIGHT_THRESHOLD_ADDR, brightThreshold); 
	if(isInfoSetted==1){
		isSetted=1;
	}
	IapProgramByte(IAP_ADDRESS+IS_SETTED_ADDR, isSetted);
	for (m = 0; m < FIELD_COUNT; m++) {
		for (n = 0; n < STAFF_INFO_FIELD_LENGTN; n++) {
			IapProgramByte(IAP_ADDRESS+STAFF_INFO_ADDR+m*STAFF_INFO_FIELD_LENGTN+n, staffInfoBuf[m][n]);
		}
	}
     
}
void updateStaffInfoIfNeccesary(){
	uchar m,n,i=0;
	if(updateStaffFlag==1){
		updateStaffFlag=0;
		isInfoSetted=1;
		for (m = 0; m < FIELD_COUNT; m++) {

			for (n = 0; n < STAFF_INFO_FIELD_LENGTN; n++) {
				

				staffInfoBuf[m][n] = buf[i++] << 4;
			
				staffInfoBuf[m][n] += (buf[i++]  & 0x0f);
			}

		}
		redisplayStaffInfoIfNecessary(1);
		storageBaseInfo();

	}
}
/*
void sendUnitInfo(){
	if(tready==0){ //发送忙碌
		SBUF=0x80;
		while(TI!=1);
		TI=0; 
	}
	else{
		tready=0 ;
		RW_485=1;
		SBUF=(stateValue%100);	
	    while(TI!=1);
		TI=0; 
	 	RW_485=0;
		tready=1;
	}
}
*/
void sendData(uchar c)
{	
	RW_485=1;
    SBUF = c;
	TI = 0;
    while(!TI); 
}

/*void ssio(void)
interrupt 4
{
	
     
	if(RI){
		RI=0;
			if((SBUF&0xC0)==0x80){
			LED_RXD=0;
			}
		   
			if(isUpdateInfoCmd()){
				if(inSetting){//为了避免修改地址时，同时修改扇区内容
					return;
				}
				
		
				if(((SBUF&0x10)==0x00)&&((SBUF&0x0F)==((localAddress>>4)&0x0f)))
				{	
					
					addressHighHalf=1;
				}else if(((SBUF&0x10)==0x10)&&((SBUF&0x0F)==(localAddress&0x0f))){
					if(addressHighHalf==1){
						commState=COMM_STATE_UPDATE_STAFF;
						bufIndex=0;	
					}
					addressHighHalf=0;
					
				}
				return;
			}
			if(isReqUnitInfo())
			{
			
				if(((SBUF&0x10)==0x00)&&((SBUF&0x0F)==((localAddress>>4)&0x0f)))
				{
					addressHighHalf=1;
				}else if(((SBUF&0x10)==0x10)&&((SBUF&0x0F)==(localAddress&0x0f))){
					if(addressHighHalf==1){
						commState=COMM_STATE_REQ_UNIT_INFO;
					//	LED_RXD=~LED_RXD;
						sendData((stateValue%100));
					}
					addressHighHalf=0;
				
				}
				return;	
			}
      
          if(commState==COMM_STATE_UPDATE_STAFF){
		 	buf[bufIndex++]=SBUF;
				
		  	if(bufIndex==48){
	
		 	 commState=COMM_STATE_NOTHING;
			 updateStaffFlag=1;
		 }
	  
		}

	}
	else if(TI){                       
		TI=0;
		if(commState==COMM_STATE_REQ_UNIT_INFO){
			commState=COMM_STATE_NOTHING;
		}
		RW_485=0;
	}

}
*/
#define UNIT_STATE_NULL  0
#define UNIT_STATE_EMPTY  1
#define UNIT_STATE_CHARGING 2
#define UNIT_STATE_FULL  3
#define UNIT_STATE_ERROR  4
/*uchar changeStateValue(uchar lstateVal){
	if(lstateVal<50){
		return UNIT_STATE_EMPTY;
	}else if(lstateVal<200){
		return UNIT_STATE_CHARGING;
	}else {
		return UNIT_STATE_FULL;
	}
}*/
void ssio(void)
interrupt 4
{
	
	if(RI){
		
		RI=0;
		switch(SBUF&0x80){
			case 0x80:{
				switch(SBUF){
					case CMD_UPDATE_STAFF:{
						if(inSetting){//为了避免修改地址时，同时修改扇区内容
							return;
						}
						
						commState=COMM_STATE_UPDATE_STAFF_WAIT_ADDRESS;
					}
					break;
					case CMD_REQ_UNIT_INFO:{
						commState=COMM_STATE_REQ_UNIT_INFO_WAIT_ADDRESS;
						
					}
					break;
				}
			}
			break;
			case 0x00:{
				switch(commState){
					case COMM_STATE_UPDATE_STAFF_WAIT_ADDRESS:{
						
						if(SBUF==localAddress){
							LED_RXD=~LED_RXD;
							commState=COMM_STATE_UPDATE_STAFF;
							bufIndex=0;	
						}else{
							commState=COMM_STATE_NOTHING;
							LED_RXD=1;//指示灯灭
						}
					}
					break;
					case COMM_STATE_REQ_UNIT_INFO_WAIT_ADDRESS:{
						
						if(SBUF==localAddress){
							LED_RXD=~LED_RXD;
							commState=COMM_STATE_REQ_UNIT_INFO;
							LED_TXD=0;
							sendData((batState|DATA_PRE));
		                   
							LED_TXD=1;//指示灯灭
							LED_RXD=1;//指示灯灭

						}else{
							commState=COMM_STATE_NOTHING;
							LED_RXD=1;//指示灯灭
						}
					}
					break;
					case COMM_STATE_UPDATE_STAFF:{
							if((SBUF&0xF0)!=DATA_PRE_UPDATE_STAFF){
								return;
							}
							buf[bufIndex++]=SBUF;
		  					if(bufIndex==48){
								LED_RXD=1;//指示灯灭
		 	 					commState=COMM_STATE_NOTHING;
							 	updateStaffFlag=1;
								ledTest=~ledTest;
							}
					}
					break;
				}	
			}
			break;
		
		}

	}
	else if(TI){                       
		TI=0;
		if(commState==COMM_STATE_REQ_UNIT_INFO){
			commState=COMM_STATE_NOTHING;
		}
		RW_485=0;
	}

}

/*void ISR_INT1(void) interrupt 2 
{
	pumpFlag=1;
		ledTest2=~ledTest2;
	//	LED0=!LED0;  
}*/
#define CHARGE_TIME_ARR_LEN 5
#define SHOW_TIME 1
#define CLEAR_TIME 0
uchar  chargeTimeArr[CHARGE_TIME_ARR_LEN+1]={'\0'}; 
void setChargeTimeArr(uint lmCount,bit showTime){
	uchar m=0; 
	if(showTime==CLEAR_TIME){
 		for(m=0;m<CHARGE_TIME_ARR_LEN;m++){
			chargeTimeArr[m]=' ';
		}
		
	}
	else{
	  uchar minute=lmCount%60; 
	  uchar hour=lmCount/60;
	  chargeTimeArr[0]='0'+(hour/10);
	  chargeTimeArr[1]='0'+(hour%10);
	  chargeTimeArr[2]='-';
	  chargeTimeArr[3]='0'+(minute/10);
	  chargeTimeArr[4]='0'+(minute%10);
	 
	}
}
void showTimeIfNeccesary(){
 	uchar m=0;
	if( inSetting==1){
		return;
	}
	if(batState==UNIT_STATE_CHARGING||batState==UNIT_STATE_FULL){
 		setChargeTimeArr(mCount,SHOW_TIME);
 	}
 	
 	else{
 		setChargeTimeArr(0,CLEAR_TIME);
 	}
 	LCD_PutSingleByteString(11,4,chargeTimeArr);
	//LCD_PutString(6,4,"");
}
void clearTime(){
	msCount=0;
	mCount=0;
}
void init() {
	uchar m;
	
	RW_485=0; //485初始处于接收状态
  	
	//定时器0初始化 50ms
	TMOD |= 0x01;	  		     
    TH0=(65536-45872)/256;	      
	TL0=(65536-45872)%256; 
	ET0=1;           
	TR0=1;  
  	
	//串口1初始化
    SCON  = 0x50;    
	TMOD |= 0x20;
	TH1 = 0xfd;
	TL1 = 0xfd;
	TR1 = 1;
	LCD_DispIni(); //先初始化12864屏
	//片内AD初始化
	P1ASF = 0x03;  //设置P1.0、P1.1口                     
    ADC_RES = 0;                        
	ADC_CONTR = ADC_POWER | ADC_SPEEDLL;
//  delayMs(50); 
//	EADC=1; //开ADC中断 不采用中断方式

	IP=0x10;//设定串口1为髙优先级
	ES=1; //开串口1中断
			
/*	//外部中断1 初始化
  	EX1=1;         
    IT1=1;  
*/
	EA = 1;
		
	init_spi(); 
	

	for(m=0;m<BAT_VAL_ARR_LEN;m++){
	 	batAdValArr[m]=-1;
	}

	isSelected=0;
	isLedDisplay=1; //灯座内LED默认灭。
}
#define MENU_STATE_NOP 0
#define MENU_STATE_IN_ADDR_SETTING 1
#define MENU_STATE_IN_ADDR_BIT_SETTING 2
#define MENU_STATE_IN_ADDR_SETTING_FINISHED 3
#define MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING 4
#define MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING 5
#define MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING_FINISHED 6
uchar menuState=MENU_STATE_NOP;
uchar selAddrBit=0;
uchar selBrightThresholdBit=0;
uchar menuIndex=0;

uchar tempAddressArr[4]={'\0'};
uchar tempBrightThresholdArr[4]={'\0'};
void initTempAddressArr(){
//	localAddress=localAddress%100;

	tempAddressArr[0]='0'+(localAddress/100);

	tempAddressArr[1]='0'+((localAddress%100)/10);

	tempAddressArr[2]='0'+(localAddress%10);
	tempAddressArr[3]='\0';
}
void initTempBrightThresholdArr(){


	tempBrightThresholdArr[0]='0'+(brightThreshold/100);

	tempBrightThresholdArr[1]='0'+((brightThreshold%100)/10);

	tempBrightThresholdArr[2]='0'+(brightThreshold%10);
	tempBrightThresholdArr[3]='\0';
}
void setInfraredFlag(){
	isInfraredAllowed=1;
	//外部中断0 初始化
	IT0 = 1;   
 	EX0 = 1;
			
	TMOD |= 0x01;
	TH0=0xFF; 
  	TL0=0x00;
	ET0=1; 
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
	if(isdebug){
		LCD_PutString(0,1,"调试");
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
		
		break;
	 case 1: //亮度设置
		menuState=MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING;
		selBrightThresholdBit=0;
		showMenuItem(menuIndex,0);
		initTempBrightThresholdArr();
		LCD_PutString(3,3,tempBrightThresholdArr);
		LCD_PutChar(2,3,26);//输出"->"
		LCD_PutPosition(3+selBrightThresholdBit,3);
		
		break;
	 case 2: //打开关闭调试
		menuState=MENU_STATE_NOP;
		isdebug=~isdebug;

		showMenu();
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
				resetUnderline();//逻辑消除之前的下划线
				isInfraredAllowed=1;
				//外部中断0 初始化
				setInfraredFlag();
				menuState=MENU_STATE_NOP;
				menuIndex=0;
				showMenu();
			}else{
				menuState=MENU_STATE_NOP;
				clearInfraredFlagIfNeccesary();
				inSetting=0;
				ClrScreen();//强行清屏	
				redisplayStaffInfoIfNecessary(1);
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
				showMenu();
			}
			else{
				if(menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
					menuState=MENU_STATE_IN_ADDR_SETTING;
				}
				else if(menuState==MENU_STATE_IN_ADDR_SETTING){
					menuState=MENU_STATE_NOP;
					
					//关闭游标及反白
					menuIndex=0;
				 	showMenu();
				}
				else if(menuState==MENU_STATE_IN_ADDR_SETTING_FINISHED){
					
					menuItemSeleted(menuIndex);
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
					menuState=MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING;
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING){
					menuState=MENU_STATE_NOP;
					
					//关闭游标及反白
					
				 	showMenu();
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING_FINISHED){
					
					menuItemSeleted(menuIndex);
				}
				else{
					menuState=MENU_STATE_NOP;
					clearInfraredFlagIfNeccesary();
					inSetting=0;
					ClrScreen();//强行清屏
					redisplayStaffInfoIfNecessary(1);
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
				localAddress=(tempAddressArr[1]-'0')*100+(tempAddressArr[3]-'0')*10+(tempAddressArr[5]-'0');
				storageBaseInfo();
				//关闭游标及反白
				LCD_PutString(2,4,"设置完毕!");
				menuState=MENU_STATE_IN_ADDR_SETTING_FINISHED;
					
			}
			else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING){
				menuState=MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING;
			}
			else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
				brightThreshold=(tempBrightThresholdArr[1]-'0')*100+(tempBrightThresholdArr[3]-'0')*10+(tempBrightThresholdArr[5]-'0');
				storageBaseInfo();
				//关闭游标及反白
				LCD_PutString(2,4,"设置完毕!");
				menuState=MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING_FINISHED;
					
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
					selAddrBit=(selAddrBit-1+3)%3;
					LCD_PutPosition(3+selAddrBit,3);
					
				}
				else if(menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
					tempAddressArr[2*selAddrBit+1]='0'+((tempAddressArr[2*selAddrBit+1]-'0'-1+10)%10);
					LCD_PutString(3,3,tempAddressArr);;
					LCD_PutPosition(3+selAddrBit,3);
					
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING){
					selBrightThresholdBit=(selBrightThresholdBit-1+3)%3;
					LCD_PutPosition(3+selBrightThresholdBit,3);
					
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
					tempBrightThresholdArr[2*selBrightThresholdBit+1]='0'+((tempBrightThresholdArr[2*selBrightThresholdBit+1]-'0'-1+10)%10);
					LCD_PutString(3,3,tempBrightThresholdArr);;
					LCD_PutPosition(3+selBrightThresholdBit,3);
					
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
					selAddrBit=(selAddrBit+1)%3;
					LCD_PutPosition(3+selAddrBit,3);
					
				}
				else if(menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
					tempAddressArr[2*selAddrBit+1]='0'+((tempAddressArr[2*selAddrBit+1]-'0'+1)%10);
					LCD_PutString(3,3,tempAddressArr);
					LCD_PutPosition(3+selAddrBit,3);
					
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING){
					selBrightThresholdBit=(selBrightThresholdBit+1)%3;
					LCD_PutPosition(3+selBrightThresholdBit,3);
					
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
					tempBrightThresholdArr[2*selBrightThresholdBit+1]='0'+((tempBrightThresholdArr[2*selBrightThresholdBit+1]-'0'+1)%10);
					LCD_PutString(3,3,tempBrightThresholdArr);
					LCD_PutPosition(3+selBrightThresholdBit,3);
					
				}
			}
		}
	}
*/
}
void displayADC(){
	
		uchar stateValueStr[5]={'\0'};
		uchar adcBrightStr[4]={'\0'};
		if(inSetting==1){
			return;
		}

		if(stateAdFlag==1){
			stateAdFlag=0;
			stateValueStr[0]='0'+(stateValue/1000);
			stateValueStr[1]='0'+((stateValue%1000)/100);
			stateValueStr[2]='0'+((stateValue%100)/10);
			stateValueStr[3]='0'+(stateValue%10);
			stateValueStr[4]='\0';
		//	LCD_PutString(0,4,"模拟量：");
  			LCD_PutSingleByteString(12,3,stateValueStr);
	
		}
		if(brightAdFlag==1){
			brightAdFlag=0;
			adcBrightStr[0]='0'+(brightValue/100);
			adcBrightStr[1]='0'+((brightValue%100)/10);
			adcBrightStr[2]='0'+((brightValue%100)%10);
			adcBrightStr[3]='\0';
			LCD_PutSingleByteString(13,2,adcBrightStr);
		}

}
void displayBatState(){
		
	
		if(inSetting==1){
			return;
		}

		{
		
			switch(batState){
			case UNIT_STATE_NULL :
				NullCGRAM();
				break;
			case UNIT_STATE_EMPTY :
				EmptyCGRAM();
				break;
			case UNIT_STATE_CHARGING :
				HalfCGRAM();
				break;
			case UNIT_STATE_FULL :
				FullCGRAM();
				break;
			case UNIT_STATE_ERROR:
				ErrorCGRAM();
				break;
			default :
				break;
			}
			DisplayCGRAM(0,4); 
			LCD_PutString(1,4,batStateDesc[batState]);
		//	delayMs(100);
		}

}
void retrieveBaseInfo(){
	uchar m,n;
	
	localAddress=IapReadByte(IAP_ADDRESS+LOCAL_ADDRESS_ADDR);
	if(localAddress==0xff){
		localAddress=0;
	}

	brightThreshold=IapReadByte(IAP_ADDRESS+BRIGHT_THRESHOLD_ADDR);
	if(brightThreshold==0xff){
		brightThreshold=0;
	}
	isSetted=IapReadByte(IAP_ADDRESS+IS_SETTED_ADDR);
	if(isSetted>1){
		isSetted=0;
	}
	for (m = 0; m < FIELD_COUNT; m++) {
		for (n = 0; n < STAFF_INFO_FIELD_LENGTN; n++) {
			staffInfoBuf[m][n]=IapReadByte(IAP_ADDRESS+STAFF_INFO_ADDR+m*STAFF_INFO_FIELD_LENGTN+n);
		}
	}
}
/*void adc_isr() interrupt 5 using  1
{	
	ADC_CONTR &= !ADC_FLAG; 
	if(currentChannel==ADC_STATE_CHANNEL){
      	          
      	stateValue=ADC_RES;
	  	stateAdFlag=1;
	}
	else if(currentChannel==ADC_BRIGHT_CHANNEL){
		brightValue=ADC_RES;
		brightAdFlag=1;
	}
	currentChannel=(++currentChannel)%2;
     ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START | currentChannel;
	  
}*/

unsigned int getAdValueByChannel(uchar channel,uchar bits){
	unsigned int result;
	bit adFinished=0;

	ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START | channel;
	while(adFinished==0){
		adFinished=(ADC_CONTR&ADC_FLAG);
	}
	if(bits==10)
	{
		result=(ADC_RES*4+(ADC_LOW2&0x03));
	}
	else{
		result=ADC_RES;
	}
	ADC_CONTR &= !ADC_FLAG; 
	return result;
}
void getAdValue(){
		
	stateValue=getAdValueByChannel(ADC_STATE_CHANNEL,10);
	batAdValArr[(batAdValArrIndex++)%BAT_VAL_ARR_LEN]=stateValue;
	stateAdFlag=1;
	delayMs(100);

	brightValue=getAdValueByChannel(ADC_BRIGHT_CHANNEL,8);
	brightAdFlag=1;
	if(brightValue<brightThreshold){
		isLedDisplay=0;	
	}
	else{
		isLedDisplay=1;
	}

	delayMs(100);

}
/*
unsigned int detectWaitTime=500;
void stateTest(){
	getAdValue();
    if(stateValue==0){
		delayMs(detectWaitTime);
		getAdValue();
    	if(stateValue==0){
			delayMs(detectWaitTime);
			getAdValue();
    		if(stateValue==0){
				
				batState=UNIT_STATE_EMPTY;
			}
		}
		return;
	}
	
  	if(stateValue>0){
		delayMs(detectWaitTime);
    	getAdValue();
		if(stateValue>0){
			delayMs(detectWaitTime);
    		getAdValue();
			if(stateValue>0){
				{
					batState=UNIT_STATE_CHARGING;
				}
			}
		}
	}
  	if(stateValue>0&&stateValue<10){
		delayMs(detectWaitTime);
		getAdValue();
    	if(stateValue>0&&stateValue<10){
			delayMs(detectWaitTime);
    		getAdValue();
			if(stateValue>0&&stateValue<10){
			
				batState=UNIT_STATE_FULL;
			}
		}
	}


}
*/
uchar oldBatState=UNIT_STATE_NULL;
uchar emptyCount=0;fullCount=0,chargingCount=0; //保存某个状态连续出现的次数
void stateTest(){
	//BAT_VAL_ARR_LEN
	uchar m,zeroCount=0,usefulCount=0,emptyRangeCount=0,fullRangeCount=0,chargingRangeCount=0;//fullRangeCount是为了解决充满时有时平均值也会小于10的问题，充满情况下一般10个采集点中至少会有3个大于10，而空载不会，但需要在实际情况中验证下正在充状态会不会有三个值掉到50以下。
	unsigned int sum=0,everage=0;
	for(m=0;m<BAT_VAL_ARR_LEN;m++){
		if(batAdValArr[m]==-1){
			batState=UNIT_STATE_NULL;
			return;
		}else if(batAdValArr[m]==0){
			zeroCount++;
		}
		else{
			if(batAdValArr[m]>100){
				chargingRangeCount++;
			}
			else if(batAdValArr[m]>15){ //充满和正在充 阈值 15？
				fullRangeCount++;
			}
			else{
				emptyRangeCount++;
			}
			usefulCount++;
			sum+=batAdValArr[m];
		}
	}
		everage=(usefulCount==0?0:sum/usefulCount);
		if(zeroCount>7||everage<=5){
				if(emptyCount<250){
					emptyCount++;
				}
				fullCount=0;
				chargingCount=0;
				
			
		} 
	

		if(fullRangeCount>4 ||everage>5){
			emptyCount=0;
			if(fullCount<250){
				fullCount++;
			}
			//充满状态的过渡期，也可认为是处于正在充电状态
			if(chargingCount<250){
				chargingCount++;
			}
			
		}
		if(chargingRangeCount>7||everage>=100){
			emptyCount=0;
			fullCount=0;
			if(chargingCount<250){
				chargingCount++;
			}
			
		}
		if(emptyCount>3){
			oldBatState=batState; //todo应在batState设置新值后再设定oldBatState状态
			batState=UNIT_STATE_EMPTY;
			clearTime();
		}
		else if(fullCount>20){
			chargingCount=0; //充满状态的过渡期结束，可认定进入充满状态
			oldBatState=batState;
			batState=UNIT_STATE_FULL;
		}
 		else if(chargingCount>3){
			oldBatState=batState;
			batState=UNIT_STATE_CHARGING;
		}

		else {
			batState=oldBatState;
		}
/*		everage=sum/usefulCount;
		if(everage<10){ //10为magicNum，经验值
			batState=UNIT_STATE_EMPTY;
		}else if(everage<50){
			batState=UNIT_STATE_FULL;
		
		}
		else{
				batState=UNIT_STATE_CHARGING;
		}
*/		
	
}
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
    {
      i=0;
    }
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
				selAddrBit=(selAddrBit-1+3)%3;
				LCD_PutPosition(3+selAddrBit,3);
				
		}
		else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING||menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
			selBrightThresholdBit=(selBrightThresholdBit-1+3)%3;
			LCD_PutPosition(3+selBrightThresholdBit,3);
			
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
				selAddrBit=(selAddrBit+1)%3;
				LCD_PutPosition(3+selAddrBit,3);
				
		}
		else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING||menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
			selBrightThresholdBit=(selBrightThresholdBit+1)%3;
			LCD_PutPosition(3+selBrightThresholdBit,3);
			
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
				tempAddressArr[selAddrBit]='0'+((tempAddressArr[selAddrBit]-'0'-1+10)%10);
				LCD_PutString(3,3,tempAddressArr);
				LCD_PutPosition(3+selAddrBit,3);
				
		}
		else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING||menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
				tempBrightThresholdArr[selBrightThresholdBit]='0'+((tempBrightThresholdArr[selBrightThresholdBit]-'0'-1+10)%10);
				LCD_PutString(3,3,tempBrightThresholdArr);
				LCD_PutPosition(3+selBrightThresholdBit,3);
				
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
				tempAddressArr[selAddrBit]='0'+((tempAddressArr[selAddrBit]-'0'+1)%10);
				LCD_PutString(3,3,tempAddressArr);
				LCD_PutPosition(3+selAddrBit,3);
				
		}
		else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING||menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
				tempBrightThresholdArr[selBrightThresholdBit]='0'+((tempBrightThresholdArr[selBrightThresholdBit]-'0'+1)%10);
				LCD_PutString(3,3,tempBrightThresholdArr);
				LCD_PutPosition(3+selBrightThresholdBit,3);
				
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
					
					//关闭游标及反白
					
				 	showMenu();
				}
				else if(menuState==MENU_STATE_IN_ADDR_SETTING_FINISHED){
					
					menuItemSeleted(menuIndex);
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
					menuState=MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING;
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING){
					menuState=MENU_STATE_NOP;
					
					//关闭游标及反白
					
				 	showMenu();
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING_FINISHED){
					
					menuItemSeleted(menuIndex);
				}
				else{
					menuState=MENU_STATE_NOP;
					clearInfraredFlagIfNeccesary();
					inSetting=0;
					ClrScreen();
					redisplayStaffInfoIfNecessary(1);
				}
	}
	break;
	case IR_EQ:
	{
		if(menuState==MENU_STATE_NOP){
			menuItemSeleted(menuIndex);
		}
		else if(menuState==MENU_STATE_IN_ADDR_SETTING||menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
			localAddress=(tempAddressArr[0]-'0')*100+(tempAddressArr[1]-'0')*10+(tempAddressArr[2]-'0');
			storageBaseInfo();
			//关闭游标及反白
			LCD_PutString(2,4,"设置完毕!");
			menuState=MENU_STATE_IN_ADDR_SETTING_FINISHED;			
		}
		else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING||menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
			brightThreshold=(tempBrightThresholdArr[0]-'0')*100+(tempBrightThresholdArr[1]-'0')*10+(tempBrightThresholdArr[2]-'0');
			storageBaseInfo();
			//关闭游标及反白
			LCD_PutString(2,4,"设置完毕!");
			menuState=MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING_FINISHED;		
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
			tempAddressArr[selAddrBit]='0'+changeNum(IRcord[2]);
			LCD_PutString(3,3,tempAddressArr);;
			LCD_PutPosition(3+selAddrBit,3);
			
		}else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING||menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
			tempBrightThresholdArr[selBrightThresholdBit]='0'+changeNum(IRcord[2]);
			LCD_PutString(3,3,tempBrightThresholdArr);;
			LCD_PutPosition(3+selBrightThresholdBit,3);
			
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

void infraredDetection(){
	if(isInfraredAllowed==1){
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
	 }
}


void Timer0_isr(void) interrupt 1 using 1
{
	if(isInfraredAllowed==1){
		TH0=0xFF; 
  		TL0=0x00;
		irtime++;
		return;
	}
	else{
	  TH0=(65536-45872)/256;	      
	  TL0=(65536-45872)%256;
 	  if(batState==UNIT_STATE_CHARGING||batState==UNIT_STATE_FULL){
		msCount++;
		if(msCount>=1200){//50ms*20*60
			msCount=0;
			mCount++;
			if(mCount>=1440){//60*24
				mCount=0;
			}
		}
	  }
	}

	
}


void main()
{

	retrieveBaseInfo();
	
	init();

	redisplayStaffInfoIfNecessary(1);
 	while(1){
		tready=1;
  		rready=1;
		getAdValue();

		updateStaffInfoIfNeccesary();
//		redisplayStaffInfoIfNecessary(0);
		if(isdebug==1){
			displayADC();
		}
		stateTest();
		displayBatState();
		showTimeIfNeccesary();
		keyScan();
		infraredDetection();
	}
}
/*
void  main(void)
{ 

	uchar xdata str[] = "西安重装",str2[] = "渭南光电科技";
		init_spi();
	 
	LCD_DispIni();
		LCD_PutString(0,0,str);
		LCD_PutString(4,1,str2);
		FullCGRAM();
		DisplayCGRAM(1,3);
	while(1)
   {  
  //  delayus(10000);	
    //  lcd_disp_sz_str(0,0,str);
	
    
   }
}*/

