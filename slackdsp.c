#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "slackdsp.h"

//sine wavetable
const signed int sine[256] = {0, 804, 1607, 2410, 3211, 4011, 4808, 5602, 6392, 7179, 7961, 8739, 9512, 10278, 11039, 11793, 12539, 13278, 14010, 14732, 15446, 16151, 16846, 17530, 18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594, 23170, 23732, 24279, 24812, 25330, 25832, 26319, 26790, 27245, 27684, 28106, 28511, 28898, 29269, 29621, 29956, 30273, 30572, 30852, 31114, 31357, 31581, 31786, 31971, 32138, 32285, 32413, 32521, 32610, 32679, 32728, 32758, 32767, 32758, 32728, 32679, 32610, 32521, 32413, 32285, 32138, 31971, 31785, 31580, 31356, 31114, 30852, 30572, 30273, 29956, 29621, 29269, 28898, 28510, 28105, 27684, 27245, 26790, 26319, 25832, 25329, 24812, 24279, 23732, 23170, 22594, 22005, 21403, 20787, 20159, 19519, 18867, 18204, 17530, 16846, 16151, 15446, 14732, 14009, 13278, 12539, 11792, 11039, 10278, 9511, 8739, 7961, 7179, 6392, 5601, 4807, 4010, 3211, 2410, 1607, 804, 0, -804, -1608, -2410, -3211, -4011, -4808, -5602, -6392, -7179, -7962, -8739, -9512, -10278, -11039, -11793, -12539, -13279, -14010, -14733, -15446, -16151, -16846, -17530, -18205, -18868, -19520, -20160, -20787, -21403, -22005, -22595, -23170, -23732, -24279, -24812, -25330, -25832, -26319, -26790, -27245, -27684, -28106, -28511, -28898, -29269, -29622, -29956, -30273, -30572, -30852, -31114, -31357, -31581, -31786, -31971, -32138, -32285, -32413, -32521, -32610, -32679, -32728, -32758, -32767, -32758, -32728, -32679, -32610, -32521, -32413, -32285, -32138, -31971, -31785, -31580, -31356, -31114, -30852, -30572, -30273, -29956, -29621, -29269, -28898, -28510, -28105, -27684, -27245, -26790, -26319, -25832, -25329, -24812, -24279, -23731, -23170, -22594, -22005, -21402, -20787, -20159, -19519, -18867, -18204, -17530, -16845, -16151, -15446, -14732, -14009, -13278, -12539, -11792, -11038, -10278, -9511, -8739, -7961, -7179, -6392, -5601, -4807, -4010, -3211, -2410, -1607, -803};

//initialise SPI port

void spi_init(void) {
    SS = 1; //disable sram
    SPI1CON1bits.DISSCK = 0; //clock enabled
    SPI1CON1bits.DISSDO = 0; //SDO enabled
    SPI1CON1bits.MODE16 = 0; //8 bit mode
    SPI1CON1bits.SMP = 0; //data sampled in middle
    SPI1CON1bits.CKE = 0; //data changes on rising edge (idle to active)
    SPI1CON1bits.CKP = 0; //idle is low
    SPI1CON1bits.PPRE = 2; //4:1 primary prescaler
    SPI1CON1bits.SPRE = 7; //1:1 secondary prescaler (10Mhz SPI clock)
    SPI1CON1bits.MSTEN = 1; //master mode
    SPI1STATbits.SPIEN = 1; //enable SPI        
}

//initialise SRAM

void sram_init(unsigned char sram_mode) {

    unsigned char temp;

    SS = 0;
    SPI1BUF = wrsr;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SPI1BUF = sram_mode;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SS = 1;
}

//write a byte to SRAM

void sram_write(unsigned long sramaddress, unsigned char sramdata) {

    unsigned char temp;
    SS = 0;

    SPI1BUF = write;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

#ifdef LC1024

    SPI1BUF = sramaddress >> 16;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

#endif

    SPI1BUF = sramaddress >> 8;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SPI1BUF = sramaddress;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SPI1BUF = sramdata;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SS = 1;

}

//read a byte from SRAM

unsigned char sram_read(unsigned long sramaddress) {

    unsigned char temp;

    SS = 0;

    SPI1BUF = read;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

#ifdef LC1024

    SPI1BUF = sramaddress >> 16;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

#endif

    SPI1BUF = sramaddress >> 8;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SPI1BUF = sramaddress;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SPI1BUF = 255;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SS = 1;

    return (temp);

}

//write a 2 byte sequence to SRAM, must be in sequential mode

