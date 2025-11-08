#include "theme.h"

namespace ui_theme {

void init() {
    // Currently no dynamic resources required
}

void apply_small_text_recursive(lv_obj_t* root) {
    if (!root) return;
    lv_obj_set_style_text_font(root, &lv_font_montserrat_10, 0);
    uint32_t child_cnt = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(root, i);
        apply_small_text_recursive(child);
    }
}

void apply_window(lv_obj_t* window) {
    if (!window) return;
    // Rounded corners
    lv_obj_set_style_radius(window, 12, 0);
    // Background: near white efefef
    lv_obj_set_style_bg_color(window, lv_color_hex(0xEFEFEF), 0);
    lv_obj_set_style_bg_opa(window, LV_OPA_COVER, 0);
    // Border: pure white
    lv_obj_set_style_border_width(window, 2, 0);
    lv_obj_set_style_border_color(window, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_opa(window, LV_OPA_COVER, 0);
    // Soft drop shadow
    lv_obj_set_style_shadow_color(window, lv_color_hex(0x999999), 0);
    lv_obj_set_style_shadow_opa(window, LV_OPA_50, 0);
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
    lv_obj_set_size(btn, 120, 28);
    // Small font for button and its label children
    apply_small_text_recursive(btn);
    // Optional: tighter paddings for a compact look
    lv_obj_set_style_pad_ver(btn, 6, 0);
    lv_obj_set_style_pad_hor(btn, 12, 0);
}

} // namespace ui_theme