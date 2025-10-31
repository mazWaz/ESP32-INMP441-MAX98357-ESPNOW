#pragma once
#include "Arduino.h"

class button
{
private:
    int _pin;
    bool mode;
    bool lastMode = false;
    bool removeData;
    bool lastRemoveData = false;

    int buttonState;
    int lastButtonState;

    unsigned long lastDebounceTime;
    unsigned long debounceDelay;

    unsigned long longPressTime;
    unsigned long longPressDelay;
    bool isLongPress;

    unsigned long lastClickTime;
    unsigned long doubleClickTimeout;
    byte clickCount;

    void onDoubleClick();
    void onLongPress();

public:
    button(int pin) : _pin(pin) {
        mode = false;
        removeData = false;

        lastButtonState = HIGH;
        buttonState = HIGH;
        lastDebounceTime = 0;
        longPressTime = 0;
        lastClickTime = 0;
        clickCount = 0;
        isLongPress = false;

        debounceDelay = 50;
        longPressDelay = 1000;
        doubleClickTimeout = 300;
    }

    void begin();
    bool getMode() {
        if (mode != lastMode) {
            lastMode = mode;
            return mode;
        }else
        {
            return false;
        }
    }
    bool setMode(bool value) {
        mode = value;
        return mode;
    }
    bool getRemove() { 
        if (removeData != lastRemoveData) {
            lastRemoveData = removeData;
            return removeData;
        }else
        {
            return false;
        }
    }
    bool setRemove(bool value) {
        removeData = value;
        return removeData;
    }
    void tick();
};
