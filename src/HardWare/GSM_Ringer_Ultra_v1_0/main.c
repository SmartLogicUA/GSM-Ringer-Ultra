/*****************************************************
This program was produced by the
CodeWizardAVR V1.25.3 Professional
Automatic Program Generator
© Copyright 1998-2007 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : GSM Ringer Ultra
Version : v1.0
Date    : 20.06.2010
Author  : T. Drozdovskiy
Company : Smart Logic
Comments: 


Chip type           : ATmega8L
Program type        : Application
Clock frequency     : 7,372800 MHz
Memory model        : Small
External SRAM size  : 0
Data Stack size     : 256
*****************************************************/

#include <mega8.h>
#include <board.h>                                
#include <sim300.h>
#include <delay.h>                     
#include <string.h>
#include <stdlib.h>
#include <ctype.h> 

#define DELAY_C         45                               // время в сек. на исходящие соединение
#define DELAY_CC        120                              // время в сек. на входящее соединение
#define DELAYMC240      7                                // количество стробов на АЦП
#define NUM_STROB_VOL   250 

#define NUM_SEC_OD_ON   60 

#define NUMCALL         5                                // количество попыток дозвона
#define AMOUNT_ATTEMPT_SMS  10                           // количество попыток отправки SMS

#define F_VERSIA            "GSM Ringer Ultra v1.0\r\ns/n 00003"      // версия програмного обеспечения

eeprom char str_zoneE[3][12]={"Zone1","Zone2","Zone3"}; 
// Declare variable setting in EEPROM and initial value             

eeprom unsigned char TermResE = 0x08;
eeprom unsigned char BuzOnE = 0x0C;                     // статус зоны при активированной системе
eeprom unsigned char SETSE = 10;                        // время активации системы
eeprom unsigned char SMSE=0x00;                         // параметры оповещения по SMS
eeprom unsigned char GETSE = 10;                        // время деактивации системы
eeprom unsigned char DELAY_ZUME = NUM_SEC_OD_ON;
eeprom unsigned char CALE = 0x00;                         // параметры оповещиния по Call
eeprom unsigned char ZoneMaskE=0x07;                    // маска зон
eeprom unsigned char ZonaE=0;                           // зоны которые сработали
eeprom unsigned char sign_E=0;                          
eeprom unsigned char NUMAE[NUMA_NUMBER][11]={"0000000000",             // номера дозвонов
                                             "0000000000",
                                             "0000000000",
                                             "0000000000"};              
eeprom unsigned char PWDE[]="000000";

#define MIN_VOLTAGE     0x65
#define MAX_VOLTAGE     0x94
#define MAX_VOLTERMRES  0x68

// Declare your variables in RAM here
register unsigned int  CounterGSM=0;                    // счетчик тиков для модема
register unsigned char ZonaS=0;                         // текущее состояние зон
register unsigned char TimeGuard=0;                  
unsigned char TermRes;
unsigned char SETSR=0;                                  // время для активации системы   
unsigned char GETSR;                 
                   // время на дективацию системы 
unsigned char CounterOpenDrain1_off = 0;
unsigned char NUMAR[NUMA_NUMBER][11];                   // номера абонентов
unsigned char rdy_counter=25;                           // время ожидания CRDY
unsigned char work_modem=120;                           // время до подачи команды АТ
unsigned char ZoneMask;                                 // маска рабочих зон
unsigned int  gsm_counter_AC=0;                         // время активного звонка
unsigned char StatusCall=0;                             // состояние дозвона абонентам
unsigned char StatusSMS=0;                              // состояние отправки SMS абонентам
unsigned char AttemptSMS=0;                             // количество попыток отправки SMS
unsigned char AttemptCall=0;                            // количество попыток дозвона
unsigned char ChangeRegNetwork=240;                     // время проверки регистрации
unsigned char Zona=0; 
unsigned char BuzOn;                                    // статус зоны при активированной системе
unsigned char RPosib = 0;
unsigned char Power = 0;                                    // ?????? ???? ??? ?????????????? ???????

bit RCallRDY=0;                                         // прохождение сообщения готовности модема
bit sign=0;                                             // состояние системы (активирована/деактивирована)

void SendAnswer(flash char *);                          // отправка строки с Flash памяти
void SetFactorySetting(void);                           // установка настроек от производителя
void eeprom2ram(void);                                  // считывание параметров с EEPROM в RAM
void SendInfo(void);                                    
unsigned char SMSF(void);
unsigned char SMSP(void);
unsigned char F_Ring(void);                             // ????? ????????? ??????
unsigned char F_SMS(void);                              // ????? ???????? SMS
unsigned char F_Crdy(void);                             // ????????????? GSM ??????
void SendCall(void);                                    // ???????? ?????????? ????????
unsigned char cmp_digit(unsigned char *,unsigned char);

unsigned char counter_adc1_m=0;
unsigned char counter_adc1_l=0;
bit level_adc1_l=0;
bit level_adc1_m=0;
                                         
unsigned char counter_adc2_m=0;
unsigned char counter_adc2_l=0;
bit level_adc2_l=0;
bit level_adc2_m=0;

unsigned char counter_adc3_m=0;
unsigned char counter_adc3_l=0;
bit level_adc3_l=0;
bit level_adc3_m=0;

unsigned char counter_but1_l=0;            
bit level_but1_l=0;            
unsigned char counter_but1_h=0;            
bit level_but1_h=0;

