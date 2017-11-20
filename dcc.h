/*
###############################################################################
# DCC-commands
Command types  Prefix Description
-------------- ------ ------------------------------
*                     Ack
!                     Baseline commands
DC             000    Decoder and Consist Control Instruction
AO             001    Advanced Operation Instructions
SD             010    Speed and Direction Instruction for reverse operation
               011    Speed and Direction Instruction for forward operation
F1             100    Function Group One Instruction
F2             101    Function Group Two Instruction
EX             110    Future Expansion
CV             111    Configuration Variable Access Instruction

Type Description                                           Format
---- ----------------------------------------------------- ----------
DC!  Digital Decoder Reset Packet For All Decoders         @B   0x00
DC!  Digital Decoder Idle Packet For All Decoders          0xff 0x00
DC   Decoder Control - Digital Decoder Reset               @@   0b0000000D
            D = 0:  Digital Decoder Reset - A Digital Decoder Reset shall erase
                    all volatile memory (including and speed and direction
                    data), and return to its initial power up state as defined
                    in S-9.2.4 section A. Command Stations shall not send
                    packets to addresses 112-127 for 10 packet times following
                    a Digital Decoder Reset. This is to ensure that the decoder
                    does not start executing service mode instruction packets
                    as operations mode packets (Service Mode instruction
                    packets have a short address in the range of 112 to 127
                    decimal.)
            D = 1:  Hard Reset - Configuration Variables 29, 31 and 32 are
                    reset to its factory default conditions, CV#19 is set to
                    "00000000" and a Digital Decoder reset (as in the above
                    instruction) shall be performed.
DC   Decoder Control - Factory Test Instruction            @@   0b0000001X
            X = 0:  Disable factory test
            X = 1:  Enable factory test
            NOTE: For manufacturers and testing. It must not be sent by any
            command station during normal operation. This instruction may be a
            multi-byte instruction.
DC   Decoder Control - Set Decoder Flags                   @@   0b0000011D 0bCCCC0SSS
            SSS     Decoder subaddress (see CV#15)
            CCCC =
              0000: Disable 111 Instructions
              0100: Disable Decoder Acknowledgement Request Instruction 
              0101: Activate Bi-Directional Communications 
              1000: Set Bi-Directional Communications 
              1001: Set 111 Instruction 
              1111: Accept 111 Instructions 
            NOTE: This instruction is under re-evaluation by the NMRA DCC
            Working Group. Manufacturers should contact the NMRA DCC
            Coordinator before implementing this instruction.
DC   Decoder Control - Set Advanced Addressing (CV#29:5)   @@   0b0000101D
            D = 0:  Disable advanced addressing
            D = 1:  Enable advanced addressing
DC   Decoder Control - Decoder Acknowledgment Request      @@   0b0000111D
            D = 0:  Disable decoder acknowledgment
            D = 1:  Enable decoder acknowledgment
DC   Consist Control (activation or deactivation)          @@   0b0001001D 0b0AAAAAAA
            A{7}:   7-bit consist address (if A = 0 then consist is deactivated)
            D:      If 0 direction of this unit is normal-direction, elsewhere
                    the direction of this unit in the consist is opposite its
                    normal direction.


AO*  128 Speed Step Control                                @@   0b00111111 DSSSSSSS [ACK]
            D:      Direction
            SSSSSSS
              = 0:  Stop
              = 1:  Emergency stop
              *:    Speed (between 2 and 127)
            ACKED:  When operations mode acknowledgment is enabled, receipt of
                    this packet must be acknowledged with an operations mode
                    acknowledgement.
AO*  Restricted Speed Step Instruction                     @@   0b00111110 E0SSSSSS [ACK]
            E = 0:  Enable restricted speed
            E = 1:  Disable
            SSSSSS: Speed steps 6-bit (NMRA S-9.2)
            ACKED:  When operations mode acknowledgment is enabled, receipt of
                    this packet must be acknowledged with an operations mode
                    acknowledgement.
AO*  Analog Function Group                                 @@   0b00111101 VVVVVVVV DDDDDDDD [ACK]
            V{8}    Analog Function Output (0-255)
              = 1:  Volume Control
            D{8}:   Analog Function Data (0-255)
            ACKED:  When operations mode acknowledgment is enabled, receipt of
                    this packet must be acknowledged with an operations mode
                    acknowledgement.
              

SD!* Speed and Direction Packet For Loco Decos             @@   0b01DCSSSS [ACK]
            D:      direction
            (CV#29:5 == 0)
              | L:      control FL (headlights)
              | SSSS
                  0:  stop
                  1:  emergency stop
                  *:  14 speed steps
            (CV#29:5 == 1)
              | C:      additional speed bit (least significant)
              | SSSS:   speed
            BONUS:  If a decoder receives a new speed step that is within one
                    step of current speed step, the Digital Decoder may select
                    a step half way between these two speed steps. This
                    provides the potential to control 56 speed steps should the
                    command station alternate speed packets.
            COMBOBREAKER: Decoders may ignore the direction information
                    transmitted in a broadcast packet for Speed and Direction
                    commands that do not contain stop or emergency stop
                    information.
            ACKED:  When operations mode acknowledgment is enabled, receipt of
                    this packet must be acknowledged with an operations mode
                    acknowledgement.
SD!  Digital Decoder Broadcast Stop Packets For All Decos  @B   0b01DC000S
            S = 0:  stop all loco motors (C and D should be zero (ignored))
            S = 1:  stop delivering energy to loco motors
            C:      if 1 then bit D may be ignored by the decoder
            D:      direction


F1*  Function Group One Instruction                        @@   0b100XDCBA [ACK]
            A:      F1 (1 == enabled; 0 == disabled)
            B:      F2 (1 == enabled; 0 == disabled)
            C:      F3 (1 == enabled; 0 == disabled)
            D:      F4 (1 == enabled; 0 == disabled)
            (CV#29:5 == 0)
              | X:  Must be 0 (has no meaning)
            (CV#29:5 == 1)
              | X:  FL headlights (1 == enabled; 0 == disabled)
            ACKED:  When operations mode acknowledgment is enabled, receipt of
                    this packet must be acknowledged with an operations mode
                    acknowledgement.


F2*  Function Group Two Instruction                        @@   0b101XDCBA [ACK]
            (X == 0)
              | A:  F5  (1 == enabled; 0 == disabled)
              | B:  F6  (1 == enabled; 0 == disabled)
              | C:  F7  (1 == enabled; 0 == disabled)
              | D:  F8  (1 == enabled; 0 == disabled)
            (X == 1)
              | A:  F9  (1 == enabled; 0 == disabled)
              | B:  F10 (1 == enabled; 0 == disabled)
              | C:  F11 (1 == enabled; 0 == disabled)
              | D:  F12 (1 == enabled; 0 == disabled)
            ACKED:  When operations mode acknowledgment is enabled, receipt of
                    this packet must be acknowledged with an operations mode
                    acknowledgement.

EX*  Binary State Control Instruction long form            @@   0b11000000 DLLLLLLL HHHHHHHH [ACK]
            L{7}:   low order bits of the binary state address;
            H{8}:   high order bits of binary state address.
            D:      binary state
            NOTE:   Addresses range from 1 to 32767. The value of 0 for the
                    address is reserved as broadcast to clear or set all 32767
                    binary states.
            COMBOBREAKER: Binary state control packets (both short and long
                    form) will not be refreshed. Therefore non-volatile storage
                    of the function status is suggested.
            ACKED:  When operations mode acknowledgment is enabled, receipt of
                    this packet shall be acknowledged accordingly with an
                    operations mode acknowledgment. Consult the Technical
                    Note(s) for additional information on this instruction.
                    (See TN-4-05)
EX*  Binary State Control Instruction short form           @@   0b11011101 DLLLLLLL [ACK]
            L{7}:   low order bits of the binary state address;
            D:      binary state
            NOTE:   Addresses range from 1 to 127. The value of 0 for the
                    address is reserved as broadcast to clear or set all 127
                    binary states.
            COMBOBREAKER: Binary state control packets (both short and long
                    form) will not be refreshed. Therefore non-volatile storage
                    of the function status is suggested.
            ACKED:  When operations mode acknowledgment is enabled, receipt of
                    this packet shall be acknowledged accordingly with an
                    operations mode acknowledgment. Consult the Technical
                    Note(s) for additional information on this instruction.
                    (See TN-4-05)
EX*  F13-F20 Function Control                              @@   0b11011110 HGFEDCBA [ACK]
            A:      F13 (1 == enabled; 0 == disabled)
            B:      F14 (1 == enabled; 0 == disabled)
            C:      F15 (1 == enabled; 0 == disabled)
            D:      F16 (1 == enabled; 0 == disabled)
            E:      F17 (1 == enabled; 0 == disabled)
            F:      F18 (1 == enabled; 0 == disabled)
            G:      F19 (1 == enabled; 0 == disabled)
            H:      F20 (1 == enabled; 0 == disabled)
            NOTE:   It is recommended, but not required, that the status of
                    these functions be saved in decoder storage such as NVRAM.
                    It is not required, and should not be assumed that the
                    state of these functions is constantly refreshed by the
                    command station. Command Stations that generate these
                    packets, and which are not periodically refreshing these
                    functions, must send at least two repetitions of these
                    commands when any function state is changed.
            ACKED:  When operations mode acknowledgment is enabled, receipt of
                    this packet shall be acknowledged accordingly with an
                    operations mode acknowledgment. Consult the Technical
                    Note(s) for additional information on this instruction.
                    (See TN-4-05)
EX*  F21-F28 Function Control                              @@   0b11011111 HGFEDCBA [ACK]
            A:      F21 (1 == enabled; 0 == disabled)
            B:      F22 (1 == enabled; 0 == disabled)
            C:      F23 (1 == enabled; 0 == disabled)
            D:      F24 (1 == enabled; 0 == disabled)
            E:      F25 (1 == enabled; 0 == disabled)
            F:      F26 (1 == enabled; 0 == disabled)
            G:      F27 (1 == enabled; 0 == disabled)
            H:      F28 (1 == enabled; 0 == disabled)
            NOTE:   It is recommended, but not required, that the status of
                    these functions be saved in decoder storage such as NVRAM.
                    It is not required, and should not be assumed that the
                    state of these functions is constantly refreshed by the
                    command station. Command Stations that generate these
                    packets, and which are not periodically refreshing these
                    functions, must send at least two repetitions of these
                    commands when any function state is changed.
            ACKED:  When operations mode acknowledgment is enabled, receipt of
                    this packet shall be acknowledged accordingly with an
                    operations mode acknowledgment. Consult the Technical
                    Note(s) for additional information on this instruction.
                    (See TN-4-05)

CV   Configuration Variable Access Instruction-short form  @@   0b1111CCCC DDDDDDDD
            CCCC =
              0000: Not available for use
              0010: Acceleration Value (CV#23)
              0011: Deceleration Value (CV#24)
              1001: Service Mode Decoder Lock Instruction (See S-9.2.3, Appendix B)
            D{8}:   Data
            ACKED:  Only a single packet is necessary to change a configuration
                    variable using this instruction. If the decoder
                    successfully receives this packet, it shall respond with an
                    operations mode acknowledgment.
CV   Configuration Variable Access Instruction-long form   @@   0b1110CCVV VVVVVVVV DDDDDDDD
            CC =
              00:   Reserved for future use
              01:   Verify byte
              11:   Write byte
              10:   Bit manipulation
            V{10}:  Selected Configuration Variable
            (CC != 10)
              | D{8}: Data to be verified or writed
            (CC == 10)
              | D{8}  interpreted as 111CDBBB
              |         => BBB: bit position within the CV
              |         => D:   bit value to be verified or written
              |         => C:   1=write bit; 0=verify bit

Legend:
    @@ Address Byte      (0b0AAAAAAA)
        Addresses 0b00000001-0b01111111 (1-127):   7 bit addresses
        Addresses 0b10000000-0b10111111 (128-191): 9 bit addresses
        Addresses 0b11000000-0b11100111 (192-231): 14 bit addresses
        Addresses 0b11101000-0b11111110 (232-254): Reserved
        Address   0b11111111 (255): Idle Packet (unused)

    @B Broadcast Address (0x00)
        Address   0b00000000            (0): Broadcast address
*/

