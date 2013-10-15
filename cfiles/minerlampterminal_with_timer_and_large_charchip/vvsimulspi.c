#include "vvsimulspi.h"

void init_spi(void)
{
	
}

void WriteByte(u8 temp)
{
	uchar  i;
    for(i=0; i<8; i++)        
    {
        SCK = 0;    
        temp <<= 1;
        MOSI = CY;            
        SCK = 1;
    }
}

u8 ReadByte(void)
{
	uchar  i,inData;
    for(i=0; i<8; i++)        
    {    
        SCK = 0;       
        inData <<= 1;    
        inData |= MISO;    
        SCK = 1;        
    }
    return(inData);

}

