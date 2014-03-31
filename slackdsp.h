/* 
 * File:   slackdsp.h
 * Author: ian
 *
 * Created on January 19, 2014, 11:54 AM
 */

#ifndef SLACKDSP_H
#define	SLACKDSP_H

//Define nice pin names

#define LED1 LATBbits.LATB1 //LED1 output
#define LED2 LATBbits.LATB2 //LED2 output
#define SWITCH1 PORTAbits.RA3
#define SWITCH2 PORTBbits.RB4
#define SS LATAbits.LATA4 //SS SPI slave select


//define other nice names

#define ADC ADC1BUF0 //ADC input
#define OUTPUTH OC1RS //PWM output MSB
#define OUTPUTL OC2RS //PWM output LSB
#define POT1 POT[0]
#define POT2 POT[1]
#define POT3 POT[2]

//define values for SRAM functions

//#define LC1024 //uncomment to use 23LC1024
#define write 2
#define read 3
#define rdsr 5
#define wrsr 1

#ifdef LC1024
#define byte 0
#define seq 64
#else
#define byte 1
#define seq 65
#endif


//declare external variables
extern const signed int sine[256];

//function prototypes

void spi_init(void);
void sram_init(unsigned char sram_mode);
void sram_write(unsigned long sramaddress, unsigned char sramdata);
unsigned char sram_read(unsigned long sramaddress);
void sram_write2(unsigned long sramaddress, unsigned int sramdata);
unsigned int sram_read2(unsigned long sramaddress);
void output(int o_in);
signed int delayline(unsigned long d_length, unsigned long d_time, signed int d_mod, signed int d_in);
signed int mulx(signed int m_in1, signed int m_in2);
signed int add(signed int a_in1, signed int a_in2);
signed int clip(signed int c_level, signed int c_in);
signed int pitchshift(unsigned int shift, signed int s_in);
signed int lowpass(signed int lp_freq, signed int lp_in);
signed int hipass(signed int hp_freq, signed int hp_in);
signed int vco(unsigned int v_speed);
unsigned long random(void);
#endif	/* SLACKDSP_H */