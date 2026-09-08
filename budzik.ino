#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "LCD_lib.h"
#include "RTC_lib.h"

#ifndef _BV
#define _BV(bit)        (1<<(bit))
#endif
#ifndef sbi
#define sbi(reg,bit)    reg |= (_BV(bit))
#endif
 
#ifndef cbi
#define cbi(reg,bit)    reg &= ~(_BV(bit))
#endif

#ifndef tbi
#define tbi(reg,bit)    reg ^= (_BV(bit))
#endif

// PD2 (A2) RS_PIN
// PD3 (A3) E_PIN
//


volatile uint8_t time_sec = 0;
volatile uint8_t time_min = 0;
volatile uint8_t time_hour = 0;

uint8_t rtc_s = 10;
uint8_t rtc_m = 48;
uint8_t rtc_h = 2;

char text[40] = "                     ";

void init(){
  DDRD |= 0b11111100; //PD2-PD7 jako wyjścia
  PORTD &= 0b00000000; //wyzerowanie wyprowadzeń
  LCD_init();
  I2C_init();
  //timer1 conf 
  /*
  TCCR1A |= 0;
  TCCR1B |= _BV(WGM12) + _BV(CS12);
  OCR1A = 62499;
  TIMSK1 |= _BV(OCIE1A);
*/
  sei();
}

int main(){
  init();
  rtc_set_time(rtc_h, rtc_m, rtc_s); // Ustawiamy czas na 00:00:00
  while (1){
    _delay_ms(900);
    LCD_clear();
    LCD_line(0);
    LCD_text("Counting:");
    LCD_line(1);
    rtc_read_time(&rtc_h, &rtc_m, &rtc_s);
    sprintf(text, "%02u:%02u:%02u", rtc_h, rtc_m, rtc_s);
    LCD_text(text);
  }
}
/*
ISR(TIMER1_COMPA_vect) {
  time_sec++;
  if (time_sec >= 60){
    time_sec = 0;
    time_min++;
    if (time_min >= 60){
      time_min = 0;
      time_hour++;
      if (time_hour >= 24 )
        time_hour = 0;
    }
  }
}*/
