#include "speaker.h"
#include "soc/i2s_reg.h"
#include <esp_log.h>

static const char *TAG = "OUT";
speaker::speaker(i2s_port_t _i2sPort, i2s_pin_config_t &_i2sSpeakerPins,i2s_config_t &_i2s_config, int size){
    i2sPort =  _i2sPort;
    i2sSpeakerPins = _i2sSpeakerPins;
    i2s_config = _i2s_config;
    rawSamplesSize = size;
    frames = (int16_t *)malloc(sizeof(int16_t) * rawSamplesSize);
}

speaker::~speaker(){
    free(frames);
}

void speaker::startSpeaker(){
    i2s_driver_install(i2sPort, &i2s_config, 0, NULL);
    i2s_set_pin(i2sPort, &i2sSpeakerPins);
    i2s_zero_dma_buffer(i2sPort);

    i2s_start(i2sPort);
}

void speaker::stopAudio(){
    i2s_stop(i2sPort);
    i2s_driver_uninstall(i2sPort);
}

void speaker::write(int16_t *samples, int count){
    int sampleIndex = 0;
    while (sampleIndex < count)
    {
        int samplestoSend = 0;
        for (int i = 0; i < rawSamplesSize && sampleIndex < count; i++){
                int sample = processSample(samples[sampleIndex]);
                frames[i * 2] = sample;
                frames[i * 2 + 1] = sample;
                samplestoSend++;
                sampleIndex++;
        }
        size_t bytes_written = 0;
        i2s_write(i2sPort, frames, samplestoSend * sizeof(int16_t) * 2, &bytes_written, portMAX_DELAY);
        if (bytes_written != samplestoSend * sizeof(int16_t) * 2)
        {
            ESP_LOGE(TAG, "Did not write all bytes");
        }
    }
}
