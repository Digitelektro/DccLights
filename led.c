#include <p18cxxx.h>
#include "led.h"

void LED_Init()
{
	TRISBbits.TRISB1 = 0;	//SCK output
	TRISCbits.TRISC7 = 0;	//SDO output
	TRISCbits.TRISC6 = 0;	//LAT output
	LED_LAT = 0;
	SSPSTAT = 0;
	SSPSTATbits.CKE = 0;
	SSPCON1 = 0b00100010;	//SPI Master mode enable, clock is FOSC/64

	//PWM Setting for 8 bit
	T2CON = 0;
	T2CONbits.TMR2ON = 1;
	PR2 = 0xFF;
	CCPR2L = 0x16;
	CCP2CON = 0x0C;	//PWM Mode ON
	SetLed((unsigned int)0x0000);

}

void SetLed(unsigned int Leds)
{
	static unsigned char dummy;
	SSPBUF = Leds & 0b11111111;
	while(!SSPSTATbits.BF);
	dummy = SSPBUF;
	dummy = SSPBUF;
	SSPBUF = (Leds >> 8) & 0b11111111;
	while(!SSPSTATbits.BF);
	dummy = SSPBUF;
	dummy = SSPBUF;
	LED_LAT = 1;
	Delay10KTCYx(1);
	LED_LAT = 0;
}

void SetDutyCycle(unsigned char Duty)
{
	CCPR2L = 255 - Duty;
}