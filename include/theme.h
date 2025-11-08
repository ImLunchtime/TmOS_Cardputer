#pragma once
#include <lvgl.h>

namespace ui_theme {
    // Initialize theme resources if needed
    void init();

    // Apply themed styles to a window container
    void apply_window(lv_obj_t* window);

    // Apply themed styles to a generic button (pill + small)
    void apply_button(lv_obj_t* btn);

    // Set small text font on this object and its descendants
    void apply_small_text_recursive(lv_obj_t* root);

    // Create a wallpaper behind all windows (currently solid black)
    lv_obj_t* create_wallpaper();
}