unsigned char counter_vol_l=0;                         // ??????? ???????????? ??????? ??????
unsigned char counter_vol_h=0;                         // ??????? ???????????? ???????? ??????
bit level_vol_l = 0;                                     // ???? ??????? ??????
bit level_vol_h = 0;                                     // ???? ???????? ??????            

bit SInfo=0;
bit CurrentNetwork=0;                           
bit sign_on=0;

#define RXB8 1
#define TXB8 0
#define UPE 2
#define OVR 3
#define FE 4
#define UDRE 5
#define RXC 7

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)

// USART Receiver buffer
#define RX_BUFFER_SIZE 240
char rx_buffer[RX_BUFFER_SIZE];

#if RX_BUFFER_SIZE<256
unsigned char rx_wr_index,rx_rd_index,rx_counter;
#else
unsigned int rx_wr_index,rx_rd_index,rx_counter;
#endif

// This flag is set on USART Receiver buffer overflow
bit rx_buffer_overflow;

// USART Receiver interrupt service routine
interrupt [USART_RXC] void usart_rx_isr(void)
{
char status,data;
status=UCSRA;
data=UDR;
if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
   {
   TimeGuard=3;
   rx_buffer[rx_wr_index]=data;
   if (++rx_wr_index == RX_BUFFER_SIZE) rx_wr_index=0;
   if (++rx_counter == RX_BUFFER_SIZE)
      {
      rx_counter=0;
      rx_buffer_overflow=1;
      };
   };
}

#ifndef _DEBUG_TERMINAL_IO_
// Get a character from the USART Receiver buffer
#define _ALTERNATE_GETCHAR_
#pragma used+
char getchar(void)
{
char data;
while (rx_counter==0);
data=rx_buffer[rx_rd_index];
if (++rx_rd_index == RX_BUFFER_SIZE) rx_rd_index=0;
#asm("cli")
--rx_counter;
#asm("sei")
return data;
}
#pragma used-
#endif

// USART Transmitter buffer
#define TX_BUFFER_SIZE 64
char tx_buffer[TX_BUFFER_SIZE];

#if TX_BUFFER_SIZE<256
unsigned char tx_wr_index,tx_rd_index,tx_counter;
#else
unsigned int tx_wr_index,tx_rd_index,tx_counter;
#endif

// USART Transmitter interrupt service routine
interrupt [USART_TXC] void usart_tx_isr(void)
{
if (tx_counter)
   {
   --tx_counter;
   UDR=tx_buffer[tx_rd_index];
   if (++tx_rd_index == TX_BUFFER_SIZE) tx_rd_index=0;
   };
}

#ifndef _DEBUG_TERMINAL_IO_
// Write a character to the USART Transmitter buffer
#define _ALTERNATE_PUTCHAR_
#pragma used+
void putchar(char c)
{
while (tx_counter == TX_BUFFER_SIZE);
#asm("cli")
if (tx_counter || ((UCSRA & DATA_REGISTER_EMPTY)==0))
   {
   tx_buffer[tx_wr_index]=c;
   if (++tx_wr_index == TX_BUFFER_SIZE) tx_wr_index=0;
   ++tx_counter;
   }
else
   UDR=c;
#asm("sei")
}
#pragma used-
#endif

// Standard Input/Output functions
#include <stdio.h>
                
// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
// Reinitialize Timer 0 value
TCNT0=0xB8;
// Place your code here
 if(CounterGSM) CounterGSM--;
 if(TimeGuard)TimeGuard--;
 if(BUT)
 {
    counter_but1_h = 0;
    if(!level_but1_l)
    {
        if(counter_but1_l >= DELAYMC240) 
        {
            level_but1_l = 1;
            level_but1_h = 0;
            if(sign_on)
            {          
                sign_on = 0;      
                SETSR = 0;    
                CounterOpenDrain1_off = 1;
                if(sign)
                {
                    Zona = 0;
                    ZonaE = 0;    
                }
            }
            else
            {
                if(TermRes&(1<<3))
                {
                    sign_on = 1; //set
                    RPosib = 1;
                    GETSR = 0;
                    if(!SInfo)
                    {   
                        Zona = 0;
                        ZonaE = 0;                
                    }
                }                
            }
        }
        else
        {
            counter_but1_l++;    
        }    
    }
 }            
 else
 {           
    counter_but1_l=0;            
    if(!level_but1_h)
    {
        if(counter_but1_h>=DELAYMC240) 
        {
            level_but1_h=1;
            level_but1_l=0;            
            if(!(TermRes&(1<<3)))
            {
                sign_on=1; //set
                RPosib = 1;
                GETSR=0;
                if(!SInfo)
                {
                    Zona=0;
                    ZonaE=0;                
                }
            }
        }
        else
        {
            counter_but1_h++;    
        }    
   }
 }
 if(VOL_PIN)
 {
    counter_vol_h=0;
    if(!level_vol_l)
    {
        if(counter_vol_l>=NUM_STROB_VOL) 
        {
            level_vol_l=1;
            level_vol_h=0;
            Power = 1;
            #ifdef DEBUG                                                                        
            SendAnswer("\r\nAppear ~220v");
            #asm("cli") 
            #endif
            AttemptSMS=AMOUNT_ATTEMPT_SMS;
        }
        else
        {
            counter_vol_l++;    
        }    
    }
 }            
 else
 {           
    counter_vol_l=0;            
    if(!level_vol_h)
    {
        if(counter_vol_h>=NUM_STROB_VOL) 
        {
            level_vol_h=1;
            level_vol_l=0;            
            Power = 2;                    
            #ifdef DEBUG                                                                        
            SendAnswer("\r\nDisAppear ~220v");
            #asm("cli") 
            #endif
            AttemptSMS=AMOUNT_ATTEMPT_SMS;
        }
        else
        {
            counter_vol_h++;    
        }    
    }
 }

