#include "storage.h"

storage::storage() {
    _semaphore = xSemaphoreCreateMutex(); // Buat mutex
    if (_semaphore == NULL) {
        Serial.println("[ERROR] Gagal membuat mutex storage!");
    }
}

storage::~storage() {
    if (_semaphore != NULL) {
        vSemaphoreDelete(_semaphore); // Hapus mutex saat objek dihancurkan
    }
}

bool storage::init() {
    xSemaphoreGive(_semaphore); // Pastikan semaphore dalam keadaan tersedia
    if (!SPIFFS.begin(true)) {
        Serial.println("[ERROR] Gagal menginisialisasi SPIFFS.");
        return false;
    }

    // Helper lambda untuk membaca file JSON
    auto readJsonFile = [](const char* path, JsonDocument& doc) -> bool {
        File file = SPIFFS.open(path, FILE_READ);
        if (!file) {
            // Tidak dianggap error jika file tidak ada, hanya info debug.
            // Serial.printf("[DEBUG] File tidak ditemukan: %s\n", path);
            return false;
        }
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        if (error) {
            Serial.printf("[ERROR] Gagal deserialize %s: %s\n", path, error.c_str());
            return false;
        }
        return true;
    };

    // Proses config.json
    JsonDocument configDoc;
    if (readJsonFile("/config.json", configDoc)) {
        for (int i = 0; i < 2; i++) {
            char key[10];
            sprintf(key, "address%d", i);
            
            JsonArray arr = configDoc[key].as<JsonArray>();
            if (!arr.isNull() && arr.size() >= 2) {
                const char* macStr = arr[0];
                const char* devName = arr[1];

                if (macStr && strlen(macStr) == 17) {
                    uint8_t* targetMac = (i == 0) ? configData.macAddress : configData.macAddress1;
                    char* targetName = (i == 0) ? configData.nameDevice1 : configData.nameDevice2;
                    
                    // Konversi string MAC ke array byte
                    for (int j = 0; j < 6; j++) {
                        targetMac[j] = strtoul(macStr + j * 3, NULL, 16);
                    }
                    
                    // Salin nama perangkat dengan aman
                    strncpy(targetName, devName, sizeof(configData.nameDevice1) - 1);
                    targetName[sizeof(configData.nameDevice1) - 1] = '\0';
                }
            }
        }
    }

    // Proses mode.json
    JsonDocument modeDoc;
    if (readJsonFile("/mode.json", modeDoc)) {
        configData.dataInt = modeDoc["volume"].as<int>(); // Default ke 0 jika tidak ada
        
        JsonArray modes = modeDoc["mode"].as<JsonArray>();
        if (!modes.isNull()) {
            size_t count = modes.size() < (sizeof(configData.modeArray)/sizeof(configData.modeArray[0])) ? modes.size() : (sizeof(configData.modeArray)/sizeof(configData.modeArray[0]));
            for (size_t i = 0; i < count; i++) {
                configData.modeArray[i] = modes[i].as<int>();
            }
        }
    }

    Serial.println("[INFO] Inisialisasi storage selesai.");
    return true;
}

