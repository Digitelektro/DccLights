#include <p18cxxx.h>
#include <string.h>
#include "dcc.h" 
#include "eeprom.h"
#include "led.h"

void DCC_Command(DCC_REC *DCCData);
void DCC_GroupOne(unsigned char Command);
void DCC_GroupTwo(unsigned char Command);
void DCC_SpeedCommdand(unsigned char Direction, unsigned char Speed);
void DCCLoadConfiguration(void);
char DCC_CRC_Check(DCC_REC *DCCData);




#pragma udata
DCC_REC dcc_rec;
DCC_CONFIG dcc_config;
unsigned char bitmask;
unsigned char successwrite = 0;
unsigned char EnableServiceMode;
unsigned char temp;
unsigned int LedPattern;
unsigned char flEnabled;


#pragma udata
unsigned char Config[64];

#pragma code
void DCC_Init()
{
	DCCLoadConfiguration();
	DCC_ACK_TRIS = OUTPUT_PIN;  //DCC Acknowledge pin
	DCC_PIN_TRIS = INPUT_PIN;
	DCC_ACK = 0;	
	TRISBbits.TRISB2 = 0;

	INTCON2bits.INTEDG0 = 1;	//INT0 interrupt on rising edge
	INTCONbits.INT0IF = 0;		//Clear interrupt flag
	INTCONbits.INT0IE = 1;		//Enable INT0 interrupt


	T0CON = 0;
	T0CONbits.T0PS1 = 1;		//Timer0 8 prescaler
	T0CONbits.T08BIT = 1;		//Timer0 8bit mode
	INTCONbits.TMR0IF = 0;		//Clear interrupt flag
	INTCON2bits.TMR0IP = 1;		//Timer0 High Priority
	INTCONbits.TMR0IE = 1;		//Enable Timer0 interrupt

	INTCONbits.PEIE = 1;		//Enable Periphal interrupt
	INTCONbits.GIE = 1;			//Enable global interrupt

	dcc_rec.state = DCCSTAT_PREAMBLE;
	dcc_rec.actualbit = 0;
	dcc_rec.bitcount = 0;
	dcc_rec.bytecount = 0;
}

char DCC_CRC_Check(DCC_REC *DCCData)
{
	unsigned char crc,i;
	crc =  0;
	for(i = 0; i < DCCData->bytecount; i++)
	{
		crc ^= DCCData->data[i];
	}
	if(DCCData->data[DCCData->bytecount] == crc)
		return 1;
	else
		return 0;
}


void DCCLoadConfiguration(void)
{
	ReadEeprom(&dcc_config.LocoAddress,CV_ADDRESS,1);
	ReadEeprom(&dcc_config.EngineeAddress,CV_5,1);
	ReadEeprom(&dcc_config.EnableEngineeMode,CV_6,1);
	ReadEeprom(&dcc_config.LedPattern1,CV_9,2);
	ReadEeprom(&dcc_config.LedPattern2,CV_11,2);
	ReadEeprom(&dcc_config.LedPattern3,CV_13,2);
	ReadEeprom(&dcc_config.LedPattern4,CV_15,2);
	ReadEeprom(&dcc_config.CV29,CV_29,1);
	ReadEeprom(&temp,CV_2,1);
	SetDutyCycle(temp);
	LedPattern = 0;
}

#pragma tmpdata ISRtmpdata
void DCC_BitStartHandle()
{
	TMR0L = 150;			//75uS time
	T0CONbits.TMR0ON = 1; 	//Timer0 enable
	//LATAbits.LATA2 = 1;	//Debug pin
}
#pragma tmpdata

#pragma tmpdata ISRtmpdata