// Start the AD conversion
ADCSRA|=0x40;             
}

// Timer 1 output compare A interrupt service routine
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
// Place your code here
 LED_ON       
 if(CounterOpenDrain1_off)
 {
    CounterOpenDrain1_off--;
    if(!CounterOpenDrain1_off) OPENDRAIN1_OFF
 }                  
 if(gsm_counter_AC) gsm_counter_AC--; 
 if(!sign)
 {
    if(sign_on&&(RPosib))
    {
        if(!SETSR)
        {
            if(((ZonaS&ZoneMask)==ZoneMask)&&(ZoneMask)&&(!Zona))
               SETSR=SETSE; 
        }
        else
        {           
            if(BuzOn&(1<<2)) OPENDRAIN1_TOG
            SETSR--;
            if(!SETSR)
            {         
                if(((ZonaS&ZoneMask)==ZoneMask)&&(ZoneMask)&&(!Zona)) 
                {
                    sign=1;
                    sign_E=1;
                    OPENDRAIN1_OFF
                }
                else
                {                 
                    CounterOpenDrain1_off = DELAY_ZUME;
                    if(CounterOpenDrain1_off)OPENDRAIN1_ON
                }
            }     
        }
    }
 }
 else
 {
    if(Zona&ZoneMask)
    {
        if(sign_on)
        {
            if(!GETSR)
            {
                GETSR=GETSE; 
            }
            else
            {
                if(BuzOn&(1<<3)) OPENDRAIN1_TOG
                GETSR--;
                if(!GETSR)
                {
                    SInfo=1;
                    StatusSMS=SMSE;
                    StatusCall=CALE;
                    AttemptCall=NUMCALL;   
                    AttemptSMS=AMOUNT_ATTEMPT_SMS;
                    sign=0;
                    RPosib = 0;
                    sign_E=0;                                        
                    CounterOpenDrain1_off = DELAY_ZUME;
                    if(CounterOpenDrain1_off)OPENDRAIN1_ON
                }   
            }
        }
    }
    else
    {
        if(!sign_on)
        {
            sign=0;
            sign_E=0;
        }
        OPENDRAIN1_OFF

    }   
 }    
 if(!RCallRDY) 
     if(rdy_counter) rdy_counter--;
 if(work_modem) work_modem--;
 if(ChangeRegNetwork) ChangeRegNetwork--;
}

// Timer 1 output compare B interrupt service routine
interrupt [TIM1_COMPB] void timer1_compb_isr(void)
{
// Place your code here
 if(!sign) LED_OFF
}

// USART Transmitter buffer
#define TX_BUFFER_SIZE1 40
char tx_buffer1[TX_BUFFER_SIZE1];

#if TX_BUFFER_SIZE1<256
unsigned char tx_wr_index1,tx_rd_index1,tx_counter1;
#else
unsigned int tx_wr_index1,tx_rd_index1,tx_counter1;
#endif

unsigned char UartswTxData;
unsigned char UartswTxBitNum;
bit UartswTxBusy=0; 

// Timer 2 output compare interrupt service routine
interrupt [TIM2_COMP] void timer2_comp_isr(void)
{
// Place your code here
	if(UartswTxBitNum)
	{
		if(UartswTxBitNum > 1)
		{
			if(UartswTxData & 0x01) PORTB.5=1;
			else PORTB.5=0;
			UartswTxData>>=1;
		}
		else
		{                                     
		    PORTB.5=1;
		}
		UartswTxBitNum--;
		OCR2+=24;
	}
	else
	{                  
	    if (tx_counter1)
        {
            --tx_counter1;
            UartswTxData = tx_buffer1[tx_rd_index1];;
	        UartswTxBitNum = 9;	
            OCR2=TCNT2+24;
            TIFR |= (1<<7);
            TIMSK |= (1<<7);
            PORTB.5=0;
            if (++tx_rd_index1 == TX_BUFFER_SIZE1) tx_rd_index1=0;
        }
        else 
        {
            UartswTxBusy = 0;
            TIMSK &= ~(1<<7);            
        }    
	}
}

void putchar1(char c)
{
while (tx_counter1 == TX_BUFFER_SIZE1);
 #asm("cli")
if (tx_counter1 || (UartswTxBusy))
   {
   tx_buffer1[tx_wr_index1]=c;
   if (++tx_wr_index1 == TX_BUFFER_SIZE1) tx_wr_index1=0;
   ++tx_counter1;
   }
else
   {
    UartswTxBusy = 1;
	UartswTxData = c;
	UartswTxBitNum = 9;	
    TIFR |= (1<<7);
    TIMSK |= (1<<7);
    OCR2=TCNT2+24;   
    PORTB.5=0;
   }
#asm("sei")
}          


#define FIRST_ADC_INPUT 1
#define LAST_ADC_INPUT 3
unsigned char adc_data[LAST_ADC_INPUT-FIRST_ADC_INPUT+1];
#define ADC_VREF_TYPE 0x20

