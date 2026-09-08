#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#ifndef BM
#define BM(bit)        (1<<(bit))
#endif
#ifndef sbi
#define sbi(reg,bit)    reg |= (BM(bit))
#endif
#ifndef cbi
#define cbi(reg,bit)    reg &= ~(BM(bit))
#endif
#ifndef tbi
#define tbi(reg,bit)    reg ^= (BM(bit))
#endif

#define RS_PIN      PD2 //A2
#define E_PIN       PD3 //A3



void LCD_send_nibble(const uint8_t data){
    PORTD &= 0b00001111; //zerowanie PD4-PD7
    PORTD |= (0b11110000 & data); //górne 4 bity danych do przesyłu
    sbi(PORTD, E_PIN);
    _delay_us(2);
    cbi(PORTD, E_PIN);
    _delay_us(1);
}
void LCD_send_byte(uint8_t data, bool ascii){
    if (ascii){
        sbi(PORTD, RS_PIN);
    } else {
        cbi(PORTD, RS_PIN);
    }
    LCD_send_nibble(data);
    data = data << 4;
    LCD_send_nibble(data);
}
void LCD_cmd(const uint8_t data){
    LCD_send_byte(data, 0);
    if (data==0x01 || data==0x02){
        _delay_ms(2);
    } else {
        _delay_us(50);
    }
}
void LCD_clear(){
    LCD_cmd(0x01);
    _delay_ms(2);
}
void LCD_char(const uint8_t data){
    LCD_send_byte(data, 1);
    _delay_us(50);
}
void LCD_text(const char* str){
    while (*str){
        LCD_char(*str++);
    }
}
void LCD_line(bool line){
    if (line == 0){
        LCD_cmd(0x80);
    } else {
        LCD_cmd(0x80+0x40);
    }
}
void LCD_move_text(bool right){
    LCD_cmd(0x18 + (right ? 0x04 : 0x00));
}

void LCD_init(){
    _delay_ms(20);
    
    cbi(PORTD, RS_PIN); //przesyłanie komend
    LCD_send_nibble(0b00110000); //ustawienie trybu 8 bit (trzykrotne dla bezpieczeństwa)
    _delay_ms(5);
    LCD_send_nibble(0b00110000);
    _delay_us(400);
    LCD_send_nibble(0b00110000); 
    _delay_us(400);

    LCD_send_nibble(0b00100000); //ustawienie trybu 4 bit
    _delay_ms(2);

    LCD_cmd(0x28); //2 lines of text display
    LCD_cmd(0x08); //display off 
    LCD_cmd(0x01); //clear display
    LCD_cmd(0x06); //kursor idzie w prawo po każdym znaku
    LCD_cmd(0x0C); //display ON
}






