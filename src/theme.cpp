#include "theme.h"
#include <lvgl.h>

// Declare custom font defined in src/simhei_12.c
LV_FONT_DECLARE(simhei_12);

namespace ui_theme {

// Keep a single system-wide font pointer
static const lv_font_t* k_sys_font = &simhei_12;

void init() {
    // Initialize default LVGL theme with the custom font for all widgets
    lv_disp_t* disp = lv_disp_get_default();
    if (disp) {
        lv_theme_t* th = lv_theme_default_init(
            disp,
            lv_palette_main(LV_PALETTE_BLUE),
            lv_palette_main(LV_PALETTE_GREY),
            true,                  // dark mode
            k_sys_font             // custom system font
        );
        lv_disp_set_theme(disp, th);
    }
}

const lv_font_t* get_system_font() {
    return k_sys_font;
}

void apply_small_text_recursive(lv_obj_t* root) {
    if (!root) return;
    lv_obj_set_style_text_font(root, k_sys_font, 0);
    uint32_t child_cnt = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(root, i);
        apply_small_text_recursive(child);
    }
}

void apply_window(lv_obj_t* window) {
    if (!window) return;
    // Rounded corners
    lv_obj_set_style_radius(window, 0, 0);
    // Dark background
    lv_obj_set_style_bg_color(window, lv_color_hex(0x1E1E1E), 0);
    lv_obj_set_style_bg_opa(window, LV_OPA_COVER, 0);
    // Border: subtle light border on dark
    lv_obj_set_style_border_width(window, 2, 0);
    lv_obj_set_style_border_color(window, lv_color_hex(0xBBBBBB), 0);
    lv_obj_set_style_border_opa(window, LV_OPA_60, 0);
    // Soft drop shadow
    lv_obj_set_style_shadow_color(window, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(window, LV_OPA_20, 0);
    lv_obj_set_style_shadow_width(window, 12, 0);
    lv_obj_set_style_shadow_spread(window, 0, 0);
    lv_obj_set_style_shadow_ofs_x(window, 4, 0);
    lv_obj_set_style_shadow_ofs_y(window, 4, 0);
    // Padding
    lv_obj_set_style_pad_all(window, 6, 0);
    lv_obj_set_style_pad_row(window, 8, 0);
    // Small text for all descendants
    apply_small_text_recursive(window);
}

void apply_button(lv_obj_t* btn) {
    if (!btn) return;
    // Ensure focusable via keypad
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    // Pill-shaped radius
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    // Smaller size
    lv_obj_set_size(btn, 120, 20);
    // Small font for button and its label children
    apply_small_text_recursive(btn);
    // Optional: tighter paddings for a compact look
    lv_obj_set_style_pad_ver(btn, 6, 0);
    lv_obj_set_style_pad_hor(btn, 12, 0);
    // Dark button base with light text
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(btn, lv_color_hex(0xEEEEEE), 0);
}

lv_obj_t* create_wallpaper() {
    lv_obj_t* wp = lv_obj_create(lv_scr_act());
    // Remove default styles to act as simple colored layer
    lv_obj_remove_style_all(wp);
    lv_obj_set_size(wp, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_set_style_bg_color(wp, lv_color_hex(0x111111), 0);
    lv_obj_set_style_bg_opa(wp, LV_OPA_COVER, 0);
    lv_obj_clear_flag(wp, LV_OBJ_FLAG_SCROLLABLE);
    // Send wallpaper to background
    lv_obj_move_background(wp);
    return wp;
}

void apply_list_menu(lv_obj_t* list) {
    if (!list) return;
    // Keep default LVGL look; reduce spacing between child items.
    // In LVGL v8, list items are separate button objects; style them individually.
    lv_obj_set_style_pad_row(list, 0, 0);

    // Apply compact style to existing children
    uint32_t child_cnt = lv_obj_get_child_cnt(list);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(list, i);
        // Only style buttons (list items); other children (e.g., scrollbar) are skipped.
        apply_small_text_recursive(child);
        lv_obj_set_style_pad_top(child, 2, 0);
        lv_obj_set_style_pad_bottom(child, 2, 0);
        lv_obj_set_style_pad_left(child, 6, 0);
        lv_obj_set_style_pad_right(child, 6, 0);
        lv_obj_set_style_min_height(child, 16, 0);
    }
}

void apply_list_menu_item(lv_obj_t* item) {
    if (!item) return;
    apply_small_text_recursive(item);
    lv_obj_set_style_pad_top(item, 2, 0);
    lv_obj_set_style_pad_bottom(item, 2, 0);
    lv_obj_set_style_pad_left(item, 6, 0);
    lv_obj_set_style_pad_right(item, 6, 0);
    lv_obj_set_style_min_height(item, 16, 0);
}

} // namespace ui_theme