// ADC interrupt service routine
// with auto input scanning
interrupt [ADC_INT] void adc_isr(void)
{
register static unsigned char input_index=0;
// Read the 8 most significant bits
// of the AD conversion result
adc_data[input_index]=ADCH;
// Select next ADC input
if (++input_index > (LAST_ADC_INPUT-FIRST_ADC_INPUT))
   input_index=0;
ADMUX=(FIRST_ADC_INPUT | (ADC_VREF_TYPE & 0xff))+input_index;
switch (input_index) {
    case 0 : {               
                 if(((adc_data[input_index]>MIN_VOLTAGE)&&(adc_data[input_index]<MAX_VOLTAGE)&&(TermRes&(1<<0))||((adc_data[input_index] < MAX_VOLTERMRES)&&(!(TermRes&(1<<0))))))
                 {           
                    counter_adc1_l=0;            
                    if(!level_adc1_m)
                    {
                        if(counter_adc1_m>=DELAYMC240) 
                        {
                            level_adc1_m=1;
                            level_adc1_l=0;            
                            LED1_ON
                            ZonaS|=(1<<0);
                        }
                        else
                        {
                            counter_adc1_m++;    
                        }    
                    }
                 }
                 else
                 {            
                    counter_adc1_m=0;            
                    if(!level_adc1_l)
                    {
                        if(counter_adc1_l >= DELAYMC240) 
                        {
                            level_adc1_m=0;
                            level_adc1_l=1;
                            LED1_OFF
                            ZonaS&=~(1<<0);
                            if(sign&&(ZoneMask&(1<<0)))
                            {
                                Zona|=(1<<0);
                                ZonaE=Zona;
                            }
                        }
                        else
                        {
                            counter_adc1_l++;    
                        }
                    }                               
                 }            
              }
    break;
    case 1 : 
    {
                 if(((adc_data[input_index]>MIN_VOLTAGE)&&(adc_data[input_index]<MAX_VOLTAGE)&&(TermRes&(1<<1))||((adc_data[input_index] < MAX_VOLTERMRES)&&(!(TermRes&(1<<1))))))
                 {           
                    counter_adc2_l=0;            
                    if(!level_adc2_m)
                    {
                        if(counter_adc2_m>=DELAYMC240) 
                        {
                            level_adc2_m=1;
                            level_adc2_l=0;            
                            LED2_ON
                            ZonaS|=(1<<1);
                        }
                        else
                        {
                            counter_adc2_m++;    
                        }    
                    }
                 }
                 else
                 {            
                    counter_adc2_m=0;            
                    if(!level_adc2_l)
                    {
                        if(counter_adc2_l >= DELAYMC240) 
                        {
                            level_adc2_m=0;
                            level_adc2_l=1;
                            LED2_OFF
                            ZonaS&=~(1<<1);
                            if(sign&&(ZoneMask&(1<<1)))
                            {
                                Zona|=(1<<1);
                                ZonaE=Zona;
                            }
                        }
                        else
                        {
                            counter_adc2_l++;    
                        }
                    }                               
                 }            
    }
    break;         
    case 2 : 
    {
                 if(((adc_data[input_index]>MIN_VOLTAGE)&&(adc_data[input_index]<MAX_VOLTAGE)&&(TermRes&(1<<2))||((adc_data[input_index] < MAX_VOLTERMRES)&&(!(TermRes&(1<<2))))))
                 {           
                    counter_adc3_l=0;            
                    if(!level_adc3_m)
                    {
                        if(counter_adc3_m>=DELAYMC240) 
                        {
                            level_adc3_m=1;
                            level_adc3_l=0;            
                            LED3_ON
                            ZonaS|=(1<<2);
                        }
                        else
                        {
                            counter_adc3_m++;    
                        }    
                    }
                 }
                 else
                 {            
                    counter_adc3_m=0;            
                    if(!level_adc3_l)
                    {
                        if(counter_adc3_l >= DELAYMC240) 
                        {
                            level_adc3_m=0;
                            level_adc3_l=1;
                            LED3_OFF
                            ZonaS&=~(1<<2);
                            if(sign&&(ZoneMask&(1<<2)))
                            {
                                Zona|=(1<<2);
                                ZonaE=Zona;
                            }
                        }
                        else
                        {
                            counter_adc3_l++;    
                        }
                    }                               
                 }            
    }
    break;
    };
}

// Declare your global variables here

