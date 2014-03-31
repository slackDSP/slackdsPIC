/* 
 * File:   pic33test_main.c
 * Author: ian
 *
 * Created on December 29, 2013, 3:20 PM
 */

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "slackdsp.h"   //include slackdsp library

// DSPIC33FJ64GP202 Configuration Bit Settings

// FBS
#pragma config BWRP = WRPROTECT_OFF     // Boot Segment Write Protect (Boot Segment may be written)
#pragma config BSS = NO_FLASH           // Boot Segment Program Flash Code Protection (No Boot program Flash segment)
#pragma config RBS = NO_RAM             // Boot Segment RAM Protection (No Boot RAM)

// FSS
#pragma config SWRP = WRPROTECT_OFF     // Secure Segment Program Write Protect (Secure segment may be written)
#pragma config SSS = NO_FLASH           // Secure Segment Program Flash Code Protection (No Secure Segment)
#pragma config RSS = NO_RAM             // Secure Segment Data RAM Protection (No Secure RAM)

// FGS
#pragma config GWRP = OFF               // General Code Segment Write Protect (User program memory is not write-protected)
#pragma config GSS = OFF                // General Segment Code Protection (User program memory is not code-protected)

// FOSCSEL
#pragma config FNOSC = FRCPLL           // Oscillator Mode (Internal Fast RC (FRC) w/ PLL)
#pragma config IESO = OFF               // Internal External Switch Over Mode (Start-up device with user-selected oscillator source)

// FOSC
#pragma config POSCMD = NONE            // Primary Oscillator Source (Primary Oscillator Disabled)
#pragma config OSCIOFNC = ON            // OSC2 Pin Function (OSC2 pin has digital I/0 function)
#pragma config IOL1WAY = OFF            // Peripheral Pin Select Configuration (Allow Multiple Re-configurations)
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Both Clock Switching and Fail-Safe Clock Monitor are disabled)

// FWDT
#pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler (1:32,768)
#pragma config WDTPRE = PR128           // WDT Prescaler (1:128)
#pragma config WINDIS = OFF             // Watchdog Timer Window (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog timer enabled/disabled by user software)

// FPOR
#pragma config FPWRT = PWR128           // POR Timer Value (128ms)
#pragma config ALTI2C = OFF             // Alternate I2C  pins (I2C mapped to SDA1/SCL1 pins)

// FICD
#pragma config ICS = PGD1               // Comm Channel Select (Communicate on PGC1/EMUC1 and PGD1/EMUD1)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG is Disabled)


//define global variables


volatile unsigned long POT[3];

