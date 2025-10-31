#include <Arduino.h>
#ifdef RECEIVER
#include "receiver/receiverTask.h"
receiverTask *receiver;

void appCore0(void *param);
void appCore1(void *param);
#endif
#ifdef TRANSMITTER 
#include "transmitter/transmitterTask.h"
transmitterTask * transmitter;

void appCore0(void *param);
void appCore1(void *param);

void applicationTask(void *param);
#endif
#ifdef DISP
#include "display/displayTask.h"
displayTask *lcd;

void applicationTask(void *param);
void appCore0(void *param);
void appCore1(void *param);
void appCore1UpdateData(void *param);
#endif




void setup(){
    Serial.begin(115200);
#ifdef RECEIVER
    receiver = new receiverTask();

    receiver->begin();

    TaskHandle_t handleCore0;
    xTaskCreatePinnedToCore(
        appCore0,
        "appCore0",
        10000,
        NULL,
        1,
        &handleCore0,
        0
    );

    TaskHandle_t handleCore1;
    xTaskCreatePinnedToCore(
        appCore1,
        "appCore1",
        10000,
        NULL,
        1,
        &handleCore1,
        1
    );
#endif
#ifdef TRANSMITTER
    transmitter = new transmitterTask();
    transmitter->begin();

    TaskHandle_t handleCore0;
    xTaskCreatePinnedToCore(
        appCore0,
        "appCore0",
        4096,
        NULL,
        1,
        &handleCore0,
        0
    );

    TaskHandle_t handleCore1;
    xTaskCreatePinnedToCore(
        appCore1,
        "appCore1",
        4096,
        NULL,
        1,
        &handleCore1,
        1
    );
#endif
#ifdef DISP
    lcd = new displayTask();
    if(lcd->begin()){
        Serial.println("Display Task Initialized Successfully");
    }

    TaskHandle_t handleCore0;
    xTaskCreatePinnedToCore(
        appCore0,
        "appCore0",
        16384,
        NULL,
        1,
        &handleCore0,
        0
    );

    lcd->lvglInit();

    TaskHandle_t handleCore1;
    xTaskCreatePinnedToCore(
        appCore1,
        "appCore1",
        8192,
        NULL,
        2,
        &handleCore1,
        1
    );

    TaskHandle_t handleCore1UpdateData;
    xTaskCreatePinnedToCore(
        appCore1UpdateData,
        "appCore1UpdateData",
        8192,
        NULL,
        1,
        &handleCore1UpdateData,
        1
    );

#endif
}

void loop(){
    vTaskDelay(pdMS_TO_TICKS(1000));
}


#ifdef RECEIVER

void appCore0(void *param){
    receiver->communication();
}

void appCore1(void *param){
    receiver->processData();
}

#endif


#ifdef RECEIVER
#endif

#ifdef TRANSMITTER
void appCore0(void *param){
    transmitter->communication();
}

void appCore1(void *param){
    transmitter->trasnmitData();
}
#endif

#ifdef DISP
void appCore0(void *param){
    lcd->initializeProcessData();
    while (true)
    {
        lcd->processData();
        vTaskDelay(5);
    }
}

void appCore1(void *param){
    while (true)
    {
        
        lcd->loop();
        vTaskDelay(10);
    }
}

void appCore1UpdateData(void *param){
    for (;;)
    {
        if (lcd) {
            lcd->tick();
        }
        vTaskDelay(10);
    }
}
#endif

