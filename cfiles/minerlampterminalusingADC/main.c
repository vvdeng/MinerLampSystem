
#include <reg52.h>  
#include<intrins.h> 
#include "insideStorage.h"             
#define uchar unsigned char
#define  _Nop()  _nop_()
#define DATA_PRE 0x50 //0101 数据前导
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
#define DataPort P0 //12864显示屏数据通道

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
             

//12864显示屏
sbit RS = P2 ^ 0;
sbit RW = P2 ^ 1;
sbit E  = P2^2;
sbit PSB = P2^3;
sbit RES = P2^4;
sbit ledTest2=P2^5;
sbit batInterrupt=P3^3;
sbit batLevel=P3^6;
sbit ledTest=P3^4;
sbit k0=P3^5;
//串口1 485读写控制
sbit RW_485=P3^7;
//按键
sbit k1=P1^4;
sbit k3=P1^5;
sbit k5=P1^6;
sbit k7=P1^7;
sbit LED_TXD=P1^2;
sbit LED_RXD=P1^3;

uchar* xdata fieldName[]={"姓名：","部门：","工种："};
uchar* xdata menu[]={"地址设置","亮度设置","红外设置"};
uchar xdata staffInfoBuf[FIELD_COUNT][STAFF_INFO_FIELD_LENGTN+1]={'\0'};
uchar xdata buf[2*FIELD_COUNT*STAFF_INFO_FIELD_LENGTN];

uchar tready,rready,stateValue=0,batState=0,brightValue=0,commState=0,bufIndex=0,updateStaffInteval,localAddress=0,curSentenceIndex,currentChannel=0,brightThreshold,isSetted;
bit isSelected=0,updateStaffFlag=0,stateAdFlag=0,brightAdFlag=0,inSetting=0,inAddrSetting=0,isInfraredAllowed=0,isLedDisplay=0,displayChange=0;

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

unsigned char xdata full16x16[]={ 
0x03,0xC0,0x03,0xC0,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,
0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,
};
unsigned char xdata half16x16[]={ 
0x03,0xC0,0x03,0xC0,0x0F,0xF0,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,
0x08,0x10,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,0x0F,0xF0,
};
unsigned char xdata empty16x16[]={ 
0x03,0xC0,0x03,0xC0,0x0F,0xF0,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,
0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x0F,0xF0,
};
unsigned char xdata error16x16[]={ 
0x03,0xC0,0x03,0xC0,0x0F,0xF0,0x08,0x10,0x08,0x10,0x08,0x10,0x0C,0x30,0x0A,0x50,
0x09,0x90,0x09,0x90,0x0A,0x50,0x0C,0x30,0x08,0x10,0x08,0x10,0x08,0x10,0x0F,0xF0,
};
//unsigned char code buffers[1024*16];
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




