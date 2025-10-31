#include "eez-flow.h"
#include "vars.h"

using namespace eez;

int32_t getVol(){
    return flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_VOLUME).getInt32();
}

const char *getAddress(){
    return flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_ADDRESS).getString();
}

const char *getDevice1(){
    return flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_DEVICE1).getString();
}

const char *getAddress2(){
    return flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_ADDRESS2).getString();
}

const char *getDevice2(){
    return flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_DEVICE2).getString();
}

const char *getChoiseDevice(){
    return flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_NAME_DEVICE).getString();
}

bool switchLoop(){
    return flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_LOOP).getBoolean();
}

int32_t getSirenTone(){
    return flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_SIREN_TONE).getInt();
}

int32_t *getModes(){
    static int32_t modesBuffer[9];
    auto value = flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_MODES);
    auto array = value.getArray();
    if (!array) {
        for (int i = 0; i < 9; ++i) modesBuffer[i] = 0;
        return modesBuffer;
    }
    uint32_t count = array->arraySize;
    if (count > 9) count = 9;
    for (uint32_t i = 0; i < count; ++i) {
        modesBuffer[i] = array->values[i].getInt32();
    }
    for (uint32_t i = count; i < 9; ++i) {
        modesBuffer[i] = 0;
    }
    return modesBuffer;
}

int getButton(){
    return flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_STATE_BUTTON).getInt();
}

int currentState(){
    return flow::getGlobalVariable(FLOW_GLOBAL_VARIABLE_CURRENT_ACTION).getInt();
}

void setCurrentState(int state){
    flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_CURRENT_ACTION, state);
}

void setPlayButton(int button){
    flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_STATE_BUTTON,button);
}


void setHidden(int button, bool hidden){
    flow::setGlobalVariable(button, hidden);
}

void setAddress(const char *data){
    flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_ADDRESS, data);
}

void setDevice1(const char *data){
    flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_DEVICE1, data);
}

void setAddress2(const char *data){
    flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_ADDRESS2, data);
}

void setDevice2(const char *data){
    flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_DEVICE2, data);
}

void setVol(int32_t vol){
    flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_VOLUME, vol);
}

void setModes(int32_t *arr, size_t len){
    if (arr == nullptr || len < 9) return;
    eez::ArrayOfInteger flowArray(9);
    for (int i = 0; i < 9; ++i) {
        flowArray.at(i, arr[i]);
    }
    flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_MODES, flowArray.value);
}