void storage::writeMacAddress(const uint8_t *mac, const char *device, int count) {
    if (_semaphore != NULL && xSemaphoreTake(_semaphore, portMAX_DELAY) == pdTRUE) {
        // Validasi parameter input, ini sudah benar.
        if (mac == nullptr || device == nullptr || count <= 0) {
            Serial.println("[ERROR] Parameter tidak valid di writeMacAddress.");
            xSemaphoreGive(_semaphore);
            return;
        }

        JsonDocument doc;
        File fileRead = SPIFFS.open("/config.json", FILE_READ);
        if (fileRead) {
            DeserializationError error = deserializeJson(doc, fileRead);
            if (error) {
                Serial.printf("[ERROR] Gagal deserialize config.json: %s\n", error.c_str());
                // Tetap lanjutkan dengan doc kosong jika deserialisasi gagal
            }
            fileRead.close();
        }

        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        bool macAlreadyExists = false;
        int firstEmptySlot = -1;

        for (int i = 0; i < count; i++) {
            char key[10];
            sprintf(key, "address%d", i);

            if (doc[key].is<JsonArray>()) {
                JsonArray arr = doc[key].as<JsonArray>();
                if (arr.size() > 0) {
                    const char* existingMac = arr[0];
                    
                    if (existingMac != nullptr && strcmp(existingMac, macStr) == 0) {
                        macAlreadyExists = true;
                        Serial.printf("[DEBUG] MAC %s sudah ada di key %s.\n", macStr, key);
                        break;
                    }
                } else {
                    if (firstEmptySlot == -1) {
                        firstEmptySlot = i;
                    }
                }
            } else {
                if (firstEmptySlot == -1) {
                    firstEmptySlot = i;
                }
            }
        }

        if (macAlreadyExists) {
            Serial.println("[DEBUG] Tidak ada perubahan, MAC sudah terdaftar.");
        } else if (firstEmptySlot != -1) {
            Serial.printf("[DEBUG] Menambahkan MAC %s ke slot %d.\n", macStr, firstEmptySlot);
            
            char key[10];
            sprintf(key, "address%d", firstEmptySlot);

            JsonArray arr = doc[key].to<JsonArray>();
            arr.clear(); 
            arr.add(macStr);
            arr.add(device);

            if (firstEmptySlot == 0) {
                memcpy(configData.macAddress, mac, 6);
                strncpy(configData.nameDevice1, device, 11);
            } else {
                memcpy(configData.macAddress1, mac, 6);
                strncpy(configData.nameDevice2, device, 11);
            }

            // Tulis kembali ke file
            File fileWrite = SPIFFS.open("/config.json", FILE_WRITE);
            if (fileWrite) {
                if (serializeJson(doc, fileWrite) == 0) {
                    Serial.println("[ERROR] Gagal menulis JSON ke file config.json");
                } else {
                    Serial.println("[DEBUG] config.json berhasil diperbarui.");
                }
                fileWrite.close();
            } else {
                Serial.println("[ERROR] Gagal membuka file config.json untuk ditulis.");
            }
        } else {
            Serial.println("[DEBUG] Tidak ada slot kosong yang tersedia.");
        }

        xSemaphoreGive(_semaphore);
    } else {
        Serial.println("[ERROR] storage::writeMacAddress: Gagal mendapatkan mutex.");
    }
}

void storage::writeMode(const uint8_t *bufferMode, int count) {
    if (bufferMode == nullptr || count <= 0) {
        Serial.println("[ERROR] Parameter tidak valid di writeMode(uint8_t*).");
        return;
    }

    JsonDocument doc;
    // Baca file yang ada untuk mempertahankan data lain (misalnya 'volume')
    File fileRead = SPIFFS.open("/mode.json", FILE_READ);
    if (fileRead) {
        deserializeJson(doc, fileRead);
        fileRead.close();
    }

    JsonArray modes = doc["mode"].to<JsonArray>();
    modes.clear();

    size_t maxModes = sizeof(configData.modeArray) / sizeof(configData.modeArray[0]);
    size_t numToWrite = (count < maxModes) ? count : maxModes;

    for (size_t i = 0; i < numToWrite; i++) {
        modes.add(bufferMode[i]);
        configData.modeArray[i] = bufferMode[i];
    }

    File fileWrite = SPIFFS.open("/mode.json", FILE_WRITE);
    if (!fileWrite) {
        Serial.println("[ERROR] Gagal membuka file untuk menulis mode.");
        return;
    }

    if (serializeJson(doc, fileWrite) == 0) {
        Serial.println("[ERROR] Gagal menulis JSON ke file mode.json");
    } else {
        Serial.println("[DEBUG] Mode berhasil disimpan ke SPIFFS");
    }
    fileWrite.close();
}

void storage::writeMode(const int32_t* bufferMode, int count) {
    if (bufferMode == nullptr || count <= 0) {
        Serial.println("[ERROR] Parameter tidak valid di writeMode(int32_t*).");
        return;
    }

    JsonDocument doc;
    // Baca file yang ada untuk mempertahankan data lain (misalnya 'volume')
    File fileRead = SPIFFS.open("/mode.json", FILE_READ);
    if (fileRead) {
        // Abaikan error deserialisasi, lanjutkan dengan doc kosong jika perlu
        deserializeJson(doc, fileRead);
        fileRead.close();
    }

    JsonArray modes = doc["mode"].to<JsonArray>();
    modes.clear(); // Hapus mode lama

    size_t maxModes = sizeof(configData.modeArray) / sizeof(configData.modeArray[0]);
    size_t numToWrite = (count < maxModes) ? count : maxModes;

    for (size_t i = 0; i < numToWrite; i++) {
        modes.add(bufferMode[i]);
        configData.modeArray[i] = bufferMode[i];
    }

    File fileWrite = SPIFFS.open("/mode.json", FILE_WRITE);
    if (!fileWrite) {
        Serial.println("[ERROR] Gagal membuka file untuk menulis mode.");
        return;
    }

    if (serializeJson(doc, fileWrite) == 0) {
        Serial.println("[ERROR] Gagal menulis JSON ke file mode.json");
    } else {
        Serial.println("[DEBUG] Mode berhasil disimpan ke SPIFFS");
    }
    fileWrite.close();
}

