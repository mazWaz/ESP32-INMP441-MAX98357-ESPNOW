#include "mic.h"
#include "soc/i2s_reg.h"
#include <esp_log.h>
#include <algorithm>

mic::mic(i2s_port_t _i2sPort, i2s_pin_config_t &_i2sPin, i2s_config_t &_i2sConfig, int size)
    : i2sPort(_i2sPort),
      i2s_mic_pins(_i2sPin),
      i2s_mic_Config(_i2sConfig),
      rawSamplesSize(size),
      micReadHead(0)
{
    rawSamples = new int32_t[size];
}

mic::~mic()
{
    delete[] rawSamples;
    rawSamples = nullptr;
}

void mic::configMic()
{
    i2s_driver_install(i2sPort, &i2s_mic_Config, 0, NULL);
    i2s_set_pin(i2sPort, &i2s_mic_pins);
    i2s_start(i2sPort);
}

void mic::startMic()
{
    // i2s_zero_dma_buffer(i2sPort);
    // i2s_start(i2sPort);
}

int mic::read(int16_t *samples, int count)
{
    size_t bytes_read = 0;
    int read_count = std::min(count, rawSamplesSize);

    esp_err_t result = i2s_read(i2sPort, rawSamples, read_count * sizeof(int32_t), &bytes_read, portMAX_DELAY);
    if (result != ESP_OK) {
        ESP_LOGE("MIC", "I2S read failed with error: %d", result);
        return 0;
    }

    int samplesRead = bytes_read / sizeof(int32_t);

    for (int i = 0; i < samplesRead; i++)
    {
        int32_t temp = rawSamples[i] >> 13;
        samples[i] = (temp > INT16_MAX) ? INT16_MAX : (temp < -INT16_MAX) ? -INT16_MAX : (int16_t)temp;
    }
    return samplesRead;
}

void mic::stopAudio()
{
    // i2s_stop(i2sPort);
    // i2s_driver_uninstall(i2sPort);
}
