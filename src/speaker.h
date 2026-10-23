#ifndef SPEAKER_H
#define SPEAKER_H

#include <avr/io.h>

// Inicjalizacja sprzętowego wyjścia na pinie 9 (Arduino D9 / AVR PB1)
inline void speaker_init(void) {
    // DDRB (Data Direction Register B - rejestr kierunku przesyłu danych dla portu B)
    // PB1 (Port B bit 1 - odpowiada pinowi 9 na płytce Arduino)
    // Ustawienie bitu PB1 na 1 konfiguruje ten pin jako wyjście.
    DDRB |= (1 << PB1);

    // TCCR1A (Timer/Counter1 Control Register A - Rejestr A sterujący pracą Timera 1)
    // Ustawiamy wszystko na 0, aby zacząć z czystą kartą.
    TCCR1A = 0; 
    
    // TCCR1B (Timer/Counter1 Control Register B - Rejestr B sterujący pracą Timera 1)
    // WGM12 (Waveform Generation Mode, bit 2 - tryb generowania fali) 
    // Włącza tryb CTC (Clear Timer on Compare match). Zegar liczy w górę i samoczynnie zeruje się po zrównaniu z progiem.
    TCCR1B = (1 << WGM12); 

    // PORTB (Data Register Port B - rejestr stanu wyjść)
    // Wymuszamy stan niski (0V) na pinie na samym początku, aby układ głośnika był wyłączony.
    PORTB &= ~(1 << PB1);
}

// Odtwarzanie dźwięku o podanej częstotliwości w hercach (Hz)
inline void speaker_play(uint16_t freq) {
    if (freq == 0) {
        // Zatrzymanie zegara poprzez wyzerowanie bitów CS (Clock Select - wybór zegara)
        TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
        
        // COM1A0 (Compare Output Mode for Channel A - bit trybu wyjścia sprzętowego)
        // Odłączamy pin PB1 od automatycznego sterowania przez Timer 1.
        TCCR1A &= ~(1 << COM1A0);
        
        // Zabezpieczenie: ponownie wymuszamy fizyczne 0V na pinie. 
        // Tranzystor się zamyka, prąd przestaje płynąć przez głośnik.
        PORTB &= ~(1 << PB1); 
        return;
    }

    // Ponowne podłączenie pinu pod Timer 1.
    // Ustawienie COM1A0 sprawia, że sprzęt sam odwraca stan pinu (z 0V na 5V i w drugą stronę)
    // przy każdym osiągnięciu progu czasowego, tworząc falę prostokątną.
    TCCR1A |= (1 << COM1A0);

    // OCR1A (Output Compare Register 1 A - 16-bitowy rejestr przechowujący wartość progu do porównania)
    // Wzór wylicza limit liczenia dla fali prostokątnej przy zegarze systemowym 16 MHz i dzielniku (preskalerze) 64.
    OCR1A = (16000000 / (2 * 64 * freq)) - 1;

    // CS11 i CS10 ustawione naraz na 1 włączają sprzętowy preskaler (dzielnik częstotliwości) o wartości 64.
    // To uruchamia timer, który natychmiast zaczyna sterować pinem bez użycia procesora.
    TCCR1B |= (1 << CS11) | (1 << CS10);
    TCCR1B &= ~(1 << CS12);
}

// Funkcja pomocnicza do szybkiego wyciszania
inline void speaker_stop(void) {
    speaker_play(0);
}

#endif // SPEAKER_H