void main(void)
{
// Declare your local variables here

// Input/Output Ports initialization
// Port B initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=Out Func1=In Func0=In 
// State7=T State6=T State5=T State4=T State3=T State2=0 State1=P State0=T 
PORTB=0x32;
DDRB=0x34;

// Port C initialization
// Func6=In Func5=Out Func4=Out Func3=In Func2=In Func1=In Func0=In 
// State6=T State5=0 State4=0 State3=T State2=T State1=T State0=P 
PORTC=0x01;
DDRC=0x10;

// Port D initialization
// Func7=In Func6=In Func5=Out Func4=Out Func3=Out Func2=Out Func1=In Func0=In 
// State7=T State6=T State5=0 State4=0 State3=0 State2=0 State1=T State0=T 
PORTD=0x00;
DDRD=0x3C;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 7,200 kHz
TCCR0=0x05;
TCNT0=0xB8;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 7,200 kHz
// Mode: CTC top=OCR1A
// OC1A output: Discon.
// OC1B output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer 1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: On
// Compare B Match Interrupt: On
TCCR1A=0x00;
TCCR1B=0x0D;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x1C;
OCR1AL=0x20;
OCR1BH=0x00;
OCR1BL=0xFF;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: 230,400 kHz
// Mode: CTC top=OCR2
// OC2 output: Disconnected
ASSR=0x00;
TCCR2=0x03;
TCNT2=0x00;
OCR2=0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
MCUCR=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x19;

// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART Mode: Asynchronous
// USART Baud rate: 115200
UCSRA=0x00;
UCSRB=0x00;
UCSRC=0x86;
UBRRH=0x00;
UBRRL=0x03;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
SFIOR=0x00;

// ADC initialization
// ADC Clock frequency: 921,600 kHz
// ADC Voltage Reference: AREF pin
// Only the 8 most significant bits of
// the AD conversion result are used
ADMUX=FIRST_ADC_INPUT | (ADC_VREF_TYPE & 0xff);
ADCSRA=0xCB;

// Watchdog Timer initialization
// Watchdog Timer Prescaler: OSC/2048k
#pragma optsize-
WDTCR=0x1F;
WDTCR=0x0F;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

if(PIN_FACT_SET)
{
    SetFactorySetting();
    while(1)
    #asm("wdr");
}        
 eeprom2ram();

SimPowerOff();

if(sign_E)
{
    sign=sign_on=1;
    RPosib = 1;
    GETSR=1;
}
if(Zona)
{
    SInfo=1;
    StatusSMS=SMSE;
    StatusCall=CALE;
    AttemptCall=NUMCALL;
    AttemptSMS=AMOUNT_ATTEMPT_SMS;
}

// Global enable interrupts
#asm("sei")            

SendAnswer(&F_VERSIA[4]);

LED_ON 
        
if(VOL_PIN)level_vol_l = 1;
else level_vol_h = 1;

SimPowerOn();  

UCSRB=0xD8;

SendCommand("AT");        
delay_ms(1000);
SendCommand("AT");        

while (1)
      {
      // Place your code here
        #asm("wdr")
      // Check receive byte with GSM
      if(GSM_PACK)              HandlerEventGSM(); 
      else
      {
        if(rx_buffer_overflowGSM)   {gsm_rx_counter=0;rx_buffer_overflowGSM=0;}
        // Check receive byte with GSM
        if(rx_counterGSM)         Receive_gsm();
        // Check receive packet with GSM
        if(GSM_PACK) continue;
      }          
      if(rx_counterGSM) continue;
      if(status_sim&&(!TimeGuard)&&(!CounterGSM))
      {
        if(status_sim&S_RING)   if(F_Ring())status_sim&=~S_RING;
        if(status_sim&S_SMS)    if(F_SMS()) status_sim&=~S_SMS;
        if(status_sim&S_CRDY)   if(F_Crdy())status_sim&=~S_CRDY;
        //if(status_sim&S_ERROR)  {SimRst();RCallRDY=0;rdy_counter=30;CurrentNetwork=0;status_sim&=~S_ERROR;}
        if(status_sim&S_CREG)   {CurrentNetwork=1;status_sim&=~S_CREG;}        
        if(status_sim&S_CREGNOT){CurrentNetwork=0;status_sim&=~S_CREGNOT;}        
      }                
      if((!work_modem)&&(!TimeGuard)&&(!CounterGSM))
      {
        if(C_SendSimpleCommand(AT,100))
        {   
            SimRst();
            RCallRDY=0;
            rdy_counter=30;
            CurrentNetwork=0;
        }
        work_modem=120;
      }
      if((!ChangeRegNetwork)&&(!TimeGuard)&&(!CounterGSM))
      {
        switch (C_CREGG()) {
            case C_REG_NET : 
                            CurrentNetwork=1;
            break;
            case C_NOT_REG_NET :CurrentNetwork=0;
            break;       
        };               
        ChangeRegNetwork=240;
      }  
      if(!RCallRDY) 
        if(!rdy_counter) 
        {
            rdy_counter=30;
            SimRst();
        }                         
      if(SInfo&&(!TimeGuard)&&(!CounterGSM)) SendInfo();
      if(CurrentNetwork&&Power) SMSP();
      };
}
                
/*
 * Send answer with Flash memory string.  
 *
 * @param	*data	a pointer to the string command 
 */
void SendAnswer(flash char *data){
 unsigned char i=0;
  do {     
   #asm("wdr")
   putcharPC(data[i]);
  }while(data[++i]); 
}                               

/*
 * Load with EEPROM to RAM setting.  
 */             
void eeprom2ram(void)
{
 unsigned char i,j;
 #asm("wdr")         
 for(j=0;j!=NUMA_NUMBER;j++)
  for(i=0;i!=11;i++)           
   NUMAR[j][i]=NUMAE[j][i];
 ZoneMask=ZoneMaskE;
 Zona=ZonaE;
 TermRes = TermResE;              
 BuzOn = BuzOnE;
}             

