#include "button.h"

void button::begin() {
    pinMode(_pin, INPUT_PULLUP);
}

void button::onDoubleClick() {
    mode = true;
}

void button::onLongPress() {
    removeData = true;
}

void button::tick() {
    int reading = digitalRead(_pin);

    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading;

            if (buttonState == LOW) {
                isLongPress = false;
                longPressTime = millis();
            }
            else {
                if (!isLongPress) {
                    clickCount++;
                    lastClickTime = millis();
                }
            }
        }
    }

    lastButtonState = reading;

    if (buttonState == LOW && !isLongPress) {
        if ((millis() - longPressTime) > longPressDelay) {
            onLongPress();
            isLongPress = true;
        }
    }

    if (clickCount > 0 && (millis() - lastClickTime) > doubleClickTimeout) {
        if (clickCount == 2) {
            onDoubleClick();
        }
        clickCount = 0;
    }
}