void sram_write2(unsigned long sramaddress, unsigned int sramdata) {

    unsigned char temp;
    SS = 0;

    SPI1BUF = write;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

#ifdef LC1024

    SPI1BUF = sramaddress >> 16;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

#endif

    SPI1BUF = sramaddress >> 8;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SPI1BUF = sramaddress;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SPI1BUF = sramdata >> 8;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SPI1BUF = sramdata;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SS = 1;

}

//read a 2 byte sequence from SRAM, must be in sequential mode

unsigned int sram_read2(unsigned long sramaddress) {

    unsigned int temp;

    SS = 0;

    SPI1BUF = read;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

#ifdef LC1024

    SPI1BUF = sramaddress >> 16;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

#endif

    SPI1BUF = sramaddress >> 8;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SPI1BUF = sramaddress;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF;

    SPI1BUF = 255;
    while (!SPI1STATbits.SPIRBF);
    temp = SPI1BUF << 8;

    SPI1BUF = 255;
    while (!SPI1STATbits.SPIRBF);
    temp = temp | SPI1BUF;

    SS = 1;

    return (temp);

}

//output

void output(int o_in) {

    unsigned int o_out;

    o_out = (o_in + 32768); //convert to unsigned
    OUTPUTH = o_out >> 8; //write upper 8 bits to Output High
    OUTPUTL = o_out & 255; //write lower 8 bits to Output low

}

//delayline

signed int delayline(unsigned long d_length, unsigned long d_time, signed int d_mod, signed int d_in) {

    static unsigned long writeaddress;
    static unsigned long readaddress;
    signed int d_out;

    writeaddress = (writeaddress + 2) & (d_length - 1);
    readaddress = (writeaddress + d_length - d_time + d_mod) & (d_length - 2);
    sram_write2(writeaddress, d_in);
    d_out = sram_read2(readaddress);
    return (d_out);
}

//mulx

signed int mulx(signed int m_in1, signed int m_in2) {

    signed long sample32;
    signed int m_out;

    sample32 = m_in1 * (long) m_in2;
    m_out = sample32 >> 16;

    return (m_out);

}

//add

signed int add(signed int a_in1, signed int a_in2) {

    signed int a_out;

    a_out = (a_in1 >> 1) + (a_in2 >> 1);

    return (a_out);

}

//clipper

signed int clip(signed int c_level, signed int c_in) {

    if (c_in < -c_level)
        c_in = -c_level;
    if (c_in > c_level)
        c_in = c_level;

    return (c_in);
}

//pitchshifter

signed int pitchshift(unsigned int s_shift, signed int s_in) {

#define slength 2048

    static signed int buffer[slength];
    static unsigned int writeaddress;
    static unsigned int readaddress;
    signed int sample1;
    signed int sample2;
    signed int env;
    static unsigned int counter;

    writeaddress = (writeaddress + 1) & (slength - 1);
    buffer[writeaddress] = s_in;

    counter = counter + s_shift;
    if (counter > 32767) {
        counter = counter & 32767;
        readaddress = (readaddress + 2) & (slength - 1);
    }

    env = (writeaddress - readaddress) << 5;
    if (env < 0)
        env = env * -1;
    if (env == -32768)
        env = 32767;

    sample1 = buffer[readaddress];
    sample2 = buffer[((readaddress + (slength / 2)) & (slength - 1))];
    sample1 = mulx(sample1, env);
    sample2 = mulx(sample2, 32767 - env);

    sample1 = (sample1 + sample2) * 2;

    return (sample1);
}

//lowpass filter

signed int lowpass(signed int lp_freq, signed int lp_in) {

    static signed long lp_out = 0;

    lp_out = ((((lp_in - lp_out) * lp_freq) >> 15) + lp_out);

    return (lp_out);
}

//highpass filter

signed int hipass(signed int hp_freq, signed int hp_in) {

    signed int hp_out;
    static signed long lp = 0;

    hp_out = ((((hp_in - lp) * hp_freq) >> 15) + lp) - hp_in;

    return (hp_out);

}
//VCO

signed int vco(unsigned int v_speed) {

    static unsigned long counter = 0;
    static unsigned char pos = 0;

    counter = counter + v_speed;
    if (counter > 32768) {
        counter = counter & 32767;
        pos = (pos + 8) & 255;

    }

    return (sine[pos]);
}

//random number generator

unsigned long random(void) {
    static unsigned long lfsr = 1234;
    unsigned char xorbit;
#define bit1 28
#define bit2 31
    xorbit = ((lfsr >> (bit1 - 1) ^ (lfsr >> (bit2 - 1)))) & 1;
    lfsr = (lfsr << 1 | xorbit);
    return (lfsr);
}