void SetFactorySetting()
{
 unsigned char i,j;
 #asm("wdr")         
 for(j = 0;j != NUMA_NUMBER;j++)
 {
    for(i = 0;i != 10;i++)           
        NUMAE[j][i] = '0';
    NUMAE[j][10] = 0;
 }                  
 for(j = 0;j != 3; j++)
 {                                     
    str_zoneE[j][0] = 'Z';
    str_zoneE[j][1] = 'o';
    str_zoneE[j][2] = 'n';
    str_zoneE[j][3] = 'e';
    str_zoneE[j][4] = j + 0x31;
    str_zoneE[j][5] = 0;    
 }
 ZoneMaskE = 0x07;
 sign = 0;        
 SMSE = 0; 
 CALE = 0;
 for(i = 0;i != 6;i++)           
  PWDE[i] = '0';
 TermResE = 0x08;              
 BuzOnE = 0x0C;
 SETSE = 10;
 GETSE = 10;
 ZonaE = 0;
 sign_E = 0;
 DELAY_ZUME = NUM_SEC_OD_ON;
}

unsigned char SMSP(void)
{                                                                  
 unsigned char str1[40]; 
 #asm("wdr")                          
 str1[0]=0;
 if(Power == 1) strcatf(str1,"Appear 220v");
 else strcatf(str1,"Disappear 220v");
 switch (C_CREGG()) {
    case C_REG_NET :
        {                    
            CurrentNetwork=1;
            if(SendSMS(str1,&NUMAR[0][0]))
            {
                AttemptSMS--;
                if(!AttemptSMS) Power = 0;
                return 0;
            }
            else
            {        
                Power = 0;
                C_SendSimpleCommand(CMGDA,800);
                return 0;
            }
        }
        break;
        case C_NOT_REG_NET :
        {
            CurrentNetwork=0;
            return 0;   
        }
        break;
        default: return 0;       
 }; 
 return 1;
}         

/*
 * Check status signaliz. if active, then 3 tones sending, disactive 1 tone sending  
 */

unsigned char SMSF(void)
{                                                                  
 unsigned char i,a,j,b;
 unsigned char str1[70]; 
 str1[0]=0;
 strcatf(str1,"Alarm:");
 for(i=0;i!=NUMA_NUMBER;i++)
 {
  #asm("wdr")                          
    if((NUMAR[i][1]!=0x30)&&(StatusSMS&(1<<i)))
    {                          
        switch (C_CREGG()) {
            case C_REG_NET :
            {                    
                CurrentNetwork=1;
                str1[6]=0;
                for(a=0;a!=3;a++)
                    if((Zona&ZoneMask)&(1<<a))
                    {
                        b = 0;                        
                        j = 6;
                        str1[j++] = ' ';
                        do {     
                            #asm("wdr")          
                            str1[j++] = str_zoneE[a][b++];
                        }while(str_zoneE[a][b]);
                        str1[j] = 0;                        
                    }
                if(SendSMS(str1,&NUMAR[i][0])) {AttemptSMS--;return 0;}
                else StatusSMS&=~(1<<i);
                delay_ms(1000);   
            }
            break;
            case C_NOT_REG_NET :
            {
                CurrentNetwork=0;
                return 0;   
            }
            break;
            default: return 0;       
         }; 
    }    
 }
 if(SMSE) C_SendSimpleCommand(CMGDA,800);  
 return 1;
}         

/**
 * Send abonents information SMS and Call abonents.  
 **/                 
void SendInfo(void)
{
    if(CurrentNetwork)
    {
        if(StatusSMS&&AttemptSMS)
        {        
            if(SMSF()) StatusSMS=0; 
            else  return;
        }
        if(StatusCall&&AttemptCall)
        {
            SendCall();
        }
    }
    if((!(StatusCall&&AttemptCall))&&(!(StatusSMS&&AttemptSMS)))
    {             
        #asm("cli")
        Zona = 0;
        ZonaE = 0;   
        SInfo = 0;
        #asm("sei")
        sign = 0;
        SETSR = 0;
        sign_on = 1;
    }
}                    

/**
 * Send abonents information SMS and Call abonents.  
 **/             
void SendCall(void)
{             
 unsigned char i;           
 while(StatusCall&&AttemptCall)
 {
 #asm("wdr")
 for(i=0;i!=NUMA_NUMBER;i++)
 {
  if((NUMAR[i][1]!=0x30)&&(StatusCall&(1<<i)))
  {          
   switch (C_CREGG()) {
        case C_REG_NET :
        {                    
            CurrentNetwork=1;
            gsm_counter_AC=10;
            while(1)
            {               
                #asm("wdr")
                if(gsm_counter_AC)
                {
                    if(!C_CLCC())
                        if(!(S_CLCC.status))    
                           break;
                }
                else
                    { AttemptCall--; return ;}
                delay_ms(1000);         
            }
            if(!(C_ATDD(&NUMAR[i][0])))
            {
                gsm_counter_AC=DELAY_C;
                GSM_PACK=0;        
                while((!GSM_PACK)&&(gsm_counter_AC))
                {          
                    #asm("wdr") 
                    if(CounterGSM)
                        Receive_gsm();
                    else
                    {
                        if(C_CLCC())return;
                        if(S_CLCC.stat=='0')
                        {
                            StatusCall&=~(1<<i);
                            CounterGSM=0;
                            #asm("sei")
                            break;    
                        }
                        if(S_CLCC.stat=='4') return;
                        CounterGSM=200;
                    }
                }                             
                while((!GSM_PACK)&&(gsm_counter_AC))
                {          
                    #asm("wdr") 
                    Receive_gsm();
                }                                     
                if(GSM_PACK)   
                {
                    if ((strstrf(&gsm_rx_buffer[0],"BUSY")!=NULL))           // Busy  
                    { 
                        if(S_CLCC.stat=='3')
                            StatusCall&=~(1<<i);
                    }
                }
                else
                {  
                    if(C_ATH())return;            
                }
                GSM_PACK=0;
                delay_ms(1000);
            }
            else return;
        }
        break;
        case C_NOT_REG_NET :
        {
            CurrentNetwork=0;
            return ;   
        }
        break;
        default: return ;       
   }; 
  }
 }             
 AttemptCall--;
 }          
}
 
