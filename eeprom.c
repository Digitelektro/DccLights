#include <p18cxxx.h>
#include "eeprom.h"

void ReadEeprom(unsigned char *Data, unsigned char Address, unsigned char Len)
{
	unsigned char i;
	EECON1 = 0;
	for(i = 0; i < Len; i++)
	{
		EEADR = Address + i;
		EECON1bits.RD = 1;
		while (EECON1bits.RD);
		*Data = EEDATA;
		Data++;
	}
}

void WriteEeprom(unsigned char *Data, unsigned char Address, unsigned char Len)
{
	unsigned char i;
	EECON1 = 0;
	EECON1bits.WREN = 1;
	for(i = 0; i < Len; i++)
	{
		EEADR = Address + i;
		EEDATA = *Data;
		//INTCONbits.GIE = 0;
		EECON2 = 0x55;
		EECON2 = 0xAA;
		EECON1bits.WR = 1;
		//INTCONbits.GIE = 1;
		while (EECON1bits.WR == 1);
		Data++;
	}
}