int main(int argc, char** argv) {

    //setup internal clock for 80MHz/40MIPS using internal fast osc and PLL
    //7.37Mhz / 3 * 65 / 2 = 79.841666Mhz
    CLKDIVbits.PLLPRE = 1; //N1 = 3
    PLLFBD = 63; //M = 65
    CLKDIVbits.PLLPOST = 0; //N2 = 2
    while (!OSCCONbits.LOCK); //wait until PLL is locked

    //Setup IO pins
    AD1PCFGL = 0xFFFF; //set all pins digital
    AD1PCFGLbits.PCFG0 = 0; //Set AN0 (pin 2) analogue = POT1
    AD1PCFGLbits.PCFG1 = 0; //Set AN1 (pin 3) analogue = POT2
    AD1PCFGLbits.PCFG2 = 0; //Set AN2 (pin 4) analogue = POT3
    AD1PCFGLbits.PCFG5 = 0; //Set AN5 (pin 7) analogue = Audio in
    RPOR4bits.RP9R = 0b10010; //RP9 (pin 18) = PWM out
    RPOR4bits.RP8R = 0b10011; //RP8 (pin 17) = PWM out
    RPINR20bits.SDI1R = 5; //RP5 (pin 14) = SDI
    RPOR3bits.RP6R = 7; //RP6 (pin 15) = SDO
    RPOR3bits.RP7R = 8; //RP7 (pin 16) = SCK
    TRISAbits.TRISA4 = 0; //RA4 (Pin 12) = output = SS (SPI slave select)
    TRISBbits.TRISB1 = 0; //RB1 (pin 5) = output = LED1
    TRISBbits.TRISB2 = 0; //RB2 (pin 6) = output = LED2

    //Setup ADC module
    AD1CON2bits.VCFG = 0; //Ref = AVDD and AVSS
    AD1CON3bits.ADRC = 0; // Use system timer
    AD1CON3bits.ADCS = 2; //TAD = 3 * TCY
    AD1CON2bits.CHPS = 0; //Convert channel 0
    AD1CHS0bits.CH0SA = 5; //use AN5
    AD1CON1bits.ASAM = 1; //Auto sample
    AD1CON1bits.SSRC = 0; //manual convert
    AD1CON1bits.AD12B = 1; //12 bit sampling
    AD1CON1bits.FORM = 1; //signed integer format
    AD1CON1bits.ADON = 1; //ADC On

    //Initialise output compare module 1 for PWM output (OUTPUTH)
    OC1CONbits.OCM = 0; //disble module
    OC1R = 127; //default duty cycle
    OC1CONbits.OCTSEL = 0; //Select timer 2 as time base
    OC1CONbits.OCM = 0b110; //Select output mode PWM

    //Initialise output compare module 2 for PWM output (OUTPUTL)
    OC2CONbits.OCM = 0; //disble module
    OC2R = 255; //default duty cycle
    OC2CONbits.OCTSEL = 0; //Select timer 2 as time base
    OC2CONbits.OCM = 0b110; //Select output mode PWM

    //Initialise timer 1
    T1CONbits.TON = 0; //Disable timer
    T1CONbits.TCS = 0; //Select internal instruction cycle clock
    T1CONbits.TGATE = 0; //Disable gated mode
    T1CONbits.TCKPS = 0; //1:1 Prescaler
    TMR1 = 0; //Clear timer register
    PR1 = 1218; //Load the period value sample rate = 32768Hz
    IPC0bits.T1IP = 1; //Set timer 1 priority level
    IFS0bits.T1IF = 0; //Clear timer 1 interrupt flag
    IEC0bits.T1IE = 1; //Enable timer 1 interrupt
    T1CONbits.TON = 1; //Start timer 1

    //Initialise timer 2
    T2CONbits.TON = 0; //Disable timer
    T2CONbits.TCS = 0; //Select internal instruction cycle clock
    T2CONbits.TGATE = 0; //Disable gated mode
    T2CONbits.TCKPS = 0; //1:1 Prescaler
    TMR2 = 0; //Clear timer register
    PR2 = 257; //Load the period value PWM freq = 155KHz
    IPC1bits.T2IP = 1; //Set timer 2 priority level
    IFS0bits.T2IF = 0; //Clear timer 2 interrupt flag
    T2CONbits.TON = 1; //Start timer 2

    //Initialise timer 3
    T3CONbits.TON = 0; //Disable timer
    T3CONbits.TCS = 0; //Select internal instruction cycle clock
    T3CONbits.TGATE = 0; //Disable gated mode
    T3CONbits.TCKPS = 3; //1:256 Prescaler
    TMR3 = 0; //Clear timer register
    PR3 = 16384; //Load the period value freq = 10Hz
    IPC2bits.T3IP = 1; //Set timer 3 priority level
    IFS0bits.T3IF = 0; //Clear timer 3 interrupt flag
    IEC0bits.T3IE = 1; //Enable timer 3 interrupt
    T3CONbits.TON = 1; //Start timer 3

    spi_init(); //initialise SPI port
    sram_init(seq); //initialise SRAM sequential mode


    for (;;) {

        //Nothing to see here

    }

    return (EXIT_SUCCESS);
}

//Timer 1 ISR - Runs every ~30us 32768Hz

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void) {

    static int sample;
    static unsigned char divider = 0;
    static int sampleout;
    unsigned int shift;
    unsigned char oversamp;
    static unsigned long LFO = 0;
   
    IFS0bits.T1IF = 0; //Clear timer 1 interrupt flag

    divider++; //sample rate divider
    if (divider == 1) { //1 = 32768Hz, 2 = 16384Hz, 4 = 8192Hz etc
        divider = 0;


        AD1CHS0bits.CH0SA = 5; //use AN5
        sample = 0;
        for (oversamp = 0; oversamp < 4; oversamp++) {
            AD1CON1bits.SAMP = 0;
            while (!AD1CON1bits.DONE);
            AD1CON1bits.DONE = 0;
            sample = sample + ADC;
        }

      sample = hipass(256, (sample << 2)); //convert to 16 bit, high pass at 40Hz

        sampleout = delayline(32768, POT1 >> 1, 0, sample);

        if (!SWITCH1) {


            LFO = LFO + 2;
            if (LFO > 65534) {
                LED1 = ~LED1;
                LFO = 0;
            }

            if (LFO < 65535)
                shift = 32768;
            if (LFO < 49151)
                shift = 24548;
            if (LFO < 32768)
                shift = 20642;
            if (LFO < 16384)
                shift = 12274;
            if (LFO < 8192)
                shift = 8192;

            sampleout = pitchshift(shift, sampleout);
            
        }
        LED2 = ~SWITCH1;

        output(sampleout); //write sampleout to output

    }
}

//Timer 3 ISR - Runs every ~100ms 10Hz, samples pots

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void) {

    static unsigned int temp_pot;
    static unsigned char potselect = 0;

    IFS0bits.T3IF = 0; //Clear timer 3 interrupt flag
    potselect = potselect + 1;
    if (potselect == 3)
        potselect = 0;
    AD1CHS0bits.CH0SA = potselect; //cycle through AN0, AN1 and AN2
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    AD1CON1bits.DONE = 0;
    temp_pot = ((ADC & 4032) << 4) + 32768; //convert to 6 bits scaled  ot 16 bit unsigned (64 values)
    POT[potselect] = temp_pot;
    AD1CHS0bits.CH0SA = 5;

}