/*
 * Handler event as "RING".  
 *
 * @return	OK	if number register in base 
 */
unsigned char F_Ring(void)
{      
    unsigned char i;
    #asm("wdr") 
    if(C_CLCC()) return;
    for(i=0;i!=NUMA_NUMBER;i++)
    { 
        if(strstr(&S_CLCC.number[0],&NUMAR[i][0])!=NULL)
        {
            GSM_PACK=0;
            C_SendSimpleCommand(ATA,40);  
            #asm("wdr")    
            RPosib = 1;
            if(sign)C_SendSimpleCommand(ACT_,40);
            else C_SendSimpleCommand(DISACT_,40);    
            gsm_counter_AC=DELAY_CC;
            GSM_PACK=0;
            while(gsm_counter_AC)
            {
                while((!GSM_PACK)&&(gsm_counter_AC))
                {
                    Receive_gsm();
                    #asm("wdr");
                    if(Zona)gsm_counter_AC=0;
                }                             
                if(GSM_PACK)   
                {
                    if ((strstrf(&gsm_rx_buffer[0],"NO CARRIER")!=NULL))      // No Carrier  
                    { 
                        GSM_PACK=0; return;
                    }
                }
                else
                {  
                    if(C_ATH())return;            
                }
                GSM_PACK=0;
            } 
            gsm_counter_AC=0;
            return 1;                              
        }
    }
    C_ATH();
    return 1;                              
}

/**
 * Handler event as "Call ready".  
 *
 * @return	OK	if number register in base 
 */
unsigned char F_Crdy(void)
{  
    #asm("wdr") 
    C_SendSimpleCommand(ATE0,40);
    C_SendSimpleCommand(CNMI,40);
    delay_ms(1000);
    C_SendSimpleCommand(CFUN,40);
    C_SendSimpleCommand(CMGF,40);
    C_SendSimpleCommand(CSMP,40);
    C_SendSimpleCommand(CREG,40);
    C_SendSimpleCommand(CMIC,40);
    C_SendSimpleCommand(ATE0V1,40);
    C_SendSimpleCommand(CUSD,40);
    RCallRDY=1;
    return 1;                              
}
/**
  * Handler event as "Receive SMS".  
  *
  * @return	OK	if number register in base 
  **/ 
