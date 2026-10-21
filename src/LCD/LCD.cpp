#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "LCD.h"
#include "../macros.h"


void LiquidCrystalDisplay::send_nibble(const uint8_t data){
    *port &= ~(0b00001111 << data_pins_offset); //zerowanie pinów danych
    *port |= (0b00001111 & data) << data_pins_offset; //górne 4 bity danych do przesyłu
    sbi(*port, e_pin);
    _delay_us(2);
    cbi(*port, e_pin);
    _delay_us(40);
}
void LiquidCrystalDisplay::send_byte(uint8_t data, bool cmd){
    if (cmd){
        cbi(*port, rs_pin);
    } else {
        sbi(*port, rs_pin);
    }
    send_nibble(data >> 4);
    send_nibble(data);
}
void LiquidCrystalDisplay::cmd(const uint8_t data){
    send_byte(data, 1);
    if (data==0x01 || data==0x02){
        _delay_ms(2);
    } else {
        _delay_us(50);
    }
}


LiquidCrystalDisplay::LiquidCrystalDisplay(volatile uint8_t *port, volatile uint8_t *ddr, uint8_t rs_pin, uint8_t e_pin, uint8_t data_pins_offset):
    port(port), ddr(ddr), rs_pin(rs_pin), e_pin(e_pin), data_pins_offset(data_pins_offset) {}

void LiquidCrystalDisplay::init(){
    *ddr |= (1 << rs_pin) | (1 << e_pin); //ustawienie pinów jako wyjścia
    *port &= ~((1 << rs_pin) | (1 << e_pin)); //wyzerowanie pinów RS i E
    *ddr |= (0b00001111 << data_pins_offset); //ustawienie pinów danych jako wyjścia

    _delay_ms(20);
    
    cbi(*port, rs_pin); //przesyłanie komend
    send_nibble(0b00000011); //ustawienie trybu 8 bit (trzykrotne dla bezpieczeństwa)
    _delay_ms(5);
    send_nibble(0b00000011);
    _delay_us(400);
    send_nibble(0b00000011); 
    _delay_us(400);

    send_nibble(0b00000010); //ustawienie trybu 4 bit
    _delay_ms(2);

    cmd(0x28); //2 lines of text display
    cmd(0x08); //display off 
    cmd(0x01); //clear display
    cmd(0x06); //kursor idzie w prawo po każdym znaku
    cmd(0x0C); //display ON
}

void LiquidCrystalDisplay::send_char(const uint8_t data){
    send_byte(data, 0);
    _delay_us(50);
}
void LiquidCrystalDisplay::send_text(const char *text){
    while (*text){
        send_char(*text++);
    }
}

void LiquidCrystalDisplay::LCD_line(bool line){
    if (line == 0){
        cmd(0x80);
    } else {
        cmd(0x80+0x40);
    }
}

void LiquidCrystalDisplay::clear(){
    cmd(0x01);
    _delay_ms(2);
}

void LiquidCrystalDisplay::LCD_move_text(bool right){
    cmd(0x18 + (right ? 0x04 : 0x00));
}