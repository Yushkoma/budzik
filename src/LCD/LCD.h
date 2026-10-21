#pragma once
#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "../macros.h"

class LiquidCrystalDisplay {
private:
    volatile uint8_t *port;
    volatile uint8_t *ddr;
    uint8_t rs_pin;
    uint8_t e_pin;
    uint8_t data_pins_offset;

    void send_nibble(const uint8_t data);
    void send_byte(uint8_t data, bool cmd = 0);
    void cmd(const uint8_t data);
    
public:
    LiquidCrystalDisplay(volatile uint8_t *port, volatile uint8_t *ddr, uint8_t rs_pin, uint8_t e_pin, uint8_t data_pins_offset);

    void init();

    void send_char(const uint8_t data);
    void send_text(const char *text);
    
    void LCD_line(bool line);
    void clear();

    void LCD_move_text(bool right);
};