/*****************************************DCC Commands*****************************************/

//Advanced Operations Instruction (001) The format of this instruction is 001CCCCC 0 DDDDDDDD
#define DCC_ADVENCED_COMMAND 0b00100000
#define DCC_ADVENCED_SPEED_COMMAND 0b00011111

//Speed and Direction Instructions (010 and 011)  The format of this instruction is xxxDDDDD 
#define DCC_DIRECTION_SPEED_COMMAND 0b01000000
#define DCC_DIRECTION_FORWARD_COMMAND 0b00100000


//Function Group One Instruction (100), The format of this instruction is 100DDDDD 
#define DCC_GROUP_ONE_COMMAND 0b10000000

#define DDC_GROUP_ONE_FL 0b00010000
#define DDC_GROUP_ONE_F1 0b00000001
#define DDC_GROUP_ONE_F2 0b00000010
#define DDC_GROUP_ONE_F3 0b00000100
#define DDC_GROUP_ONE_F4 0b00001000


//Function Group Two Instruction (101), This instruction has the format 101SDDDD 
#define DCC_GROUP_TWO_COMMAND 0b10100000

#define DDC_GROUP_ONE_F5 0b00000001
#define DDC_GROUP_ONE_F6 0b00000010
#define DDC_GROUP_ONE_F7 0b00000100
#define DDC_GROUP_ONE_F8 0b00001000
#define DDC_GROUP_ONE_F9 0b00000001
#define DDC_GROUP_ONE_F10 0b0000010
#define DDC_GROUP_ONE_F11 0b0000100
#define DDC_GROUP_ONE_F12 0b0001000



