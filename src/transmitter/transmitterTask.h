#ifndef TRANSMITTER_TASK_H
#define TRANSMITTER_TASK_H

#include <Arduino.h>
#include <driver/gpio.h>

#include "commEspNow.h"
#include "mic.h"
#include "buffer.h"
#include "storage.h"
#include "button/button.h"

#define BYTE_RATE 1024

#define ESP_NOW_WIFI_CHANNEL 1

#define TRANSPORT_HEADER_SIZE 0

// Variabel yang dipindahkan dari config.cpp
extern uint8_t transportHeader[TRANSPORT_HEADER_SIZE];

class transmitterTask
{
private:
    commEspNow *mCommunication;
    storage *mMemory;
    mic *mInput; 
    Buffer *outBuffer;
    button *mButton;

    // int16_t *samples = reinterpret_cast<int16_t *>(malloc(sizeof(int16_t) * 128));
    int16_t *samples = nullptr;

    typedef enum{
        KEY_RELEASED = 0,
        KEY_DELETE_ADDRESS = 254,
        KEY_STATUS_BINDING = 255
    }sendingButton;
    
    int dataByte;
    int lastByte = 0;
    const TickType_t delayTicks = pdMS_TO_TICKS(5);
    void playButton();
public:
    transmitterTask();
    ~transmitterTask();
    void begin();
    void trasnmitData();
    void communication();
    void clearSample();
};

#endif // TRANSMITTER_TASK_H
