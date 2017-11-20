//main.c



/* --- configuration bits -------------------------------------------------- */
 #pragma config PLLDIV   = 6         //24 MHz crystal 
 #pragma config CPUDIV   = OSC1_PLL2   
 #pragma config USBDIV   = 2         // Clock source from 96MHz PLL/2
 #pragma config FOSC     = HSPLL_HS
#pragma config FCMEN    = OFF
#pragma config IESO     = OFF
#pragma config PWRT     = OFF
#pragma config BOR      = ON
#pragma config BORV     = 3
#pragma config VREGEN   = ON
#pragma config WDT      = OFF
#pragma config WDTPS    = 32768
#pragma config MCLRE    = ON
#pragma config LPT1OSC  = OFF
#pragma config PBADEN   = OFF
#pragma config CCP2MX   = OFF		//Alternate CCP2 pin
#pragma config STVREN   = ON
#pragma config LVP      = OFF
//#pragma config ICPRT    = OFF     // Dedicated In-Circuit Debug/Programming
#pragma config XINST    = OFF       // Extended Instruction Set
#pragma config CP0      = OFF
#pragma config CP1      = OFF
#pragma config CP2      = OFF
//#pragma config CP3      = OFF
#pragma config CPB      = OFF
#pragma config CPD      = OFF
#pragma config WRT0     = OFF
#pragma config WRT1     = OFF
#pragma config WRT2     = OFF
//#pragma config WRT3     = OFF
#pragma config WRTB     = OFF       // Boot Block Write Protection
#pragma config WRTC     = OFF
#pragma config WRTD     = OFF
#pragma config EBTR0    = OFF
#pragma config EBTR1    = OFF
#pragma config EBTR2    = OFF
//#pragma config EBTR3    = OFF
#pragma config EBTRB    = OFF

/** I N C L U D E S **********************************************************/
#include <p18cxxx.h>
#include <stdio.h>
#include "led.h"
#include "dcc.h" 

/** DEFINES **********************************************************/




/** P R I V A T E  P R O T O T Y P E S ***************************************/
void YourHighPriorityISRCode();
void YourLowPriorityISRCode();
void __delay_ms(unsigned int ms);
void initSPI();
void SetLed(unsigned int Leds);
void ReadEeprom(unsigned char *Data, unsigned char Address, unsigned char Len);
void WriteEeprom(unsigned char *Data, unsigned char Address, unsigned char Len);

/** V E C T O R  R E M A P P I N G *******************************************/
#define REMAPPED_RESET_VECTOR_ADDRESS			0x1400
#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1408
#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1418
extern void _startup (void);        
#pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
void _reset (void)
{
    _asm goto _startup _endasm
}

#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
void Remapped_High_ISR (void)
{
     _asm goto YourHighPriorityISRCode _endasm
}

#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
void Remapped_Low_ISR (void)
{
     _asm goto YourLowPriorityISRCode _endasm
}

#pragma code HIGH_INTERRUPT_VECTOR = 0x08
void High_ISR (void)
{
     _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
}
#pragma code LOW_INTERRUPT_VECTOR = 0x18
void Low_ISR (void)
{
     _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
}

#pragma code


//Actual interrupt handling routines.
#pragma interrupt YourHighPriorityISRCode
void YourHighPriorityISRCode()
{
	if(INTCONbits.INT0IF == 1)
	{
		DCC_BitStartHandle();
		INTCONbits.INT0IF = 0;
	}
	if(INTCONbits.TMR0IF == 1)
	{
		DCC_BitHandle();
		INTCONbits.TMR0IF = 0;
	}
}
#pragma interruptlow YourLowPriorityISRCode
void YourLowPriorityISRCode()
{


}	


/** D E C L A R A T I O N S **************************************************/

#pragma udata
unsigned int i;
extern unsigned int LedPattern;

/** MAIN **************************************************/
#pragma code


void main(void)	
{
	ADCON1 = 0x0F;				//Analog inputs as digital I/O
	TRISA = 0;
	TRISB = 0;
	LED_Init();
	DCC_Init();
	INTCONbits.PEIE = 1;		//Enable Periphal interrupt
	INTCONbits.GIE = 1;			//Enable global interrupt

	while(1)
	{
		SetLed(LedPattern);
		__delay_ms(10);
	}

}

void __delay_ms(unsigned int ms)
{
	do
	{
		Delay10KTCYx(1);
	}while(ms--);
}







