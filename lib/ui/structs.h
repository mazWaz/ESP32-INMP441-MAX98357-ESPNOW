#ifndef EEZ_LVGL_UI_STRUCTS_H
#define EEZ_LVGL_UI_STRUCTS_H

#include "eez-flow.h"


#if defined(EEZ_FOR_LVGL)

#include <stdint.h>
#include <stdbool.h>

#include "vars.h"

using namespace eez;

enum FlowStructures {
    FLOW_STRUCTURE_MODE_SIREN = 16384,
    FLOW_STRUCTURE_LOOPING = 16385
};

enum FlowArrayOfStructures {
    FLOW_ARRAY_OF_STRUCTURE_MODE_SIREN = 81920,
    FLOW_ARRAY_OF_STRUCTURE_LOOPING = 81921
};

enum modeSirenFlowStructureFields {
    FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_1 = 0,
    FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_2 = 1,
    FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_3 = 2,
    FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_4 = 3,
    FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_5 = 4,
    FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_6 = 5,
    FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_7 = 6,
    FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_8 = 7,
    FLOW_STRUCTURE_MODE_SIREN_NUM_FIELDS
};

enum loopingFlowStructureFields {
    FLOW_STRUCTURE_LOOPING_FIELD_SWITCH_LOOP = 0,
    FLOW_STRUCTURE_LOOPING_NUM_FIELDS
};

struct modeSirenValue {
    Value value;
    
    modeSirenValue() {
        value = Value::makeArrayRef(FLOW_STRUCTURE_MODE_SIREN_NUM_FIELDS, FLOW_STRUCTURE_MODE_SIREN, 0);
    }
    
    modeSirenValue(Value value) : value(value) {}
    
    operator Value() const { return value; }
    
    operator bool() const { return value.isArray(); }
    
    int mode_1() {
        return value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_1].getInt();
    }
    void mode_1(int mode_1) {
        value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_1] = IntegerValue(mode_1);
    }
    
    int mode_2() {
        return value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_2].getInt();
    }
    void mode_2(int mode_2) {
        value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_2] = IntegerValue(mode_2);
    }
    
    int mode_3() {
        return value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_3].getInt();
    }
    void mode_3(int mode_3) {
        value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_3] = IntegerValue(mode_3);
    }
    
    int mode_4() {
        return value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_4].getInt();
    }
    void mode_4(int mode_4) {
        value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_4] = IntegerValue(mode_4);
    }
    
    int mode_5() {
        return value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_5].getInt();
    }
    void mode_5(int mode_5) {
        value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_5] = IntegerValue(mode_5);
    }
    
    int mode_6() {
        return value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_6].getInt();
    }
    void mode_6(int mode_6) {
        value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_6] = IntegerValue(mode_6);
    }
    
    int mode_7() {
        return value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_7].getInt();
    }
    void mode_7(int mode_7) {
        value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_7] = IntegerValue(mode_7);
    }
    
    int mode_8() {
        return value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_8].getInt();
    }
    void mode_8(int mode_8) {
        value.getArray()->values[FLOW_STRUCTURE_MODE_SIREN_FIELD_MODE_8] = IntegerValue(mode_8);
    }
};

typedef ArrayOf<modeSirenValue, FLOW_ARRAY_OF_STRUCTURE_MODE_SIREN> ArrayOfmodeSirenValue;
struct loopingValue {
    Value value;
    
    loopingValue() {
        value = Value::makeArrayRef(FLOW_STRUCTURE_LOOPING_NUM_FIELDS, FLOW_STRUCTURE_LOOPING, 0);
    }
    
    loopingValue(Value value) : value(value) {}
    
    operator Value() const { return value; }
    
    operator bool() const { return value.isArray(); }
    
    bool switch_loop() {
        return value.getArray()->values[FLOW_STRUCTURE_LOOPING_FIELD_SWITCH_LOOP].getBoolean();
    }
    void switch_loop(bool switch_loop) {
        value.getArray()->values[FLOW_STRUCTURE_LOOPING_FIELD_SWITCH_LOOP] = BooleanValue(switch_loop);
    }
};

typedef ArrayOf<loopingValue, FLOW_ARRAY_OF_STRUCTURE_LOOPING> ArrayOfloopingValue;


#endif

#endif /*EEZ_LVGL_UI_STRUCTS_H*/
