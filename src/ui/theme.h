#pragma once
#include <lvgl.h>

namespace ui_theme {
    // Available themes for per-app selection
    enum class ThemeId { Dark, Light };

    // Initialize theme resources if needed
    void init();

    // Access the system font set by theme initialization
    const lv_font_t* get_system_font();

    // Apply themed styles to a window container
    void apply_window(lv_obj_t* window);
    void apply_window(lv_obj_t* window, ThemeId theme);

    // Query theme associated with an object (walks up to window root)
    ThemeId get_theme_for(lv_obj_t* obj);

    // Clear theme association for a window (called on window destruction)
    void clear_window_theme(lv_obj_t* window);

    // Apply themed styles to a generic button (pill + small)
    void apply_button(lv_obj_t* btn);

    // Apply compact item style to list menus (16px item height)
    void apply_list_menu(lv_obj_t* list);

    // Apply compact 16px style to a single list menu item (button)
    void apply_list_menu_item(lv_obj_t* item);

    // Set small text font on this object and its descendants
    void apply_small_text_recursive(lv_obj_t* root);

    // Create a wallpaper behind all windows (currently solid black)
    lv_obj_t* create_wallpaper();
}