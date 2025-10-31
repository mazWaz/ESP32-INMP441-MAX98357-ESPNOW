#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "storage.h"
#include "buffer.h"

const int maxEspNowPacketSize = 127;
typedef enum {
    MODE_SPEAKER = 0,
    MODE_SIRINE_TONE = 1,
    MODE_INISIALITATION = 2
} typemode;

typedef enum {
    VOL = 0x56,   // 'V'
    TEST = 0x54,  // 'T'
    BOOL = 0x42,  // 'B'
    REMOT = 0x52, // 'R'
} espNowHeader;
class commEspNow {
private:
    typedef struct message {
        uint8_t buffer[maxEspNowPacketSize];
        char data[20] = "";
        int dataLen;
    } message;

    message messageData;
    Buffer* audioBuffer;
    storage* memoryStorage;
    uint8_t wifiChannel;
    char jsonBufferLocal[sizeof(messageData.data) + 1];
    int bufferSize;
    int index;
    int headerSize;
    int lastData;
    bool isLoop = false;
    bool lastBool;
    int buttonValue = 0;
    const uint8_t* mac = nullptr;

    SemaphoreHandle_t _commMutex;   

public:
    commEspNow(Buffer* audioBuffer, storage* memoryStorage, uint8_t wifiChannel);
    ~commEspNow();
    bool begin();
    bool addPeer();
    bool addPeer(const uint8_t* _mac);
    void sendData();
    
    // Fungsi audio
    void addSample(int16_t sample);
    void flush();

    // sending data 
    int sendDataInt(int data, uint8_t header);
    bool sendDataBool(bool data);
    void sendModeSiren(const uint8_t *modelBuffer);
    // recieve data
    bool parsingData();
    int getButtonValue();
    int getMode();
    bool getBool();
    
    // Fungsi binding
    void statusBinding();
    bool binding();
    
    // Header settings
    int setHeader(const int headerSize, const uint8_t* header);
    
    // Friend fungsi untuk callback
    friend void receiverCallback(const uint8_t* macAddr, const uint8_t* data, int dataLen);
    friend void transmitterCallback(const uint8_t* macAddr, const uint8_t* data, int dataLen);
    friend void displayCallback(const uint8_t* macAddr, const uint8_t* data, int dataLen);
};

// Callback function declarations
void receiverCallback(const uint8_t* macAddr, const uint8_t* data, int dataLen);
void transmitterCallback(const uint8_t* macAddr, const uint8_t* data, int dataLen);
void displayCallback(const uint8_t* macAddr, const uint8_t* data, int dataLen);