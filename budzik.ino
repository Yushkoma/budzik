#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "src/LCD/LCD.h"
#include "src/RTC/RTC.h"
#include "src/I2C/I2C.h"
#include "src/macros.h"
#include "src/speaker.h"
// PD2 (A2) RS_PIN
// PD3 (A3) E_PIN
// PD4-PD7 (A4-A7) are used for data transmission to the display
#define LCD_RS_PIN      PD2 //A2
#define LCD_E_PIN       PD3 //A3
#define LCD_DATA_PINS_OFFSET 4


struct alarm_time {
    uint8_t hour;
    uint8_t min;
};

struct alarm_time alarm[7] = {{8, 25}, {8, 25}, {10, 25}, {8, 30}, {8, 30}, {10, 00}, {21, 14}}; // Alarm times for each day of the week (0=Monday, 6=Sunday)
uint8_t current_day_of_week = 0; // Current day of the week (0=Monday, 6=Sunday)
uint8_t next_alarm_day = 0; // Last day of the week, initialized to an invalid value to ensure the first update occurs


uint8_t sec = 0;
uint8_t min = 0;
uint8_t hour = 0;
uint8_t day = 0;
uint8_t month = 0;
uint8_t year = 0;
char text[40] = "                     ";
volatile unsigned long timer_millis = 0;
bool speaker_active = false;
bool button_pressed = false;

bool second_changed_check();
void lcd_refresh(LiquidCrystalDisplay &lcd);
bool check_button();
void speaker_check();
bool button_60_sec_pressed_check();
ISR(TIMER0_COMPA_vect);
void timer0_init();
unsigned long millis();



void init(){
  I2C_init();
  timer0_init();
  speaker_init();
  cbi(DDRB, 2); // Set PB2 (pin 10) as input for the button
  sbi(PORTB, 2); // Enable pull-up resistor on PB2 (pin 10)

  sbi(DDRB, 3); // Set PB3 (pin 11) as output for the green LED
  //rtc_set_compilation_data(); // Set RTC to the compilation time of the code
  rtc_set_week_day(6); // Set the day of the week to Monday (0)

  //rtc_read_time(&hour, &min, &sec);
  rtc_read_date(&day, &month, &year);
  current_day_of_week = rtc_get_week_day(); // Adjust to 0=Monday
  next_alarm_day = current_day_of_week; // Initialize next_alarm_day to the current day
}


int main(){
  init();

  unsigned long int millis_last = millis();

  LiquidCrystalDisplay lcd(&PORTD, &DDRD, LCD_RS_PIN, LCD_E_PIN, LCD_DATA_PINS_OFFSET);
  lcd.init();
  lcd.clear();

  while (1){
    static uint8_t checks_ticks_counter = 0;
    if (millis() - millis_last >= 10) {

      button_pressed = check_button();
      speaker_check();
      if (button_60_sec_pressed_check()) {
        speaker_active = false;
        speaker_play(600);
        _delay_ms(200);
        speaker_stop();
      }

      if (checks_ticks_counter >= 5) { //update czasu i wyswietlacza co 50ms
        checks_ticks_counter = 0;
        if (second_changed_check()){
          current_day_of_week = rtc_get_week_day();
          lcd_refresh(lcd);

          if ((hour == alarm[current_day_of_week].hour) && (min == alarm[current_day_of_week].min) && (sec == 0)){
            speaker_active = true;
          }
        }
      }
      
      checks_ticks_counter++;
      millis_last = millis();
    }
  }
}


void timer0_init() {
    // TCCR0A (Timer/Counter Control Register A) - rejestr sterujący. Włącza tryb CTC (reset timera po osiągnięciu wartości).
    TCCR0A |= (1 << WGM01);
    // TCCR0B (Timer/Counter Control Register B) - rejestr sterujący. Włącza preskaler (dzielnik zegara) na 64.
    TCCR0B |= (1 << CS01) | (1 << CS00);
    // OCR0A (Output Compare Register A) - rejestr wartości do porównania. Liczy do 249 (co daje 1 ms przy 16 MHz).
    OCR0A = 249;
    // TIMSK0 (Timer/Counter Interrupt Mask Register) - włącza wyzwalanie przerwania dla Timera 0.
    TIMSK0 |= (1 << OCIE0A);
    // sei() (Set Global Interrupt Flag) - włączenie obsługi przerwań.
    sei();
}
ISR(TIMER0_COMPA_vect) {
    timer_millis++;
}
unsigned long millis() {
    unsigned long czas;
    // ATOMIC_BLOCK wyłącza przerwania na czas kopiowania danych, aby uniknąć błędów odczytu na 8-bitowym mikrokontrolerze.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        czas = timer_millis;
    }
    return czas;
}

bool second_changed_check(){
  static uint8_t second_last = 0;
  rtc_read_time(&hour, &min, &sec);
  if (sec != second_last){
    second_last = sec;
    return true;
  }
  return false;
}
void lcd_refresh(LiquidCrystalDisplay &lcd){
    lcd.LCD_line(0);
    sprintf(text, "%02u:%02u:%02u()%02u:%02u", hour, min, sec, alarm[current_day_of_week].hour, alarm[current_day_of_week].min);
    lcd.send_text(text);

    lcd.LCD_line(1);
    rtc_read_date(&day, &month, &year);
    sprintf(text, "%02u.%02u.20%02u   ", day, month, year);
    lcd.send_text(text); 
    lcd.send_text(get_day_of_week_string(current_day_of_week)); 

}
bool check_button() {
  static uint8_t history = 0xFF;
    history = (history << 1) | ((PINB & (1 << PB2)) ? 1 : 0);
    if (!(history & 0x0F)) {
        sbi(PORTB, 3); // Turn on the green LED when the button is pressed
        return 1;
    }
    cbi(PORTB, 3); // Turn off the green LED when the button is not pressed
    return 0;
}
bool button_60_sec_pressed_check(){
  static uint16_t press_ticks = 0;

  if (button_pressed && speaker_active) {
    press_ticks++;
    if (press_ticks >= 5000) {
      press_ticks = 0;
      return true;
    }
    else {
      return false;
    }
  } 
  else {
    press_ticks = 0; // reset counter if button is released
    return false;
  }
}
void speaker_check() {
  static unsigned long last_speaker_check = 0;
  unsigned long now = millis();

  if (speaker_active) {
    if (!button_pressed) {
      if (now - last_speaker_check >= 1800) {
        speaker_play(3000);
        last_speaker_check = now;
      } 
      else if (now - last_speaker_check >= 900) {
        speaker_stop();
      } 
      else if (now - last_speaker_check >= 600) {
        speaker_play(800);
      } 
      else if (now - last_speaker_check >= 300) {
        speaker_stop();
      }
      else {
        speaker_play(800);
      }
    }
    else {
      speaker_stop();
    }
  }
}
char* get_day_of_week_string(uint8_t day_of_week) {
    switch (day_of_week) {
        case 0: return "Mon";
        case 1: return "Tue";
        case 2: return "Wed";
        case 3: return "Thu";
        case 4: return "Fri";
        case 5: return "Sat";
        case 6: return "Sun";
        default: return "err";
    }
}