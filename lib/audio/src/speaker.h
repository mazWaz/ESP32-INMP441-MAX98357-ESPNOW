#pragma once

#include <Arduino.h>
#include <driver/i2s.h>

class speaker
{
private:
    int16_t *frames;
    int rawSamplesSize;
    i2s_pin_config_t i2sSpeakerPins;
    i2s_port_t i2sPort;
    i2s_config_t i2s_config;
public:
    speaker(i2s_port_t _i2sPort, i2s_pin_config_t &_i2sSpeakerPins,i2s_config_t &_i2s_config, int size);
    ~speaker();
    void startSpeaker();
    void stopAudio();
    int16_t processSample(int16_t sample){return sample;}
    void write(int16_t *samples, int count);
};