bool storage::saveVolume(int data){
        JsonDocument doc;

        File fileWrite = SPIFFS.open("/mode.json", FILE_WRITE);
        if (!fileWrite) {
            // ESP_LOGE(TAG,"[ERROR] Gagal membuka file untuk menulis volume");
            return false;
        }

        doc["volume"] = data;
        configData.dataInt = data;
        serializeJson(doc, fileWrite);
        
        fileWrite.close();
        return true;
}

void storage::deleteAddress() {
    if (_semaphore != NULL && xSemaphoreTake(_semaphore, portMAX_DELAY) == pdTRUE) {
        File file = SPIFFS.open("/config.json", FILE_WRITE);
        if (!file) {
            Serial.println("[ERROR] failed to open file for writing");
            xSemaphoreGive(_semaphore);
            return;
        }
        
        JsonDocument doc; 

        // Menulis dokumen kosong akan menghapus semua konten
        if (serializeJson(doc, file) == 0) {
             Serial.println("[ERROR] Gagal menulis JSON kosong ke config.json");
        }
        file.close();
        memset(configData.macAddress, 0, 6);
        memset(configData.macAddress1, 0, 6);
        memset(configData.nameDevice1, 0, sizeof(configData.nameDevice1));
        memset(configData.nameDevice2, 0, sizeof(configData.nameDevice2));
        Serial.println("Addresses cleared successfully");
        xSemaphoreGive(_semaphore);
    } else {
        Serial.println("[ERROR] storage::deleteAddress: Gagal mendapatkan mutex.");
    }
}

bool storage::hapusAlamat(const char *deviceName) {
    if (_semaphore != NULL && xSemaphoreTake(_semaphore, portMAX_DELAY) == pdTRUE) {
        File fileRead = SPIFFS.open("/config.json", FILE_READ);
        if (!fileRead) {
            Serial.println("[ERROR] Gagal membuka config.json untuk dibaca.");
            xSemaphoreGive(_semaphore);
            return false;
        }

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, fileRead);
        fileRead.close();

        if (error) {
            Serial.printf("[ERROR] deserializeJson() gagal: %s\n", error.c_str());
            xSemaphoreGive(_semaphore);
            return false;
        }

        JsonObject root = doc.as<JsonObject>();
        const char* keyToRemove = nullptr;

        for (JsonPair kv : root) {
            JsonArray arr = kv.value().as<JsonArray>();
            
            if (!arr.isNull() && arr.size() >= 2) {
                const char* currentDeviceName = arr[1];

                if (currentDeviceName && strcmp(currentDeviceName, deviceName) == 0) {
                    
                    keyToRemove = kv.key().c_str();
                    Serial.printf("Perangkat '%s' ditemukan pada kunci '%s'. Akan dihapus.\n", deviceName, keyToRemove);
                    break;
                }
            }
        }

        if (keyToRemove != nullptr) {
            if (strcmp(keyToRemove, "address0") == 0) {
                memset(configData.macAddress, 0, sizeof(configData.macAddress));
                memset(configData.nameDevice1,0,sizeof(configData.nameDevice1));
            } else if (strcmp(keyToRemove, "address1") == 0) {
                memset(configData.macAddress1, 0, sizeof(configData.macAddress1));
                memset(configData.nameDevice2,0,sizeof(configData.nameDevice2));
            }
            
            root.remove(keyToRemove); // Hapus entry dari JSON

            File fileWrite = SPIFFS.open("/config.json", FILE_WRITE);
            if (!fileWrite) {
                Serial.println("[ERROR] Gagal membuka config.json untuk ditulis.");
                xSemaphoreGive(_semaphore);
                return false;
            }

            if (serializeJson(doc, fileWrite) == 0) {
                Serial.println("[ERROR] Gagal menulis ke file.");
            } else {
                Serial.println("[DEBUG] File config.json berhasil diperbarui.");
            }
            fileWrite.close();
            xSemaphoreGive(_semaphore);
            return true;
        } else {
            Serial.printf("[DEBUG] Perangkat dengan nama '%s' tidak ditemukan.\n", deviceName);
            xSemaphoreGive(_semaphore);
            return false;
        }
    } else {
        Serial.println("[ERROR] storage::hapusAlamat: Gagal mendapatkan mutex.");
        return false;
    }
}

