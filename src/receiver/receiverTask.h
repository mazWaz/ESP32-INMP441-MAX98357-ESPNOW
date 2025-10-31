#include <Arduino.h>
#include <driver/gpio.h>

#include "commEspNow.h"
#include "speaker.h"
#include "sirine.h"
#include "buffer.h"
#include "storage.h"
#include "button/button.h"

#define BYTE_RATE 128

#define ESP_NOW_WIFI_CHANNEL 1

#define TRANSPORT_HEADER_SIZE 0
extern uint8_t transportHeader[TRANSPORT_HEADER_SIZE];

class receiverTask{
private:
    commEspNow *mCommunication;
    storage *mMemory; 
    speaker *mOutput;
    sirine *mSirine;
    Buffer *outBuffer;
    button *mButton; 

    int16_t *samples = nullptr;
    int16_t *sine_buffer = nullptr;
    
    int siren = 0;
    int mode =0;
    bool isLoop = false;
    int clickTimeout =300;
    int clickTime = 0;
    int clickCount;
    int SirenModeClick;
    int volumeAudio = 0;

    SemaphoreHandle_t _taskMutex; 

    unsigned long currentTime;
    void clearSample();
public:
    receiverTask();
    ~receiverTask();
    void begin();
    void communication();
    void processData();
};
