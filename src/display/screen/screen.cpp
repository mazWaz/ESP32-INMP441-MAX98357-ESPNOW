#include "screen.h"

// Definisi variabel global tft
LGFX tft;

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf)
{
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

// Implementasi fungsi callback statis
void Screen::disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    Screen* screen = (Screen*)lv_display_get_user_data(disp);
    screen->handleDisplayFlush(disp, area, px_map);
}

void Screen::touchpad_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
    Screen* screen = (Screen*)lv_indev_get_user_data(indev);
    screen->handleTouchpadRead(indev, data);
}

// Implementasi metode-metode Screen
void Screen::handleDisplayFlush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    if (area->x1 >= SCREEN_WIDTH || area->y1 >= SCREEN_HEIGHT) {
        lv_disp_flush_ready(disp);
        return;
    }

    if (area->x2 >= SCREEN_WIDTH) {
        w = SCREEN_WIDTH - area->x1;
    }

    if (area->y2 >= SCREEN_HEIGHT) {
        h = SCREEN_HEIGHT - area->y1;
    }

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.writePixels((lgfx::rgb565_t *)px_map, w * h);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

void Screen::handleTouchpadRead(lv_indev_t *indev, lv_indev_data_t *data) {
    uint16_t touchX, touchY;
    bool touched = tft.getTouch(&touchX, &touchY);

    if (!touched) {
        data->state = LV_INDEV_STATE_RELEASED;
    } else {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

Screen::Screen() : display(nullptr), touchpad(nullptr), displayBuffer(nullptr) {
}

Screen::~Screen() {
    if (displayBuffer) {
        heap_caps_free(displayBuffer);
    }
}

bool Screen::begin() {
    Serial.println("Initializing display...");
    
    // Initialize display
    if (!tft.begin()) {
        Serial.println("Failed to initialize display");
        return false;
    }
    
    tft.setRotation(0);
    tft.setBrightness(255);
    Serial.println("Display initialized successfully");

    // Initialize LVGL
    lv_init();
    lv_tick_set_cb((lv_tick_get_cb_t)millis);
    Serial.println("LVGL initialized");

    // Allocate display buffer
    displayBuffer = (lv_color_t*)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!displayBuffer) {
        Serial.println("Failed to allocate display buffer");
        return false;
    }
    Serial.printf("Display buffer allocated: %d bytes\n", DRAW_BUF_SIZE);

    // Setup display
    display = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    if (!display) {
        Serial.println("Failed to create LVGL display");
        return false;
    }
    
    lv_display_set_user_data(display, this);
    lv_display_set_render_mode(display, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(display, disp_flush_cb);
    lv_display_set_buffers(display, displayBuffer, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Setup touchpad
    touchpad = lv_indev_create();
    if (!touchpad) {
        Serial.println("Failed to create LVGL input device");
        return false;
    }
    
    lv_indev_set_user_data(touchpad, this);
    lv_indev_set_type(touchpad, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touchpad, touchpad_read_cb);

    Serial.println("Screen initialization completed successfully");
    return true;
}

void Screen::lvHandler() {
    lv_timer_handler();
} 