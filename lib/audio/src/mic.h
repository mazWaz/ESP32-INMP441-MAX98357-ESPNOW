#pragma once

#include <Arduino.h>
#include <driver/i2s.h>

class mic
{
private:
    int32_t *rawSamples = nullptr;
    int rawSamplesSize;
    int micReadHead;
    const int16_t noiseThreshold = 6;
    
    i2s_port_t i2sPort;
    i2s_pin_config_t i2s_mic_pins;

    i2s_config_t i2s_mic_Config;


    
public:
    mic(i2s_port_t _i2sPort, i2s_pin_config_t &_i2sPin, i2s_config_t &_i2sConfig, int size);
    ~mic();
    void configMic();
    void startMic();
    int read(int16_t *samples, int count);
    void stopAudio();
};
