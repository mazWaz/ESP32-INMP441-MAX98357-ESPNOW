#include "commEspNow.h"
#include <esp_log.h>
static const char *TAG = "commEspNow";

#ifdef RECEIVER
const char messaging[6] = "sound";
#endif
#ifdef TRANSMITTER
const char messaging[6] = "remot";
#endif
#ifdef DISP
const char messaging[6] = "displ";
#endif

static commEspNow* instance = NULL;

void receiverCallback(const uint8_t* macAddr, const uint8_t* data, int dataLen) {

    if (strcmp((char*)data, "remot") == 0 || strcmp((char*)data, "displ") == 0) {
        Serial.println("[DEBUG] Menerima esp32remote/displ, menyimpan MAC address...");
        if (instance->memoryStorage) {
            instance->memoryStorage->writeMacAddress(macAddr,(char*)data, 2);
        }
    } else {

        if (memcmp(macAddr, instance->memoryStorage->getMac(), 6) == 0 || memcmp(macAddr, instance->memoryStorage->getMac1(), 6) == 0) {
            if (dataLen > sizeof(instance->messageData)) {
                Serial.printf("[ERROR] Data length (%d) too large for messageData (%d). Truncating.\n", dataLen, sizeof(instance->messageData));
                memcpy(&instance->messageData, data, sizeof(instance->messageData));
            } else {
                memcpy(&instance->messageData, data, dataLen);
            }

            instance->messageData.data[sizeof(instance->messageData.data) - 1] = '\0'; 
            xSemaphoreTake(instance->_commMutex, portMAX_DELAY);
            strncpy(instance->jsonBufferLocal, instance->messageData.data, sizeof(instance->messageData.data));
            instance->jsonBufferLocal[sizeof(instance->messageData.data)] = '\0';
            xSemaphoreGive(instance->_commMutex);

            if (instance->messageData.dataLen == MODE_SPEAKER && strlen(instance->messageData.data) == 0) {
                int headerSize = instance->headerSize;
                if (instance->audioBuffer) {
                    size_t actual_data_len = sizeof(instance->messageData.buffer) - headerSize;
                    if (actual_data_len > 0) {
                        instance->audioBuffer->addBuffer(instance->messageData.buffer + headerSize, actual_data_len);
                    }
                }
            } else if (instance->messageData.dataLen == MODE_INISIALITATION && strlen(instance->messageData.data) == 0) {
                instance->memoryStorage->writeMode(instance->messageData.buffer, 9);
            }
            
        } else {
            Serial.println("[DEBUG] MAC address tidak cocok dengan yang tersimpan di storage");
        }
    }
}

void transmitterCallback(const uint8_t* macAddr, const uint8_t* data, int dataLen) {
    if (!instance || !macAddr || !data) {
        Serial.println("[ERROR] Null pointer in transmitterCallback");
        return;
    }
    
    if (dataLen <= 0) {
        Serial.println("[ERROR] Invalid data length in transmitterCallback");
        return;
    }

    if (strcmp((char*)data, "sound") == 0) {
        Serial.println("[DEBUG] Menerima sound, menyimpan MAC address...");
        if (instance->memoryStorage) {
            instance->memoryStorage->writeMacAddress(macAddr,(char*)data, 2);
        }
    }
}

void displayCallback(const uint8_t* macAddr, const uint8_t* data, int dataLen) {
    if (strcmp((char*)data, "sound") == 0) {
        Serial.println("[DEBUG] Menerima sound/, menyimpan MAC address...");
        instance->memoryStorage->writeMacAddress(macAddr,(char*)data, 2);
    }
}

commEspNow::~commEspNow() {
}

commEspNow::commEspNow(Buffer* audioBuffer, storage* memoryStorage, uint8_t wifiChannel) {
    _commMutex = xSemaphoreCreateMutex();
    Serial.println("[DEBUG] Inisialisasi commEspNow");
    instance = this;
    this->audioBuffer = audioBuffer;
    this->memoryStorage = memoryStorage;
    this->wifiChannel = wifiChannel;
    this->bufferSize = maxEspNowPacketSize;
    this->index = 0;
    this->headerSize = 0;
    this->lastData = 0;
    this->mac = nullptr;
}

