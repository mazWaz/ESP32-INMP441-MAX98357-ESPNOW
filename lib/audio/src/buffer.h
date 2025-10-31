#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>

class Buffer
{
private:
    // outputBuffer
    uint8_t *buffer;
    SemaphoreHandle_t semaphore;
    int numberSamplesToBuffer;
    int readHead;
    int writeHead;
    int availableSamples;
    int bufferSize;
    bool buffering;
public:
    Buffer(int numberSample);
    ~Buffer();
    void addBuffer(const uint8_t *samples, int count);
    void removeBuffer(int16_t *samples, int count, int rank);
    void flush();
    bool getBuffer(){return buffering;}
};