void DCC_BitHandle()
{
	//LATAbits.LATA2 = 0;	//Debug pin
	T0CONbits.TMR0ON = 0;	//Timer0 disable
	if(DCC_PIN == 1)
	{
		dcc_rec.actualbit = 0;
	}
	else
	{
		dcc_rec.actualbit = 128;
	}
	switch(dcc_rec.state)
	{
		case DCCSTAT_PREAMBLE:
		{
			dcc_rec.bitcount++;
			if(dcc_rec.bitcount >= 10 && dcc_rec.actualbit == 0)
			{
				dcc_rec.state = DCCSTAT_ADDRESS;
				dcc_rec.bitcount = 0;
			}
			else if(dcc_rec.bitcount < 10 && dcc_rec.actualbit == 0)
			{
				dcc_rec.bitcount = 0;
			}

			break;
		}
		case DCCSTAT_ADDRESS:
		{
				
			if(dcc_rec.bitcount < 8)
			{
				bitmask = (0b10000000 >> dcc_rec.bitcount);
				dcc_rec.data[dcc_rec.bytecount] = (dcc_rec.data[dcc_rec.bytecount] & (~bitmask)) | (dcc_rec.actualbit >> dcc_rec.bitcount);
				dcc_rec.bitcount++;
			}
			else
			{
				dcc_rec.bitcount = 0;
				dcc_rec.bytecount = 1;
				if(dcc_rec.actualbit == 0)
					dcc_rec.state = DCCSTAT_BYTE;
			}
			break;
		}
		case DCCSTAT_BYTE:
		{
			if(dcc_rec.bitcount < 8)
			{
				bitmask = (0b10000000 >> dcc_rec.bitcount);
				dcc_rec.data[dcc_rec.bytecount] = (dcc_rec.data[dcc_rec.bytecount] & (~bitmask)) | (dcc_rec.actualbit >> dcc_rec.bitcount);
				dcc_rec.bitcount++;
			}
			else
			{
				dcc_rec.bitcount = 0;
				if(dcc_rec.actualbit == 128)
				{	
					dcc_rec.state = DCCSTAT_EOF;
				}		
				else
				{
					dcc_rec.bytecount++;
				}
			}
			break;
		}
		case DCCSTAT_EOF:
		{
			if(DCC_CRC_Check(&dcc_rec) == 1)	//Error check
				DCC_Command(&dcc_rec);
			memset(dcc_rec.data, 0, sizeof(dcc_rec.data));		
			dcc_rec.bytecount = 0;
			dcc_rec.state = DCCSTAT_PREAMBLE;
			break;
		}
		case DCCSTAT_ERROR:
		{
			memset(dcc_rec.data, 0, sizeof(dcc_rec.data));		
			dcc_rec.bytecount = 0;
			dcc_rec.state = DCCSTAT_PREAMBLE;
			break;
		}
	}
	
}
#pragma tmpdata


void DCC_Command(DCC_REC *DCCData)
{
	int n;
	if((EnableServiceMode == 1) && ((DCCData->data[0] & 0b11110000) == DCC_INSTRUCTION_PACKET))
	{
			switch(DCCData->data[0] & 0b00001100)
			{
				case DCC_INSTRUCTION_WRITE_BYTE:
				{
					unsigned char Data = DCCData->data[2];
					WriteEeprom(&Data, DCCData->data[1], 1);
					DCC_ACK = 1;
					Delay10KTCYx(10);
					DCC_ACK = 0;	
					break;	
				}
				case DCC_INSTRUCTION_VERIFY_BYTE:
				{
					unsigned char readed;
					ReadEeprom(&readed, DCCData->data[1], 1);
					if(DCCData->data[2] == readed)
					{
						DCC_ACK = 1;
						Delay10KTCYx(10);
						DCC_ACK = 0;
					}
					break;
				}
				case DCC_INSTRUCTION_BIT_MANIPULATION:
				{
					unsigned char readed;
					unsigned char bitpos = DCCData->data[2] & 0b00000111;
					unsigned char Bit = (DCCData->data[2] & 0b00001000) >> 3;
					ReadEeprom(&readed, DCCData->data[1], 1);
					if(DCCData->data[2] & 0b00010000)							//bit write NOT TESTED yet!!!
					{
						readed = readed & ~(1 << bitpos) | (Bit << bitpos);
						WriteEeprom(&readed, DCCData->data[1], 1);
						DCC_ACK = 1;
						Delay10KTCYx(10);
						DCC_ACK = 0;
					}	
					else														//bit verify
					{
						if(((readed >> bitpos) & 0b00000001) == Bit)
						{
							DCC_ACK = 1;
							Delay10KTCYx(10);
							DCC_ACK = 0;
						}
					} 
					break;
				}
		}
		EnableServiceMode == 0;
	}
	else
	{
		EnableServiceMode == 0;
		switch(DCCData->data[1] & 0b11100000)
		{
			case DCC_CONFIGURATION_ACCESS:
			{
				if(dcc_config.LocoAddress != DCCData->data[0])
					return;
				if((DCCData->data[1] & 0b00001100) == DCC_INSTRUCTION_WRITE_BYTE)	
				{
					unsigned char Data = DCCData->data[3];
					WriteEeprom(&Data, DCCData->data[2], 1);
					DCCLoadConfiguration();
				}
				break;
			}
			case DCC_ADVENCED_COMMAND:
			{
				if(dcc_config.EngineeAddress != DCCData->data[0])
					return;
				if(DCCData->data[1] & 0b00011111 & DCC_ADVENCED_SPEED_COMMAND)	//Speed command
				{
					if(DCCData->data[2] & 0b10000000)							//Direction command
					{
						DCC_SpeedCommdand(1, 0b01111111 & DCCData->data[2]);
					}
					else
					{
						DCC_SpeedCommdand(0, 0b011111 & DCCData->data[2]);
					}
				}

				break;
			}
			case DCC_DIRECTION_SPEED_COMMAND:
			{
				if(dcc_config.EngineeAddress != DCCData->data[0])
					return;
				if(DCCData->data[1] & DCC_DIRECTION_FORWARD_COMMAND)		//Forward?
				{
					DCC_SpeedCommdand(1, 0b00001111 & DCCData->data[2]);
				}
				else
				{	
					DCC_SpeedCommdand(0, 0b00001111 & DCCData->data[2]);
				}
				break;
			}
			case DCC_GROUP_ONE_COMMAND:
			{
				if(dcc_config.LocoAddress != DCCData->data[0])
					return;
				DCC_GroupOne(DCCData->data[1] & 0b00011111);

				break;
			}

			case DCC_GROUP_TWO_COMMAND:
			{
				if(dcc_config.LocoAddress != DCCData->data[0])
					return;
				DCC_GroupTwo(DCCData->data[1] & 0b00011111);
				break;
			}
			case DCC_RESET_PACKET:
			{
				if(DCCData->data[0] != 0)
					return;
				EnableServiceMode = 1;
				break;
			}
		}
	}
	
}