bool commEspNow::begin() {
    Serial.println("[DEBUG] Memulai inisialisasi ESP-NOW");
    
    if (memoryStorage == nullptr) {
        Serial.println("[ERROR] memoryStorage tidak terinisialisasi");
        return false;
    }

    delay(100);
    
    WiFi.mode(WIFI_STA);
    delay(50);
    WiFi.disconnect();
    delay(100);
    
    Serial.print("[DEBUG] MAC Address: ");
    Serial.println(WiFi.macAddress());

    Serial.println("[DEBUG] Setup WiFi channel");
    esp_wifi_set_promiscuous(true);
    delay(10);
    esp_wifi_set_channel(wifiChannel, WIFI_SECOND_CHAN_NONE);
    delay(10);
    esp_wifi_set_promiscuous(false);
    delay(50);

    Serial.println("[DEBUG] Inisialisasi ESP-NOW");
    esp_err_t result = esp_now_init();
    if (result == ESP_OK) {
        delay(50);
        
        #ifdef RECEIVER
            Serial.println("[DEBUG] ESPNow Init in Receiver Success");
            esp_now_register_recv_cb(receiverCallback);
        #endif
        
        #ifdef TRANSMITTER
            Serial.println("[DEBUG] ESPNow Init in Transmitter Success");
            esp_now_register_recv_cb(transmitterCallback);
        #endif
        
        #ifdef DISP
            Serial.println("[DEBUG] ESPNow Init in Display Success");
            esp_now_register_recv_cb(displayCallback);
        #endif
        
        delay(50);
    } else {
        Serial.printf("[ERROR] ESPNow Init failed: %s\n", esp_err_to_name(result));
        return false;
    }

    Serial.println("[DEBUG] Inisialisasi ESP-NOW berhasil");
    return true;
}

