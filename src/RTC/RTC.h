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
  return ((val >> 4) * 10) + (val & 0x0F);
}

// Zamiana dec na BCD
uint8_t decToBcd(uint8_t val) {
  return ((val / 10) << 4) | (val % 10);
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

    // Odczyt z maskowaniem flag sprzętowych RTC (np. tryb 12/24h)
    *sekunda = bcdToDec(I2C_read(1) & 0x7F); 
    *minuta = bcdToDec(I2C_read(1) & 0x7F);
    *godzina = bcdToDec(I2C_read(0) & 0x3F);
    
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

    // Odczyt z maskowaniem flag sprzętowych RTC (np. Century Bit)
    *day = bcdToDec(I2C_read(1) & 0x3F); 
    *month = bcdToDec(I2C_read(1) & 0x1F);
    *year = bcdToDec(I2C_read(0)); // Rok wykorzystuje całe 8 bitów
    
    I2C_stop();
}


void get_date_compilation(uint8_t *day, uint8_t *month, uint8_t *year) {
  *year = (__DATE__[9] - '0') * 10 + (__DATE__[10] - '0');

  // 2. Parsowanie dnia ("dd")
  // Cyfry dnia znajdują się pod indeksami 4 i 5.
  char d1 = __DATE__[4];
  if (d1 == ' ') {
      d1 = '0'; // Jeśli pierwszy znak dnia to spacja (np. "  9"), zamień go ręcznie na znak '0'
  }
  *day = (d1 - '0') * 10 + (__DATE__[5] - '0');

  switch(__DATE__[0]) {
      case 'J': // Jan (styczeń), Jun (czerwiec), Jul (lipiec)
          if (__DATE__[1] == 'a') *month = 1;
          else if (__DATE__[2] == 'n') *month = 6;
          else *month = 7;
          break;
      case 'F': *month = 2; break; // Feb (luty)
      case 'M': // Mar (marzec), May (maj)
          if (__DATE__[2] == 'r') *month = 3;
          else *month = 5;
          break;
      case 'A': // Apr (kwiecień), Aug (sierpień)
          if (__DATE__[1] == 'p') *month = 4;
          else *month = 8;
          break;
      case 'S': *month = 9; break;  // Sep (wrzesień)
      case 'O': *month = 10; break; // Oct (październik)
      case 'N': *month = 11; break; // Nov (listopad)
      case 'D': *month = 12; break; // Dec (grudzień)
  }
}

void rtc_set_compilation_data() {
  uint8_t hour, min, sec;
  uint8_t day, month, year;
  hour = (__TIME__[0] - '0') * 10 + (__TIME__[1] - '0');
  min = (__TIME__[3] - '0') * 10 + (__TIME__[4] - '0');
  sec = (__TIME__[6] - '0') * 10 + (__TIME__[7] - '0');
  rtc_set_time(hour, min, sec);
  get_date_compilation(&day, &month, &year);
  rtc_set_date(day, month, year);
}

void rtc_set_week_day(uint8_t day) {
    // Zabezpieczenie przed wpisaniem wartości spoza zakresu (0-6)
    if (day < 0) day = 0;
    if (day > 6) day = 6;
    day++; // RTC używa zakresu 1-7 dla dni tygodnia, więc dodajemy 1
    I2C_start();
    
    I2C_send((RTC_ADDR << 1) | 0); // Tryb zapisu
    I2C_send(0x03);                // Wskaźnik na rejestr 0x03 (Day)

    I2C_send(decToBcd(day));       // Wysłanie dnia tygodnia
    
    I2C_stop();
}

uint8_t rtc_get_week_day(void) {
    uint8_t day;
    
    I2C_start();
    I2C_send((RTC_ADDR << 1) | 0); // Tryb zapisu (aby ustawić wskaźnik)
    I2C_send(0x03);                // Wskaźnik na rejestr 0x03 (Day)
    
    I2C_start();                   // Repeated start
    I2C_send((RTC_ADDR << 1) | 1); // Tryb odczytu

    // Odczytujemy jeden bajt. Używamy 0 (NACK), bo to jedyny/ostatni odczytywany bajt.
    // Maskujemy wartości do 3 najmłodszych bitów (0x07), by odfiltrować ewentualne śmieci.
    day = bcdToDec(I2C_read(0) & 0x07); 
    
    I2C_stop();
    
    return day-1;
}