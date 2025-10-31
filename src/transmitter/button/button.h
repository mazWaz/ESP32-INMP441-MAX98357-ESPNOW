#pragma once

// #include "OneButton.h"
// #include <driver/gpio.h>
#include <Arduino.h>
#include "config.h"


class button
{
private:
    //banjar (kabel kuning)
    const int pin1 = ROW_1;
    const int pin2 = ROW_2;
    const int pin3 = ROW_3;
    ////baris (kabel biru)
    const int pin4 = COL_1;
    const int pin5 = COL_2;
    const int pin6 = COL_3;
    const int pin7 = COL_4;
    int massage;

    const int rowPins[3];
    const int colPins[4];

    char getKey(int row, int col);
    int lastPressedKey = 0; 

public:
    button()
        : rowPins{pin1,pin2,pin3}, colPins{pin4,pin5,pin6,pin7} { 
            massage = 0;
    }

    inline void begin();
    inline void checkKey();
    int getButton() { return massage; }
    void setButton() { massage = 0; }
};

inline void button::begin(){
    for (int i = 0; i < 3; i++)
    {
        pinMode(rowPins[i], INPUT_PULLUP);
    }
    
    for (int i = 0; i < 4; i++)
    {
        pinMode(colPins[i], OUTPUT);
        digitalWrite(colPins[i], HIGH);
    }
}

// Membuat fungsi getKey dengan parameter tambahan untuk mode 1 sampai 8
// Fungsi getKey yang lebih efektif dengan input array int
inline char button::getKey(int row, int col){
    int keys[3][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {-9, 254, 255, -4}
    };
    return keys[row][col];
}

inline void button::checkKey() {
    int currentPressedKey = 0; // Variabel lokal untuk menyimpan tombol yang ditekan pada pemindaian ini
    bool keyFoundThisScan = false;

    // Pindai seluruh keypad untuk menemukan tombol yang ditekan
    for (int col = 0; col < 4; col++) {
        digitalWrite(colPins[col], LOW);
        for (int row = 0; row < 3; row++) {
            if (digitalRead(rowPins[row]) == LOW) {
                currentPressedKey = getKey(row, col);
                keyFoundThisScan = true;
                break;
            }
        }
        digitalWrite(colPins[col], HIGH);
        if (keyFoundThisScan) {
            break; 
        }
    }
    if (keyFoundThisScan && currentPressedKey != lastPressedKey) {
        massage = currentPressedKey;     
        lastPressedKey = currentPressedKey; 
    }
    else if (!keyFoundThisScan && lastPressedKey != 0) {
        massage = 0;        
        lastPressedKey = 0; 
    }
}