bool commEspNow::addPeer() {
    uint8_t* mac = nullptr;

    if (strcmp(memoryStorage->device1(), "sound") == 0) {
        mac = memoryStorage->getMac();
    } else if (strcmp(memoryStorage->device2(), "sound") == 0) {
        mac = memoryStorage->getMac1();
    }

    if (!mac || mac[0] == 0) {
        return false; 
    }

    if (esp_now_is_peer_exist(mac)) {
        return true;
    }

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_STA;

    esp_err_t result = esp_now_add_peer(&peerInfo);
    
    if (result == ESP_OK) {
        Serial.printf("Peer added successfully with MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return true;
    } else {
        Serial.printf("[ERROR] Failed to add peer: %s\n", esp_err_to_name(result));
        return false;
    }
}

bool commEspNow::addPeer(const uint8_t* _mac){
    mac = _mac;
    if (mac == nullptr || mac[0] == 0) {
        return false;
    }

    if (!esp_now_is_peer_exist(mac)) {
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, mac, 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;
        peerInfo.ifidx = WIFI_IF_STA;
        esp_err_t res = esp_now_add_peer(&peerInfo);
        if (res == ESP_OK) {
            Serial.println("Peer berhasil ditambahkan.");
            return true;
        } else {
            Serial.print("Gagal menambah peer: ");
            Serial.println(esp_err_to_name(res));
            return false;
        }
    }
    return true;
}

void commEspNow::sendData() {
    esp_err_t result = esp_now_send(mac, (uint8_t*)&messageData, sizeof(messageData));
    if (result != ESP_OK) {
        Serial.printf("Send Error: %s\n", esp_err_to_name(result));
        
        if (result == ESP_ERR_ESPNOW_NO_MEM) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void commEspNow::addSample(int16_t sample) {
    messageData.buffer[index + headerSize] = (sample + 16384) >> 8;
    index++;

    if ((index + headerSize) == bufferSize) {
        memset(messageData.data, 0, sizeof(messageData.data));
        messageData.dataLen = MODE_SPEAKER;
        sendData();
        index = 0;
    }
}

int commEspNow::sendDataInt(int data, uint8_t header) {
    bool isVolHeader = (header == VOL);
    if (!isVolHeader && data == lastData) {
        return 0;
    }

    Serial.print("[DEBUG] Mengirim data integer. Header: ");
    Serial.print(header);
    Serial.print(", Data: ");
    Serial.println(data);

    JsonDocument doc;
    doc["h"] = header;
    doc["d"] = data;

    serializeJson(doc, messageData.data);
    if (isVolHeader) {
        messageData.dataLen = MODE_INISIALITATION;
    } else {
        messageData.dataLen = MODE_SIRINE_TONE;
    }

    sendData();
    memset(messageData.data, 0, sizeof(messageData.data));
    if (!isVolHeader) {
        lastData = data;
        Serial.print("[DEBUG] lastData diupdate menjadi: ");
        Serial.println(lastData);
    }
    return 1;
}

bool commEspNow::sendDataBool(bool data){
    if (data != lastBool)
    {
        JsonDocument doc;
        doc["h"] = BOOL;
        doc["d"] = data == true ? 1 : 0;
        serializeJson(doc, messageData.data);
        messageData.dataLen = MODE_SIRINE_TONE;
        sendData();
        memset(messageData.data, 0, sizeof(messageData.data));
        lastBool = data;
        return true;
    }else
    {
       return false; 
    }
}

void commEspNow::sendModeSiren(const uint8_t *modelBuffer){
    memcpy(messageData.buffer, modelBuffer, sizeof(messageData.buffer));
    memset(messageData.data, 0, sizeof(messageData.data));
    messageData.dataLen = MODE_INISIALITATION;
    sendData();
}

void commEspNow::flush() {
    if (index > 0) {
        sendData();
        index = 0;
    }
}

bool commEspNow::parsingData() {
    if (xSemaphoreTake(_commMutex, portMAX_DELAY) != pdTRUE) {
        return false;
    }

    bool success = false;
    if (strlen(jsonBufferLocal) > 0) {
        JsonDocument jsonDoc;
        DeserializationError error = deserializeJson(jsonDoc, jsonBufferLocal);
    if (!error) {
            uint8_t header = jsonDoc["h"];
            int value = jsonDoc["d"];

            switch (header) {
                case VOL:
                    instance->memoryStorage->saveVolume(value);
                    success = true;
                    break;
                case TEST:
                    instance->buttonValue = value;
                    success = true;
                    break;
                case BOOL:
                    instance->isLoop = (value == 1);
                    success = true;
                    break;
                case REMOT: {
                    int32_t* modeTones = instance->memoryStorage->readModeTones();
                    instance->buttonValue = (value >= 0 && value < 9) ? modeTones[value] : 0;
                    success = true;
                    break;
                }
                default:
                    // Unknown header, do nothing
                    break;
            }
        }
    }

    xSemaphoreGive(_commMutex);
    return success;
}

int commEspNow::getButtonValue() {
    int value = buttonValue;
    return value;
}

int commEspNow::getMode() {
    int value = messageData.dataLen;
    return value;
}

bool commEspNow::getBool(){
    bool value = isLoop;
    return value;
}

void commEspNow::statusBinding() {
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_STA;

    for (int i = 0; i < 6; i++) {
        peerInfo.peer_addr[i] = 0xFF;
    }

    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
        Serial.println(messaging);
        esp_now_send(peerInfo.peer_addr, (uint8_t*)messaging, 12);
        esp_now_del_peer(peerInfo.peer_addr);
    }
}

bool commEspNow::binding() {
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_STA;

    for (int i = 0; i < 6; i++) {
        peerInfo.peer_addr[i] = 0xFF;
    }

    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
        Serial.println(messaging);
        esp_err_t result = esp_now_send(peerInfo.peer_addr, (uint8_t*)messaging, 12);
        if (result != ESP_OK) {
            Serial.printf("Send Error: %s\n", esp_err_to_name(result));
        }else
        {
            Serial.printf("Send Success");
        }
        esp_now_del_peer(peerInfo.peer_addr);
        
        return true;
    }
    return false;
}

int commEspNow::setHeader(const int headerSize, const uint8_t* header) {
    if ((headerSize < bufferSize) && (header)) {
        this->headerSize = headerSize;
        memcpy(messageData.buffer, header, headerSize);
        return 0;
    } else {
        return -1;
    }
}