void FullCGRAM()
{ 
     int i;
     Write_Cmd(0x30); 
     Write_Cmd(0x40);
     for(i=0;i<16;i++)
       {
	    Write_Data(full16x16[i*2]);
        Write_Data(full16x16[i*2+1]);
      }
}   
void HalfCGRAM()
{ 
     int i;
     Write_Cmd(0x30); 
     Write_Cmd(0x40);
     for(i=0;i<16;i++)
       {
	    Write_Data(half16x16[i*2]);
        Write_Data(half16x16[i*2+1]);
      }
}
void EmptyCGRAM()
{ 
     int i;
     Write_Cmd(0x30); 
     Write_Cmd(0x40);
     for(i=0;i<16;i++)
       {
	    Write_Data(empty16x16[i*2]);
        Write_Data(empty16x16[i*2+1]);
      }
} 
void ErrorCGRAM()
{ 
     int i;
     Write_Cmd(0x30); 
     Write_Cmd(0x40);
     for(i=0;i<16;i++)
       {
	    Write_Data(error16x16[i*2]);
        Write_Data(error16x16[i*2+1]);
      }
} 
void DisplayCGRAM(unsigned char x,unsigned char y)
{ 
 switch(y)
     {
	  case 1: Write_Cmd(0x80+x);break;
	  case 2: Write_Cmd(0x90+x);break;
	  case 3: Write_Cmd(0x88+x);break;
	  case 4: Write_Cmd(0x98+x);break;
      default:break;
	 }
    Write_Data(00);
    Write_Data(00);

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
	LCD_PutString(x,y,localAddressArr);
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
		showAddress(6,1);
}
void redisplayStaffInfoIfNecessary(bit instanceFlag){
	if(isSetted==0||inSetting==1){
		return;	
	}
	
	if((instanceFlag==1||displayChange==1)&&isLedDisplay==1){
		displayStaffInfo();

	}
	if((instanceFlag==1||displayChange==1)&&isLedDisplay==0){
		ClrScreen();
	}
}
void storageBaseInfo(){
	uchar m,n;
	IapEraseSector(IAP_ADDRESS); 
	IapProgramByte(IAP_ADDRESS+LOCAL_ADDRESS_ADDR, localAddress);
    IapProgramByte(IAP_ADDRESS+BRIGHT_THRESHOLD_ADDR, brightThreshold); 
	isSetted=1;
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
uchar changeStateValue(uchar lstateVal){
	if(lstateVal<50){
		return UNIT_STATE_EMPTY;
	}else if(lstateVal<200){
		return UNIT_STATE_CHARGING;
	}else {
		return UNIT_STATE_FULL;
	}
}
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
							sendData(((changeStateValue(stateValue))|DATA_PRE));
							LED_TXD=1;//指示灯灭
							LED_RXD=1;//指示灯灭

						}else{
							commState=COMM_STATE_NOTHING;
							LED_RXD=1;//指示灯灭
						}
					}
					break;
					case COMM_STATE_UPDATE_STAFF:{
							buf[bufIndex++]=SBUF;
		  					if(bufIndex==48){
								LED_RXD=1;//指示灯灭
		 	 					commState=COMM_STATE_NOTHING;
							 	updateStaffFlag=1;
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



void init() {

	RW_485=0; //485初始处于接收状态
  	
	//定时器0初始化
/* 	TMOD |= 0x01;	  		     
    TH0=(65536-45872)/256;	      
	TL0=(65536-45872)%256; 
	ET0=1;           
	TR0=1;  
*/  	
	//串口1初始化
    SCON  = 0x50;    
	TMOD |= 0x20;
	TH1 = 0xfd;
	TL1 = 0xfd;
	TR1 = 1;

	//片内AD初始化
	P1ASF = 0x03;  //设置P1.0、P1.1口                     
    ADC_RES = 0;                        
	ADC_CONTR = ADC_POWER | ADC_SPEEDLL;
//  delayMs(50); 
//	EADC=1; //开ADC中断 不采用中断方式

	IP=0x10;//设定串口1为髙优先级
	ES=1; //开串口1中断
			


	EA = 1;
		
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
	isSelected=0;
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

uchar tempAddressArr[7]={'\0'};
uchar tempBrightThresholdArr[7]={'\0'};
void initTempAddressArr(){
//	localAddress=localAddress%256;
	tempAddressArr[0]=' ';
	tempAddressArr[1]='0'+(localAddress/100);
	tempAddressArr[2]=' ';
	tempAddressArr[3]='0'+((localAddress%100)/10);
	tempAddressArr[4]=' ';
	tempAddressArr[5]='0'+(localAddress%10);
	tempAddressArr[6]='\0';
}
void initTempBrightThresholdArr(){

	tempBrightThresholdArr[0]=' ';
	tempBrightThresholdArr[1]='0'+(brightThreshold/100);
	tempBrightThresholdArr[2]=' ';
	tempBrightThresholdArr[3]='0'+((brightThreshold%100)/10);
	tempBrightThresholdArr[4]=' ';
	tempBrightThresholdArr[5]='0'+(brightThreshold%10);
	tempBrightThresholdArr[6]='\0';
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
	ET0=0;
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
		showMenuItem(menuIndex,1);
		initTempAddressArr();
		LCD_PutString(3,3,tempAddressArr);
		LCD_PutChar(2,3,26);//输出"->"
		LCD_PutPosition(3+selAddrBit,3);
		Write_Cmd(0x0f);
		break;
	 case 1: //亮度设置
		menuState=MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING;
		selBrightThresholdBit=0;
		showMenuItem(menuIndex,0);
		initTempBrightThresholdArr();
		LCD_PutString(3,3,tempBrightThresholdArr);
		LCD_PutChar(2,3,26);//输出"->"
		LCD_PutPosition(3+selBrightThresholdBit,3);
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
	if(k0==0)
	{
		delayMs(10);
		if(k0==0)
		{
			while(!k0);
			if(inSetting==0){
			
				inSetting=1;
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
	if(k1==0)
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
					
					Write_Cmd(0x0C);//关闭游标及反白
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
					
					Write_Cmd(0x0C);//关闭游标及反白
					
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
				Write_Cmd(0x0C);//关闭游标及反白
				LCD_PutString(2,4,"设置完毕!");
				menuState=MENU_STATE_IN_ADDR_SETTING_FINISHED;
					
			}
			else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING){
				menuState=MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING;
			}
			else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
				brightThreshold=(tempBrightThresholdArr[1]-'0')*100+(tempBrightThresholdArr[3]-'0')*10+(tempBrightThresholdArr[5]-'0');
				storageBaseInfo();
				Write_Cmd(0x0C);//关闭游标及反白
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
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
					tempAddressArr[2*selAddrBit+1]='0'+((tempAddressArr[2*selAddrBit+1]-'0'-1+10)%10);
					LCD_PutString(3,3,tempAddressArr);;
					LCD_PutPosition(3+selAddrBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING){
					selBrightThresholdBit=(selBrightThresholdBit-1+3)%3;
					LCD_PutPosition(3+selBrightThresholdBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
					tempBrightThresholdArr[2*selBrightThresholdBit+1]='0'+((tempBrightThresholdArr[2*selBrightThresholdBit+1]-'0'-1+10)%10);
					LCD_PutString(3,3,tempBrightThresholdArr);;
					LCD_PutPosition(3+selBrightThresholdBit,3);
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
					selAddrBit=(selAddrBit+1)%3;
					LCD_PutPosition(3+selAddrBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_ADDR_BIT_SETTING){
					tempAddressArr[2*selAddrBit+1]='0'+((tempAddressArr[2*selAddrBit+1]-'0'+1)%10);
					LCD_PutString(3,3,tempAddressArr);
					LCD_PutPosition(3+selAddrBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING){
					selBrightThresholdBit=(selBrightThresholdBit+1)%3;
					LCD_PutPosition(3+selBrightThresholdBit,3);
					Write_Cmd(0x0f);
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
					tempBrightThresholdArr[2*selBrightThresholdBit+1]='0'+((tempBrightThresholdArr[2*selBrightThresholdBit+1]-'0'+1)%10);
					LCD_PutString(3,3,tempBrightThresholdArr);
					LCD_PutPosition(3+selBrightThresholdBit,3);
					Write_Cmd(0x0f);
				}
			}
		}
	}

}
void displayADC(){
		uchar simuStateValue;
		uchar stateValueStr[4]={'\0'};
		uchar adcBrightStr[4]={'\0'};
		if(inSetting==1){
			return;
		}
		if(isLedDisplay==0){
			return;
		}
		if(stateAdFlag==1){
			stateAdFlag=0;
			stateValueStr[0]='0'+(stateValue/100);
			stateValueStr[1]='0'+((stateValue%100)/10);
			stateValueStr[2]='0'+((stateValue%100)%10);
			stateValueStr[3]='\0';
			LCD_PutString(0,4,"模拟量：");
  			LCD_PutString(5,4,stateValueStr);
			simuStateValue=changeStateValue(stateValue);
		
			switch(simuStateValue){
			case UNIT_STATE_EMPTY  :
				EmptyCGRAM();
				break;
			case UNIT_STATE_CHARGING :
				HalfCGRAM();
				break;
			case UNIT_STATE_FULL :
				FullCGRAM();
				break;
			case UNIT_STATE_ERROR :
				ErrorCGRAM();
				break;
			default :
				break;
			}
			DisplayCGRAM(7,4); 
		//	delayMs(100);
		}
		if(brightAdFlag==1){
			brightAdFlag=0;
			adcBrightStr[0]='0'+(brightValue/100);
			adcBrightStr[1]='0'+((brightValue%100)/10);
			adcBrightStr[2]='0'+((brightValue%100)%10);
			adcBrightStr[3]='\0';
			LCD_PutString(6,3,adcBrightStr);
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
		brightThreshold=100;
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

uchar getAdValueByChannel(uchar channel){
	uchar result;
	bit adFinished=0;

	ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START | channel;
	while(adFinished==0){
		adFinished=(ADC_CONTR&ADC_FLAG);
	}
	result=ADC_RES;
	ADC_CONTR &= !ADC_FLAG; 
	return result;
}
void getAdValue(){
	bit isLedDisplayOld=isLedDisplay;		
	stateValue=getAdValueByChannel(ADC_STATE_CHANNEL);
	stateAdFlag=1;
	delayMs(100);

	brightValue=getAdValueByChannel(ADC_BRIGHT_CHANNEL);
	brightAdFlag=1;
	if(brightValue>brightThreshold){
		isLedDisplay=1;	
	}
	else{
		isLedDisplay=0;
	}
	if(isLedDisplay!=isLedDisplayOld){
		displayChange=1;
	}
	else{
		displayChange=0;
	}
	delayMs(100);

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
				selAddrBit=(selAddrBit-1+3)%3;
				LCD_PutPosition(3+selAddrBit,3);
				Write_Cmd(0x0f);
		}
		else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING||menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
			selBrightThresholdBit=(selBrightThresholdBit-1+3)%3;
			LCD_PutPosition(3+selBrightThresholdBit,3);
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
				selAddrBit=(selAddrBit+1)%3;
				LCD_PutPosition(3+selAddrBit,3);
				Write_Cmd(0x0f);
		}
		else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING||menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
			selBrightThresholdBit=(selBrightThresholdBit+1)%3;
			LCD_PutPosition(3+selBrightThresholdBit,3);
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
		else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING||menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
				tempBrightThresholdArr[2*selBrightThresholdBit+1]='0'+((tempBrightThresholdArr[2*selBrightThresholdBit+1]-'0'-1+10)%10);
				LCD_PutString(3,3,tempBrightThresholdArr);
				LCD_PutPosition(3+selBrightThresholdBit,3);
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
		else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING||menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
				tempBrightThresholdArr[2*selBrightThresholdBit+1]='0'+((tempBrightThresholdArr[2*selBrightThresholdBit+1]-'0'+1)%10);
				LCD_PutString(3,3,tempBrightThresholdArr);
				LCD_PutPosition(3+selBrightThresholdBit,3);
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
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
					menuState=MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING;
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING){
					menuState=MENU_STATE_NOP;
					
					Write_Cmd(0x0C);//关闭游标及反白
					
				 	showMenu();
				}
				else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING_FINISHED){
					
					menuItemSeleted(menuIndex);
				}
				else{
					menuState=MENU_STATE_NOP;
					clearInfraredFlagIfNeccesary();
					inSetting=0;

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
			localAddress=(tempAddressArr[1]-'0')*100+(tempAddressArr[3]-'0')*10+(tempAddressArr[5]-'0');
			storageBaseInfo();
			Write_Cmd(0x0C);//关闭游标及反白
			LCD_PutString(2,4,"设置完毕!");
			menuState=MENU_STATE_IN_ADDR_SETTING_FINISHED;			
		}
		else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING||menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
			brightThreshold=(tempBrightThresholdArr[1]-'0')*100+(tempBrightThresholdArr[3]-'0')*10+(tempBrightThresholdArr[5]-'0');
			storageBaseInfo();
			Write_Cmd(0x0C);//关闭游标及反白
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
			tempAddressArr[2*selAddrBit+1]='0'+changeNum(IRcord[2]);
			LCD_PutString(3,3,tempAddressArr);;
			LCD_PutPosition(3+selAddrBit,3);
			Write_Cmd(0x0f);
		}else if(menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_SETTING||menuState==MENU_STATE_IN_BRIGHT_THRESHOLD_BIT_SETTING){
			tempBrightThresholdArr[2*selBrightThresholdBit+1]='0'+changeNum(IRcord[2]);
			LCD_PutString(3,3,tempBrightThresholdArr);;
			LCD_PutPosition(3+selBrightThresholdBit,3);
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

	
}
void main()
{

	retrieveBaseInfo();
	
	init();
	
 	while(1){
		tready=1;
  		rready=1;
		getAdValue();
	
		updateStaffInfoIfNeccesary();
		redisplayStaffInfoIfNecessary(0);
		displayADC();
//		displayBatState();
		keyScan();
		infraredDetection();
	}
}
