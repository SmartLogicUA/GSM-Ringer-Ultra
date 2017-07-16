/*
  CodeVisionAVR C Compiler
  (C) 2008 Taras Drozdovsky, My.
*/

#ifndef _BOARD_INCLUDED_
#define _BOARD_INCLUDED_

#include <mega8.h>
                
#define CRCEH       511
#define CRCEL       510
#define HND         496

#define LED_ON      PORTD.5=1;
#define LED_OFF     PORTD.5=0;
#define LED_TOD     PORTD.5=~PORTD.5;

#define LED1_ON     PORTD.4=1;
#define LED1_OFF    PORTD.4=0;
#define LED1_TOD    PORTD.4=~PORTD.4;

#define LED2_ON     PORTD.3=1;
#define LED2_OFF    PORTD.3=0;
#define LED2_TOD    PORTD.3=~PORTD.3;

#define LED3_ON     PORTD.2=1;
#define LED3_OFF    PORTD.2=0;
#define LED3_TOD    PORTD.2=~PORTD.2;

// Declare PIN POWER SIM
#define SIM_ON    PORTB.2
#define VDD_EXT   PINB.1
      
#define BUT       PINC.0
#define VOL_PIN   PINC.5

#define PIN_FACT_SET    PINB.3

#define OPENDRAIN1_OFF  PORTC.4=0;
#define OPENDRAIN1_ON   PORTC.4=1;
#define OPENDRAIN1_TOG  PORTC.4=~PORTC.4;

#define NUMA_NUMBER 4

#define  rx_buffer_overflowGSM rx_buffer_overflow
#define rx_counterGSM  rx_counter
#define tx_counterGSM  tx_counter
#define putcharGSM     putchar
#define getcharGSM     getchar

#define rx_buffer_overflowPC rx_buffer_overflow1
#define rx_counterPC  rx_counter1
#define tx_counterPC  tx_counter1
#define putcharPC     putchar1
#define getcharPC     getchar1

// Possible respons after executant command
#define C_OK              0             
#define C_ERROR           2
#define C_REG_NET         8
#define C_NOT_REG_NET     9

#define S_RING      (1<<0)
#define S_SMS       (1<<1)
#define S_CRDY      (1<<2)
#define S_POWERDOWN (1<<3)
#define S_ERROR     (1<<4) 
#define S_CREG      (1<<5)
#define S_CREGNOT   (1<<6)
    
#endif         