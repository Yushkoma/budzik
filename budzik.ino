#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "src/LCD/LCD.h"
#include "src/RTC/RTC.h"
#include "src/I2C/I2C.h"
#include "src/macros.h"

// PD2 (A2) RS_PIN
// PD3 (A3) E_PIN
// PD4-PD7 (A4-A7) are used for data transmission to the display

#define LCD_RS_PIN      PD2 //A2
#define LCD_E_PIN       PD3 //A3
#define LCD_DATA_PINS_OFFSET 4

void get_date_compilation(void);

uint8_t rtc_sec = 10;
uint8_t rtc_min = 32;
uint8_t rtc_hour = 0;
uint8_t rtc_day = 0;
uint8_t rtc_month = 0;
uint8_t rtc_year = 0;

char text[40] = "                     ";

void init(){
  I2C_init();

  sei();
}



int main(){
  init();

  rtc_hour = (__TIME__[0] - '0') * 10 + (__TIME__[1] - '0');
  rtc_min = (__TIME__[3] - '0') * 10 + (__TIME__[4] - '0');
  rtc_sec = (__TIME__[6] - '0') * 10 + (__TIME__[7] - '0');
  rtc_set_time(rtc_hour, rtc_min, rtc_sec);
  get_date_compilation();
  rtc_set_date(rtc_day, rtc_month, rtc_year);

  LiquidCrystalDisplay lcd(&PORTD, &DDRD, LCD_RS_PIN, LCD_E_PIN, LCD_DATA_PINS_OFFSET);
  lcd.init();
  lcd.clear();
  while (1){
    _delay_ms(50);
    uint8_t second_last = rtc_sec;
    rtc_read_time(&rtc_hour, &rtc_min, &rtc_sec);
    if (rtc_sec != second_last){
      lcd.LCD_line(0);
      sprintf(text, "%02u:%02u:%02u   ", rtc_hour, rtc_min, rtc_sec);
      lcd.send_text(text);

      lcd.LCD_line(1);
      rtc_read_date(&rtc_day, &rtc_month, &rtc_year);
      sprintf(text, "%02u.%02u.20%02u   ", rtc_day, rtc_month, rtc_year);
      lcd.send_text(text); 
    }
  }
}

void get_date_compilation(){
  rtc_year = (__DATE__[9] - '0') * 10 + (__DATE__[10] - '0');

  // 2. Parsowanie dnia ("dd")
  // Cyfry dnia znajdują się pod indeksami 4 i 5.
  char d1 = __DATE__[4];
  if (d1 == ' ') {
      d1 = '0'; // Jeśli pierwszy znak dnia to spacja (np. "  9"), zamień go ręcznie na znak '0'
  }
  rtc_day = (d1 - '0') * 10 + (__DATE__[5] - '0');

  switch(__DATE__[0]) {
      case 'J': // Jan (styczeń), Jun (czerwiec), Jul (lipiec)
          if (__DATE__[1] == 'a') rtc_month = 1;
          else if (__DATE__[2] == 'n') rtc_month = 6;
          else rtc_month = 7;
          break;
      case 'F': rtc_month = 2; break; // Feb (luty)
      case 'M': // Mar (marzec), May (maj)
          if (__DATE__[2] == 'r') rtc_month = 3;
          else rtc_month = 5;
          break;
      case 'A': // Apr (kwiecień), Aug (sierpień)
          if (__DATE__[1] == 'p') rtc_month = 4;
          else rtc_month = 8;
          break;
      case 'S': rtc_month = 9; break;  // Sep (wrzesień)
      case 'O': rtc_month = 10; break; // Oct (październik)
      case 'N': rtc_month = 11; break; // Nov (listopad)
      case 'D': rtc_month = 12; break; // Dec (grudzień)
  }
}

