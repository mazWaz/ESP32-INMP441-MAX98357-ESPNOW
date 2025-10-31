#include "displayTask.h"

// This global variable seems out of place. Consider moving it inside a class if possible.
bool whenLoop;

#include "../../lib/ui/eez-flow.h"
#include "Arduino.h"

using namespace eez;
using namespace eez::flow;

// External function declarations
extern int32_t getVol();
extern const char *getAddress();
extern const char *getDevice1();
extern const char *getAddress2();
extern const char *getDevice2();
extern const char *getChoiseDevice();
extern bool switchLoop();
extern int32_t getSirenTone();
extern int32_t *getModes();
extern int getButton();
extern int currentState();
extern void setCurrentState(int state);
extern void setHidden(int button, bool hidden);
extern void setAddress(const char *data);
extern void setDevice1(const char *data);
extern void setAddress2(const char *data);
extern void setDevice2(const char *data);
extern void setVol(int32_t vol);   
extern void setModes(int32_t* arr, size_t len);
extern void setPlayButton(int button);

#ifndef TRANSPORT_HEADER_SIZE
#define TRANSPORT_HEADER_SIZE 0
#endif

#define ESP_NOW_WIFI_CHANNEL 1

uint8_t transportHeader[TRANSPORT_HEADER_SIZE] = {};

displayTask::displayTask() : 
    mScreen(new Screen()),
    mMemory(new storage()),
    mOutputBuffer(new Buffer(300 * 16)),
    mCommunication(new commEspNow(mOutputBuffer, mMemory, ESP_NOW_WIFI_CHANNEL)),
    toneSelected(nullptr),
    addr1(nullptr),
    addr2(nullptr),
    m_device1(nullptr),
    m_device2(nullptr),
    currentModes(9, 0) // Initialize vector with 9 zeros
{
    _initSemaphore = xSemaphoreCreateBinary();
    mCommunication->setHeader(TRANSPORT_HEADER_SIZE, transportHeader);
    memset(macStr, 0, sizeof(macStr));
}

displayTask::~displayTask()
{
    // Proper cleanup
    if (mCommunication) { delete mCommunication; }
    if (mOutputBuffer) { delete mOutputBuffer; }
    if (mMemory) { delete mMemory; }
    if (mScreen) { delete mScreen; }
    // No need to delete _getMode, std::vector handles it.
}

std::vector<uint8_t> displayTask::convertTouint8t(const std::vector<int32_t>& dataInt) {
    std::vector<uint8_t> result;
    if (dataInt.empty()) {
        return result;
    }

    result.reserve(dataInt.size());
    for (int32_t value : dataInt) {
        result.push_back(static_cast<uint8_t>(value));
    }

    return result;
}

bool displayTask::initializeProcessData(){
    if (!mMemory->init()) {
        Serial.println("ERROR: Storage initialization failed!");
        return false;
    }
    Serial.println("Storage initialized successfully");
    
    if (!mCommunication->begin()) {
        Serial.println("ERROR: Communication initialization failed!");
        return false;
    }
    Serial.println("Communication initialized successfully");
    
    toneSelected = mMemory->readModeTones();
    currentVolume = mMemory->getVolume();
    addr1 = mMemory->getMac();
    addr2 = mMemory->getMac1();
    m_device1 = mMemory->device1();
    m_device2 = mMemory->device2();

    xSemaphoreGive(_initSemaphore); 

    return true;
}

bool displayTask::lvglInit(){
    xSemaphoreTake(_initSemaphore, portMAX_DELAY); 

    if (!mScreen->begin()) {
        Serial.println("ERROR: Screen initialization failed!");
        return false;
    }
    Serial.println("Screen initialized successfully");
    
    ui_init();
    setVol(currentVolume);

    if (toneSelected != nullptr) {
        setModes(toneSelected, 9);
        currentModes.assign(toneSelected, toneSelected + 9);
    } else {
        int32_t defaultModes[9] = {0};
        setModes(defaultModes, 9);
    }

    if (addr1 == nullptr) {
        setAddress("");
        setDevice1("");
        setHidden(FLOW_GLOBAL_VARIABLE_TOP_BUTTON_HIDDEN, true);
    } else {
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                addr1[0], addr1[1], addr1[2], addr1[3], addr1[4], addr1[5]);
        setAddress(macStr);
        setDevice1(m_device1);
        setHidden(FLOW_GLOBAL_VARIABLE_TOP_BUTTON_HIDDEN, false);
    }

    if (addr2 == nullptr) {
        setAddress2("");
        setDevice2("");
        setHidden(FLOW_GLOBAL_VARIABLE_HIDDEN_MASSAGE, true);
    } else {
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                addr2[0], addr2[1], addr2[2], addr2[3], addr2[4], addr2[5]);
        setAddress2(macStr);
        setDevice2(m_device2);
        setHidden(FLOW_GLOBAL_VARIABLE_HIDDEN_MASSAGE, false);
    }
    Serial.println("UI initialized successfully");

    return true;
}