//Configuration Variable Access Instruction (111)
#define DCC_CONFIGURATION_ACCESS 0b11100000



#define DCC_RESET_PACKET 0b00000000
#define DCC_IDLE_PACKET 0b11111111


//Service Mode Packets

#define DCC_INSTRUCTION_PACKET 0b01110000

#define DCC_INSTRUCTION_BIT_MANIPULATION 0b00001000
#define DCC_INSTRUCTION_VERIFY_BYTE 0b00000100
#define DCC_INSTRUCTION_WRITE_BYTE 0b00001100

#define INPUT_PIN           1
#define OUTPUT_PIN          0

#define DCC_PIN			PORTBbits.RB0
#define DCC_PIN_TRIS	TRISBbits.TRISB0

#define DCC_ACK_TRIS	TRISAbits.TRISA0
#define DCC_ACK			LATAbits.LATA0

#define LED_1			LATAbits.LATA1
#define LED_2			LATAbits.LATA2
#define LED_3			LATAbits.LATA3
#define LED_4			LATAbits.LATA4


/*************************DEFINE CV REGISTERS IN EEPROM*********************************/
#define CV_ADDRESS  0x00
#define CV_2		0x01	//Duty Cycle
#define CV_3		0x02	//Acceleration Value
#define CV_4		0x03	//Deceleration value
#define CV_5		0x04	//Enginee Address
#define CV_6		0x05	//Enable Enginee mode
#define CV_7		0x06	//Version Number
#define CV_8		0x07	//Factory number
#define CV_9		0x08	//F1 LedPatterLow
#define CV_10		0x09	//F1 LedPatterHigh
#define CV_11		0x0A	//F2 LedPatterLow
#define CV_12		0x0B	//F2 LedPatterHigh
#define CV_13		0x0C	//F3 LedPatterLow
#define CV_14		0x0D	//F3 LedPatterHigh
#define CV_15		0x0E	//F4 LedPatterLow
#define CV_16		0x0F	//F4 LedPatterHigh
#define CV_29		0x1C	//Config register
#define CV100		0x63	//Start bootloader if 0xAA is written

/***************************************** P U B L I C  P R O T O T Y P E S *****************************************/



#define MAX_DCC_DATA 64

typedef struct
{	unsigned char state;
	unsigned char actualbit;
	unsigned char bitcount;
	unsigned char bytecount;
	unsigned char data[64];
}DCC_REC;

typedef enum
{
	DCCSTAT_PREAMBLE = 0,
	DCCSTAT_ADDRESS,
	DCCSTAT_BYTE,
	DCCSTAT_EOF,
	DCCSTAT_ERROR
}DCC_STATES;

typedef struct
{
	unsigned char LocoAddress;
	unsigned char EngineeAddress;
	unsigned char EnableEngineeMode;
	unsigned int LedPattern1;
	unsigned int LedPattern2;
	unsigned int LedPattern3;
	unsigned int LedPattern4;
	unsigned char CV29;

}DCC_CONFIG;


void DCC_BitStartHandle();
void DCC_BitHandle();
void DCC_Init();
char DCC_CRC_Check(DCC_REC *DCCData);


//Railcom support??


