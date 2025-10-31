#pragma once

#include <lvgl.h>
#include "lovyanGfxSetup.h"

// Deklarasi extern untuk tft
extern LGFX tft;

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf);
#endif

class Screen {
private:
    static constexpr int TFT_HOR_RES = SCREEN_WIDTH;
    static constexpr int TFT_VER_RES = SCREEN_HEIGHT;
    static constexpr int DRAW_BUF_SIZE = (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8));
    
    lv_display_t* display;
    lv_indev_t* touchpad;
    lv_color_t* displayBuffer;

    // Deklarasi fungsi callback statis
    static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
    static void touchpad_read_cb(lv_indev_t *indev, lv_indev_data_t *data);

    // Deklarasi metode handler
    void handleDisplayFlush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
    void handleTouchpadRead(lv_indev_t *indev, lv_indev_data_t *data);

public:
    Screen();
    ~Screen();
    bool begin();
    void lvHandler();
};
