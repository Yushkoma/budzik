#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "../I2C/I2C.h"

#define RTC_ADDR 0x68

#include "../macros.h"

// Zamiana BCD na dec
uint8_t bcdToDec(uint8_t val) {
  return ((val / 16 * 10) + (val % 16));
}

// Zamiana dec na BCD
uint8_t decToBcd(uint8_t val) {
  return ((val / 10 * 16) + (val % 10));
}

void rtc_set_time(uint8_t godzina, uint8_t minuta, uint8_t sekunda) {
    I2C_start();
    
    I2C_send((RTC_ADDR << 1) | 0);
    I2C_send(0x00);

    I2C_send(decToBcd(sekunda)); 
    I2C_send(decToBcd(minuta));
    I2C_send(decToBcd(godzina));
    
    I2C_stop();
}

void rtc_read_time(uint8_t *godzina, uint8_t *minuta, uint8_t *sekunda) {
    I2C_start();
    I2C_send((RTC_ADDR << 1) | 0);
    I2C_send(0x00);
    
    I2C_start();
    I2C_send((RTC_ADDR << 1) | 1);

    *sekunda = bcdToDec(I2C_read(1)); 
    *minuta = bcdToDec(I2C_read(1));
    *godzina = bcdToDec(I2C_read(0));
    
    I2C_stop();
}

void rtc_set_date(uint8_t day, uint8_t month, uint8_t year) {
    I2C_start();
    
    I2C_send((RTC_ADDR << 1) | 0);
    I2C_send(0x04);

    I2C_send(decToBcd(day)); 
    I2C_send(decToBcd(month));
    I2C_send(decToBcd(year));
    
    I2C_stop();
}

void rtc_read_date(uint8_t *day, uint8_t *month, uint8_t *year) {
    I2C_start();
    I2C_send((RTC_ADDR << 1) | 0);
    I2C_send(0x04);
    
    I2C_start();
    I2C_send((RTC_ADDR << 1) | 1);

    *day = bcdToDec(I2C_read(1)); 
    *month = bcdToDec(I2C_read(1));
    *year = bcdToDec(I2C_read(0));
    
    I2C_stop();
}