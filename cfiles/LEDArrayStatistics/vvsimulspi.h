#ifndef __VV_SIMULSPI_H__
#define __VV_SIMULSPI_H__
#include <reg52.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned long

#define uchar unsigned char
#define uint unsigned int

sbit _CS = P1^3;            
sbit SCK = P1^7;            
sbit MISO = P1^6;           
sbit MOSI = P1^5;           
#define BUFFSIZE 16

void init_spi(void);

void WriteByte(u8 temp);

u8 ReadByte(void);
#endif
