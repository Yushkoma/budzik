#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "I2C_lib.h"

#define RTC_ADDR 0x68

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

// Zamiana BCD na dec
uint8_t bcdToDec(uint8_t val) {
  return ((val / 16 * 10) + (val % 16));
}

// Zamiana dec na BCD
uint8_t decToBcd(uint8_t val) {
  return ((val / 10 * 16) + (val % 10));
}

void rtc_set_time(uint8_t godzina, uint8_t minuta, uint8_t sekunda) {
    I2C_start(); // Budzimy magistralę
    
    I2C_send((RTC_ADDR << 1) | 0); // (0xD0) Adres układu + kierunek: ZAPIS
    I2C_send(0x00); // Ustawiamy wskaźnik pamięci RTC na rejestr sekund (0x00)
    
    // Zapisujemy kolejne wartości skonwertowane na BCD.
    // Dzięki auto-inkrementacji RTC sam wskoczy na adres 0x01 i 0x02.
    I2C_send(decToBcd(sekunda)); 
    I2C_send(decToBcd(minuta));
    I2C_send(decToBcd(godzina));
    
    I2C_stop(); // Kończymy transmisję
}

// --- ODCZYT CZASU Z RTC ---

// Używamy tutaj wskaźników (gwiazdka przed zmienną), aby funkcja 
// mogła fizycznie podmienić wartości zmiennych zadeklarowanych w pętli loop.
void rtc_read_time(uint8_t *godzina, uint8_t *minuta, uint8_t *sekunda) {
    I2C_start();
    I2C_send((RTC_ADDR << 1) | 0); // (0xD0) Adres układu + ZAPIS
    I2C_send(0x00); // Informujemy RTC, że chcemy wystartować od sekund (0x00)
    
    I2C_start(); // RESTART - powtarzamy start, aby nie zrywać połączenia
    I2C_send((RTC_ADDR << 1) | 1); // (0xD1) Adres układu + ODCZYT
    
    // Pobieramy dane. Funkcja I2C_read_ack odesła potwierdzenie (chcę więcej danych),
    // a I2C_read_nack odesła sygnał zakończenia (nie wysyłaj więcej).
    // Surowy bajt z magistrali od razu wrzucamy w funkcję odkodowującą z BCD na DEC.
    
    *sekunda = bcdToDec(I2C_read(1)); 
    *minuta = bcdToDec(I2C_read(1));
    *godzina = bcdToDec(I2C_read(0)); // Przy ostatnim pobranym bajcie kończymy NACK-iem
    
    I2C_stop();
}