#pragma once
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

void I2C_init(void){
    cbi(TWSR, TWPS0); // Preskaler = 1
    cbi(TWSR, TWPS1);
    TWBR = 72; // Baud rate = 100kHz przy F_CPU = 16MHz
    
    TWCR = (0 |(1<<TWEN)); // rejestr sterujący - włączenie TWI
}

void I2C_start(void){
    TWCR = (1<<TWSTA) | (1<<TWEN) | (1<<TWINT); // Wysłanie sygnału START
    while (!(TWCR & (1<<TWINT))); // Czekanie na zakończenie transmisji
}

void I2C_stop(void){
    TWCR = (1<<TWSTO) | (1<<TWEN) | (1<<TWINT);
    while (TWCR & (1<<TWSTO)); // Sprzęt sam zeruje TWSTO po zakończeniu
}

uint8_t I2C_read(bool ack = false){
    if (ack){
        TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWEA); // Włączenie transmisji i wysłanie ACK
    } else {
        TWCR = (1<<TWEN) | (1<<TWINT); // Włączenie transmisji bez wysyłania ACK
    }
    while (!(TWCR & (1<<TWINT))); // Czekanie na zakończenie transmisji
    return TWDR; // Zwracamy odczytany bajt danych
}

void I2C_send(uint8_t data){
    TWDR = data; // Wczytanie danych do rejestru danych
    TWCR = (1<<TWEN) | (1<<TWINT); // Włączenie transmisji
    while (!(TWCR & (1<<TWINT))); // Czekanie na zakończenie transmisji
}