#pragma once

#include <stdint.h>

// BM (Bit Mask - Maska bitowa)
// Przesuwa jedynkę w lewo o podaną liczbę pozycji, tworząc maskę z jednym zapalonym bitem
#ifndef BM
#define BM(bit) (1 << (bit))
#endif

// sbi (Set Bit - Ustaw bit)
// Ustawia wybrany bit w rejestrze na stan wysoki (1) za pomocą operacji bitowej OR
#ifndef sbi
#define sbi(reg, bit) ((reg) |= BM(bit))
#endif

// cbi (Clear Bit - Wyzeruj bit)
// Zeruje wybrany bit w rejestrze (0) za pomocą operacji bitowej AND z zanegowaną maską
#ifndef cbi
#define cbi(reg, bit) ((reg) &= ~BM(bit))
#endif

// tbi (Toggle Bit - Przełącz bit)
// Zmienia stan wybranego bitu na przeciwny (0 na 1, 1 na 0) za pomocą operacji bitowej XOR
#ifndef tbi
#define tbi(reg, bit) ((reg) ^= BM(bit))
#endif