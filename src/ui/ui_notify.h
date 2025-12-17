#pragma once
#include <lvgl.h>
#include <stdint.h>
namespace ui_notify {
    void showText(const char* text, uint32_t duration_ms = 2000);
    void showSymbol(const char* symbol, const char* text, uint32_t duration_ms = 2000);
    void showImage(const lv_img_dsc_t* img, const char* text, uint32_t duration_ms = 2000);
    void hide();
}