void DCC_GroupOne(unsigned char Command)
{
	if(Command & DDC_GROUP_ONE_FL)
	{
		if(dcc_config.EnableEngineeMode == 0)
		{
			flEnabled = 1;
			LED_4 = 1;
			LED_3 = 1;
			LED_2 = 1;
		}
	}
	else
	{
		if(dcc_config.EnableEngineeMode == 0)
		{
			if(flEnabled == 1)
			{
				LED_4 = 0;
				LED_3 = 0;
				LED_2 = 0;
			}
			flEnabled = 0;
			
		}
	}
	if(Command & DDC_GROUP_ONE_F1)
	{
		LedPattern |= dcc_config.LedPattern1;	
	}
	else
	{
		LedPattern &= ~dcc_config.LedPattern1;	
	}
	if(Command & DDC_GROUP_ONE_F2)
	{
		LedPattern |= dcc_config.LedPattern2;	
	}
	else
	{
		LedPattern &= ~dcc_config.LedPattern2;	
	}
	if(Command & DDC_GROUP_ONE_F3)
	{
		LedPattern |= dcc_config.LedPattern3;	
	}
	else
	{
		LedPattern &= ~dcc_config.LedPattern3;	
	}
	if(Command & DDC_GROUP_ONE_F4)
	{
		LedPattern |= dcc_config.LedPattern4;	
	}
	else
	{
		LedPattern &= ~dcc_config.LedPattern4;	
	}

}

void DCC_GroupTwo(unsigned char Command)
{
	if(Command & 0b00010000) 		//F5-F8 keys
	{
		if(Command & DDC_GROUP_ONE_F5)
		{
			if(dcc_config.EnableEngineeMode == 0)
			{
				LED_1 = 1;
			}
		}
		else
		{
			if(dcc_config.EnableEngineeMode == 0)
			{
				LED_1 = 0;
			}
		}
		if(Command & DDC_GROUP_ONE_F6)
		{
			if(dcc_config.EnableEngineeMode == 0 && flEnabled == 0)
			{
				LED_2 = 1;
			}
		}
		else
		{
			if(dcc_config.EnableEngineeMode == 0 && flEnabled == 0)
			{
				LED_2 = 0;
			}
		}
		if(Command & DDC_GROUP_ONE_F7)
		{
			if(dcc_config.EnableEngineeMode == 0 && flEnabled == 0)
			{
				LED_3 = 1;
			}
		}
		else
		{
			if(dcc_config.EnableEngineeMode == 0 && flEnabled == 0)
			{
				LED_3 = 0;
			}
		}
		if(Command & DDC_GROUP_ONE_F8)
		{
			if(dcc_config.EnableEngineeMode == 0 && flEnabled == 0)
			{
				LED_4 = 1;
			}
		}
		else
		{
			if(dcc_config.EnableEngineeMode == 0 && flEnabled == 0)
			{
				LED_4 = 0;
			}
		}
	}
	else	//F9-F12
	{
		
	}

}

void DCC_SpeedCommdand(unsigned char Direction, unsigned char Speed)
{
	if(dcc_config.EnableEngineeMode > 0)
	{
		if(Direction)
		{
			if(dcc_config.CV29 & 0b00000001)	//Reverse direction
			{
				LED_4 = 0;
				LED_3 = 0;
				LED_2 = 0;
				LED_1 = 1;
			}
			else
			{
				LED_4 = 1;
				LED_3 = 1;
				LED_2 = 1;
				LED_1 = 0;

			}
		}
		else
		{
			if(dcc_config.CV29 & 0b00000001)	//Reverse direction
			{
				LED_4 = 1;
				LED_3 = 1;
				LED_2 = 1;
				LED_1 = 0;
			}
			else
			{
				LED_4 = 0;
				LED_3 = 0;
				LED_2 = 0;
				LED_1 = 1;
			}
		}
	}
}