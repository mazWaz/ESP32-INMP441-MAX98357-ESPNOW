#ifndef CONFIG_H
#define CONFIG_H

#include <driver/gpio.h>
#include <driver/i2s.h>

// speaker settings
#define USE_I2S_SPEAKER_OUTPUT
#define I2S_SPEAKER_SERIAL_CLOCK GPIO_NUM_7
#define I2S_SPEAKER_LEFT_RIGHT_CLOCK GPIO_NUM_6
#define I2S_SPEAKER_SERIAL_DATA GPIO_NUM_8
// Shutdown line if you have this wired up or -1 if you don't
#define I2S_SPEAKER_SD_PIN -1

//binding button receiver
#define BINDING_BUTTON GPIO_NUM_14

// transmit button
#define GPIO_TRANSMIT_BUTTON GPIO_NUM_13

// I2S Microphone Settings
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_2 // sck
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_3 // ws
#define I2S_MIC_SERIAL_DATA GPIO_NUM_4 // sd

// Keypad rows/cols
const int ROW_1 = GPIO_NUM_9;
const int ROW_2 = GPIO_NUM_10;
const int ROW_3 = GPIO_NUM_11;
const int COL_1 = GPIO_NUM_15;
const int COL_2 = GPIO_NUM_16;
const int COL_3 = GPIO_NUM_17;
const int COL_4 = GPIO_NUM_18;

#define SAMPLE_RATE 48000 

#endif // PROJECT_APP_CONFIG_H