unsigned char F_SMS(void)
{                     
    unsigned char i,j = 0;           
    unsigned char mas[200];
    unsigned char str[]="NM1";
    unsigned char str1[]="NZ1";
    unsigned char *pointer;
    #asm("wdr") 
    C_CMGLU(&mas[0]);    
    
    pointer=strstrf(&mas[0],"PWD");
    if(pointer!=NULL)
    { 
     if(cmp_digit((pointer+3),6))
     {   
     for(i=0;i!=6;i++)
        if(PWDE[i]!=(*(pointer+3+i)))break;
     if(i==6)    
     {     
         for(i=0;i!=6;i++)
            if(PWDE[i]!='0') break;
         if(i != 6) j++;
         #asm("wdr")                    
         pointer=strstrf(mas,"USD");
         if(pointer!=NULL)
         {                                           
            pointer +=3;
            i = 0;
            do {     
            #asm("wdr")          
            i++;
            }while(((*(pointer+i)) != '@')&&(i != 40));
            *(pointer+i) = 0;
            delay_ms(3000);   
            if(!C_USSD(pointer))
            {
                delay_ms(2000);   
                SendSMS(pointer,&NUMAR[0][0]);
            }
            //else SendSMS("USSD ERROR",&NUMAR[0][0]);
            delay_ms(2000);   
            C_SendSimpleCommand(CMGDA,800);
            return 1;                              
         }        
         for(;j!=4;j++)
         {
            str[2]=j+0x31;
            pointer=strstr(mas,str);
            pointer += 3;
            if(pointer!=NULL)
            {
                if(cmp_digit(pointer,10))
                {
                    for(i=0;i!=10;i++)
                        NUMAR[j][i]=NUMAE[j][i]=*(pointer+i);   
                }
            }        
         }         
         pointer=strstrf(&mas[0],"NPW");
         if(pointer!=NULL)
         {                                               
            pointer += 3;
            if(cmp_digit(pointer,6))
            {
                for(i=0;i!=6;i++)
                 PWDE[i]=*(pointer+i);   
            }
         }
         pointer=strstrf(&mas[0],"PTR");      
         if(pointer!=NULL)
         {                        
            pointer += 3;
            if(cmp_digit(pointer,4))
            {
                TermRes=0x0;
                for(i=0;i!=4;i++)
                {                           
                    if(*(pointer+i)=='1') TermRes|=(1<<i);   
                }
                TermResE = TermRes;  
            }
         }                      
         pointer=strstrf(&mas[0],"BPO");      
         if(pointer!=NULL)
         {                      
            pointer += 3;
            if(cmp_digit(pointer,2))
            {
                BuzOn=0x0;
                if(*(pointer) == '1') BuzOn|=(1<<2);   
                if(*(pointer+1) == '1') BuzOn|=(1<<3);   
                BuzOnE = BuzOn;  
            }
         }                      
         for(j=0;j!=3;j++)        // Set name zone 1-3
         {
            str1[2]=0x31+j;
            pointer=strstr(mas,str1);
            if(pointer!=NULL)
            {                 
                pointer += 3;
                i = 0;
                do {     
                #asm("wdr")          
                str_zoneE[j][i] = *(pointer+i);
                i++;
                }while(((*(pointer+i)) != '*')&&(i != 10));
                str_zoneE[j][i] = 0;
            }        
         }         
         pointer=strstrf(&mas[0],"ACO"); // ?????
         if(pointer!=NULL)
         {
            SETSE=(unsigned char)atoi(pointer+3);
         }
         pointer=strstrf(&mas[0],"ACF"); // ?????
         if(pointer!=NULL)
         {
            GETSE=(unsigned char)atoi(pointer+3);
         }
         pointer=strstrf(&mas[0],"DNZ"); // ?????
         if(pointer!=NULL)
         {
            DELAY_ZUME=(unsigned char)atoi(pointer+3);
         }
         pointer=strstrf(&mas[0],"NPC");      
         if(pointer!=NULL)
         {
            pointer += 3;
            if(cmp_digit(pointer,4))
            {
                CALE=0x0;
                for(i=0;i!=4;i++)
                {                           
                    if(*(pointer+i)=='1') CALE|=(1<<i);   
                }
            }
         }                      
         pointer=strstrf(&mas[0],"NPS");
         if(pointer!=NULL)
         {                      
            pointer += 3;
            if(cmp_digit(pointer,4))
            {
                SMSE=0x0;
                for(i=0;i!=4;i++)
                {                           
                    if(*(pointer+i)=='1') SMSE|=(1<<i);   
                }
            }
         }
         pointer=strstrf(&mas[0],"ACZ");
         if(pointer!=NULL)
         {                      
            pointer += 3;
            if(cmp_digit(pointer,3))
            {
                ZoneMask=0x0;
                for(i=0;i!=3;i++)
                {                           
                    if(*(pointer+i)=='1') ZoneMask|=(1<<i);
                }
                ZoneMaskE=ZoneMask;
                sign=0;
                sign_E=0;
            }
         }
         mas[0]=mas[1]=0;
         strcatf(&mas[0]," NM2");
         strcat(&mas[0],&NUMAR[1][0]);
         strcatf(&mas[0]," NM3");
         strcat(&mas[0],&NUMAR[2][0]);
         strcatf(&mas[0]," NM4");
         strcat(&mas[0],&NUMAR[3][0]);
         strcatf(&mas[0]," NPC");
         for(i=0;i!=4;i++)
         {
            if(CALE&(1<<i))  strcatf(&mas[0],"1");
            else   strcatf(&mas[0],"0");
         }
         strcatf(&mas[0]," NPS");
         for(i=0;i!=4;i++)
         {
            if(SMSE&(1<<i))  strcatf(&mas[0],"1");
            else   strcatf(&mas[0],"0");
         }
         strcatf(&mas[0]," ACZ");
         for(i=0;i!=3;i++)
         {
            if(ZoneMask&(1<<i))  strcatf(&mas[0],"1");
            else   strcatf(&mas[0],"0");
         }     
         for(j=0;j!=3;j++)        // Set name zone 1-3
         {
            str1[2]=0x31+j;
            strcatf(mas," ");
            strcat(mas,str1);
            i = 0;
            do {     
                #asm("wdr")          
                mas[i+170] = str_zoneE[j][i];
                i++;
                }while(str_zoneE[j][i]);
            mas[i+170] = 0;        
            strcat(mas,&mas[170]);
         }
         strcatf(&mas[0]," ACO");
         i=SETSE;
         itoa(i,&mas[170]);
         strcat(&mas[0],&mas[170]);
         strcatf(&mas[0]," ACF");
         i=GETSE;
         itoa(i,&mas[170]);
         strcat(&mas[0],&mas[170]);
         strcatf(&mas[0]," DNZ");
         i=DELAY_ZUME;
         itoa(i,&mas[170]);
         strcat(&mas[0],&mas[170]);

         strcatf(&mas[0]," BPO");
         if(BuzOn&(1<<2))  strcatf(mas,"1");
         else   strcatf(mas,"0");
         if(BuzOn&(1<<3))  strcatf(mas,"1");
         else   strcatf(mas,"0");
         strcatf(&mas[0]," PTR");
         for(i=0;i!=4;i++)
         {
            if(TermRes&(1<<i))  strcatf(&mas[0],"1");
            else   strcatf(&mas[0],"0");
         }     
         strcatf(&mas[0]," PWD");
         for(i=0;i!=6;i++)
            mas[170+i]=PWDE[i];
         mas[176]=0;
         strcat(&mas[0],&mas[170]);  
         SendSMS(&mas[0],&NUMAR[0][0]);
         delay_ms(2000);   
     }
     }
   }       
    C_SendSimpleCommand(CMGDA,800);
    return 1;                              
}
unsigned char cmp_digit(unsigned char *mas,unsigned char number)
{
    unsigned char i;         
    for(i=0;i!=number;i++)
        if(!(isdigit(mas[i]))) return 0;
    return 1;    
}                                                                          