bool displayTask::begin(){
    Serial.println("Application started");
    Serial.print("My IDF Version is: ");
    Serial.println(esp_get_idf_version());
    return true;
}

void displayTask::tick(){
    if (!mScreen) {
        Serial.println("ERROR: Screen object is null!");
        return;
    }
    mScreen->lvHandler();
    ui_tick();
}

void displayTask::loop(){
    // Update all local state variables from UI/external functions
    deviceSelected = getChoiseDevice();
    currentVolume = map(getVol(), 0, 100, 0, 90);
    currentSwitchState = switchLoop();
    currentButtonState = getButton();
    currentSirenTone = getSirenTone();
    currentStateValue = currentState();

    int32_t* modesFromUI = getModes();
    if (memcmp(currentModes.data(), modesFromUI, 9 * sizeof(int32_t)) != 0) {
        currentModes.assign(modesFromUI, modesFromUI + 9);
    }

    if (!currentSwitchState && currentButtonState != 2) {
        setPlayButton(0);
    }
}

void displayTask::processData(){
    if (lastStateValue == currentStateValue) {
        // No new action, handle continuous actions if any
        if (currentStateValue == actionState_none && deviceSelected) {
            uint8_t* macAddress = nullptr;
            if (m_device1 && strcmp(deviceSelected, m_device1) == 0) macAddress = addr1;
            else if (m_device2 && strcmp(deviceSelected, m_device2) == 0) macAddress = addr2;

            if (macAddress && mCommunication->addPeer(macAddress)) {
                mCommunication->sendDataBool(currentSwitchState);
                if (currentSwitchState) {
                    if (currentButtonState == 1) { //release
                        mCommunication->sendDataInt(0, TEST);
                    } else if (currentButtonState == 2) { //pressed
                        mCommunication->sendDataInt(currentSirenTone, TEST);
                    } else if (currentButtonState == 3) { //close
                        mCommunication->sendDataInt(63, TEST);
                    }
                } else {
                    if (currentButtonState == 2) {
                        mCommunication->sendDataInt(currentSirenTone, TEST);
                    } else {
                        mCommunication->sendDataInt(0, TEST);
                    }
                }
            }
        }
        return;
    }

    // A new state/action has been triggered
    lastStateValue = currentStateValue;
    uint8_t* macAddress = nullptr;

    // Determine target MAC address based on selected device
    if (deviceSelected) {
        if (m_device1 && strcmp(deviceSelected, m_device1) == 0) macAddress = addr1;
        else if (m_device2 && strcmp(deviceSelected, m_device2) == 0) macAddress = addr2;
    }

    switch (lastStateValue) {
        case actionState_binding: {
            if (mCommunication->binding()) {
                Serial.printf("Binding successful for %s\n", deviceSelected);
            } else {
                Serial.println("Binding failed");
            }
            setCurrentState(actionState_none); // Reset state
            break;
        }

        case actionState_delete_address: {
            if (mMemory->hapusAlamat(deviceSelected)) {
                Serial.println("Address deleted successfully");
                // Reload addresses after deletion
                addr1 = mMemory->getMac();
                addr2 = mMemory->getMac1();
            } else {
                Serial.println("Failed to delete address");
            }
            setCurrentState(actionState_none); // Reset state
            break;
        }

        case actionState_save_sirine: {
            if (macAddress) {
                mCommunication->addPeer(macAddress);
                std::vector<uint8_t> dataTones = convertTouint8t(currentModes);
                if (!dataTones.empty()) {
                    mCommunication->sendModeSiren(dataTones.data());
                }
                mMemory->writeMode(currentModes.data(), currentModes.size());
            }
            setCurrentState(actionState_none); // Reset state
            break;
        }

        case actionState_send_volume: {
            if (macAddress) {
                mCommunication->addPeer(macAddress);
                if (mCommunication->sendDataInt(currentVolume, VOL) == 1) {
                    mMemory->saveVolume(currentVolume);
                }
            }
            setCurrentState(actionState_none); // Reset state
            break;
        }
        
        case actionState_none:
        default:
            // Handled in the continuous action section above
            break;
    }
}
