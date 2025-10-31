#pragma once 

#include <Arduino.h>
#include <driver/i2s.h>

#define SAMPLE_RATE_WAVE 44100
#define WAVE_TABLE_SIZE 128
#define N_VALUE 0.4f
#define M_VALUE 0.4f

class sirine
{
private:
    i2s_pin_config_t i2sSpeakerPins;
    i2s_port_t i2sPort;
    i2s_config_t i2s_config;

    uint8_t volume;      
    uint8_t mode;      
    uint8_t envelopeVolume;  
    uint32_t phaseAccumulator;
    unsigned long modeStartTime;

    int16_t waveTable[WAVE_TABLE_SIZE];
    int16_t combineTable[WAVE_TABLE_SIZE];
    int16_t airhornTable[WAVE_TABLE_SIZE];
    int16_t toneTable[WAVE_TABLE_SIZE];
    volatile uint32_t phaseStep = 0;

    const uint8_t modeTableMap(int mode);
public:
    sirine(i2s_port_t _i2sPort, i2s_pin_config_t &_i2sSpeakerPins,i2s_config_t &_i2s_config);
    ~sirine();
    void startSirine();
    void stopSirine();
    void generateWaveTable();
    void generateI2sTone(uint8_t _mode);
    void generateSineWave(int vol);
    void cleanBuffer();
};
