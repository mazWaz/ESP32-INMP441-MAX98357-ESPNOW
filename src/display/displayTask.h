#ifndef DISPLAYTASK_H
#define DISPLAYTASK_H

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// #include "eez-flow.h"
#include "ui.h"
#include "vars.h"
#include "screen/screen.h"
#include "storage.h"
#include "Buffer.h"
#include "commEspNow.h"
#include "structs.h" 

#include <array>
#include <vector>

class Screen;
class storage;
class Buffer;
class commEspNow;

class displayTask {
public:
    displayTask();
    ~displayTask();

    bool begin();
    bool initializeProcessData();
    bool lvglInit();
    void tick();
    void loop();
    void processData();

private:
    std::vector<uint8_t> convertTouint8t(const std::vector<int32_t>& dataInt);

    // Pointers to managed objects
    Screen* mScreen;
    storage* mMemory;
    Buffer* mOutputBuffer;
    commEspNow* mCommunication;
    
    // Data from storage
    int32_t* toneSelected;        
    uint8_t* addr1;
    uint8_t* addr2;
    char *m_device1;
    char *m_device2;

    // State variables
    int currentStateValue = 0;
    int lastStateValue = 0;
    
    char macStr[18]; // MAC string format is 17 chars + null terminator
    const char *deviceSelected = nullptr;
    std::vector<int32_t> currentModes;
    int currentVolume = 0;
    bool currentSwitchState = false;
    int currentButtonState = 0;
    int currentSirenTone = 0;

    SemaphoreHandle_t _initSemaphore;
};
#endif // DISPLAYTASK_H