uint8_t targetMac[6];
uint8_t *storage::getMac(int i){
    // static uint8_t targetMac[6];
    uint8_t *mac = nullptr;
    // Handle config.json
    File file = SPIFFS.open("/config.json", FILE_READ);
    if (file) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        char key[10];
        sprintf(key, "address%d", i);
        
        if (doc[key].is<JsonArray>()) {
            JsonArray arr = doc[key];
            
            if (!arr.isNull() && arr.size() >= 2) {
                const char* macStr = arr[0];
                
                if (macStr && strlen(macStr) == 17) { // Memastikan macStr valid
                    
                    for (int j = 0; j < 6; j++) {
                        targetMac[j] = strtoul(macStr + j*3, NULL, 16);
                    }
                    mac = targetMac;
                    
                }
            }
        }
    }
    return mac;
}

uint8_t *storage::getMac(){
    uint8_t *mac_ptr = nullptr;
    xSemaphoreTake(_semaphore, portMAX_DELAY);
    if (configData.macAddress[0] != 0)
    {
        mac_ptr = configData.macAddress;
    }
    xSemaphoreGive(_semaphore);
    return mac_ptr;
}

uint8_t *storage::getMac1(){
    uint8_t *mac_ptr = nullptr;
    xSemaphoreTake(_semaphore, portMAX_DELAY);
    if (configData.macAddress1[0] != 0)
    {
        mac_ptr = configData.macAddress1;
    }
 
    xSemaphoreGive(_semaphore);
    return mac_ptr;
}

int32_t *storage::readModeTones(){
    return configData.modeArray;
}

const char *storage::getDevice(int i){
    char* dev = nullptr;

    File file = SPIFFS.open("/config.json", FILE_READ);
    if (!file) {
        Serial.println("[ERROR] Gagal membuka file config.json");
        return nullptr;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.printf("[ERROR] Gagal parsing JSON: %s\n", error.c_str());
        return nullptr;
    }

    char key[10];
    sprintf(key, "address%d", i);
    Serial.printf("[DEBUG] Mencari kunci: %s\n", key);
    
    if (doc[key].is<JsonArray>()) {
        JsonArray arr = doc[key];
        
        Serial.printf("[DEBUG] Ukuran array: %d\n", arr.size());
        
        if (!arr.isNull() && arr.size() >= 2) {
            const char *devName = arr[1];
            Serial.printf("[DEBUG] Nama device ditemukan: %s\n", devName);

            dev = (i == 0) ? configData.nameDevice1 : configData.nameDevice2;
            strncpy(dev, devName, sizeof(configData.nameDevice1) - 1);
            dev[sizeof(configData.nameDevice1) - 1] = '\0';
            
            Serial.printf("[DEBUG] Nama device disalin: %s\n", dev);
        } else {
            Serial.println("[DEBUG] Array kosong atau tidak memiliki cukup elemen");
        }
    } else {
        Serial.println("[DEBUG] Kunci tidak ditemukan atau bukan array");
    }
    
    return dev;
}

char *storage::device1(){
    char *name_ptr = nullptr;
    if (_semaphore != NULL && xSemaphoreTake(_semaphore, portMAX_DELAY) == pdTRUE) {
        name_ptr = configData.nameDevice1;
        xSemaphoreGive(_semaphore);
    } else {
        Serial.println("[ERROR] device1: Gagal mendapatkan mutex, mengembalikan nullptr.");
    }
    return name_ptr;
}

char *storage::device2(){
    char *name_ptr = nullptr;
    if (_semaphore != NULL && xSemaphoreTake(_semaphore, portMAX_DELAY) == pdTRUE) {
        name_ptr = configData.nameDevice2;
        xSemaphoreGive(_semaphore);
    } else {
        Serial.println("[ERROR] device2: Gagal mendapatkan mutex, mengembalikan nullptr.");
    }
    return name_ptr;
}

int storage::getVolume(){
    return configData.dataInt;
}