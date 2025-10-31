#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *about;
    lv_obj_t *setting_page;
    lv_obj_t *mode_sirine;
    lv_obj_t *about_back_2;
    lv_obj_t *about_button;
    lv_obj_t *about_button_2;
    lv_obj_t *binding_button_4;
    lv_obj_t *button_exit;
    lv_obj_t *mode1_1;
    lv_obj_t *mode2_1;
    lv_obj_t *mode3_1;
    lv_obj_t *mode4_1;
    lv_obj_t *mode5_1;
    lv_obj_t *mode6_1;
    lv_obj_t *mode7_1;
    lv_obj_t *mode8_1;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj10;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *obj14;
    lv_obj_t *obj15;
    lv_obj_t *obj16;
    lv_obj_t *obj17;
    lv_obj_t *obj18;
    lv_obj_t *obj19;
    lv_obj_t *obj2;
    lv_obj_t *obj20;
    lv_obj_t *obj20__approve;
    lv_obj_t *obj20__button_device1;
    lv_obj_t *obj20__button_piring_1;
    lv_obj_t *obj20__button_piring_2;
    lv_obj_t *obj20__close;
    lv_obj_t *obj20__delete_address;
    lv_obj_t *obj20__obj21;
    lv_obj_t *obj20__obj22;
    lv_obj_t *obj20__obj23;
    lv_obj_t *obj20__obj24;
    lv_obj_t *obj20__obj25;
    lv_obj_t *obj20__obj26;
    lv_obj_t *obj20__obj27;
    lv_obj_t *obj20__obj28;
    lv_obj_t *obj29;
    lv_obj_t *obj3;
    lv_obj_t *obj30;
    lv_obj_t *obj31;
    lv_obj_t *obj31__button_device1_1;
    lv_obj_t *obj31__button_play;
    lv_obj_t *obj31__close_2;
    lv_obj_t *obj31__obj32;
    lv_obj_t *obj31__obj33;
    lv_obj_t *obj31__obj34;
    lv_obj_t *obj31__play_button;
    lv_obj_t *obj31__save_button;
    lv_obj_t *obj31__switch_siren;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *obj9;
    lv_obj_t *setting_button_4;
    lv_obj_t *switch_screen;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_ABOUT = 2,
    SCREEN_ID_SETTING_PAGE = 3,
    SCREEN_ID_MODE_SIRINE = 4,
    SCREEN_ID_BOX_MESSAGE = 5,
    SCREEN_ID_BOX_MESSAGE_SIRINE = 6,
};

void create_screen_main();
void tick_screen_main();

void create_screen_about();
void tick_screen_about();

void create_screen_setting_page();
void tick_screen_setting_page();

void create_screen_mode_sirine();
void tick_screen_mode_sirine();

void create_user_widget_box_message(lv_obj_t *parent_obj, void *flowState, int startWidgetIndex);
void tick_user_widget_box_message(void *flowState, int startWidgetIndex);

void create_user_widget_box_message_sirine(lv_obj_t *parent_obj, void *flowState, int startWidgetIndex);
void tick_user_widget_box_message_sirine(void *flowState, int startWidgetIndex);

void create_screens();
void tick_screen(int screen_index);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/