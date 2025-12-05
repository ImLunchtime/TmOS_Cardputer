#include "theme.h"
#include <lvgl.h>
#include <unordered_map>

// Declare custom font defined in src/simhei_12.c
LV_FONT_DECLARE(simhei_12);
// extern const lv_img_dsc_t bg2; // Wallpaper image descriptor from src/bg2.c

namespace ui_theme {

// Keep a single system-wide font pointer
static const lv_font_t* k_sys_font = &simhei_12;

// Track per-window theme associations
static std::unordered_map<const lv_obj_t*, ThemeId> s_window_themes;

void init() {
    // Initialize default LVGL theme with the custom font for all widgets
    lv_disp_t* disp = lv_disp_get_default();
    if (disp) {
        lv_theme_t* th = lv_theme_default_init(
            disp,
            lv_palette_main(LV_PALETTE_BLUE),
            lv_palette_main(LV_PALETTE_GREY),
            false,                 // light mode (LVGL defaults)
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

ThemeId get_theme_for(lv_obj_t* obj) {
    if (!obj) return ThemeId::Dark;
    // Walk up the parent chain to find the associated window theme
    const lv_obj_t* cur = obj;
    while (cur) {
        auto it = s_window_themes.find(cur);
        if (it != s_window_themes.end()) return it->second;
        cur = lv_obj_get_parent(const_cast<lv_obj_t*>(cur));
    }
    return ThemeId::Dark; // default
}

void clear_window_theme(lv_obj_t* window) {
    if (!window) return;
    s_window_themes.erase(window);
}

void apply_window(lv_obj_t* window, ThemeId theme) {
    if (!window) return;
    // Associate this window with a theme
    s_window_themes[window] = theme;

    if (theme == ThemeId::Dark) {
        // Rounded corners
        lv_obj_set_style_radius(window, 0, 0);
        // Dark background
        lv_obj_set_style_bg_color(window, lv_color_hex(0x1E1E1E), 0);
        lv_obj_set_style_bg_opa(window, LV_OPA_COVER, 0);
        // Border: subtle light border on dark
        lv_obj_set_style_border_width(window, 1, 0);
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
    } else {
        // Light theme: leave LVGL default styles intact (no overrides)
        // Intentionally avoid setting colors, borders, radius, paddings or fonts.
    }
}

void apply_window(lv_obj_t* window) {
    // Default to dark theme for backward compatibility
    apply_window(window, ThemeId::Dark);
}

void apply_button(lv_obj_t* btn) {
    if (!btn) return;
    // Ensure focusable via keypad
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    ThemeId th = get_theme_for(btn);
    if (th == ThemeId::Dark) {
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
    } else {
        // Light theme: preserve LVGL defaults; avoid custom styling.
    }
}

lv_obj_t* create_wallpaper() {
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x808080), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    return scr;
}

void apply_list_menu(lv_obj_t* list) {
    if (!list) return;
    ThemeId th = get_theme_for(list);
    if (th == ThemeId::Dark) {
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
    } else {
        // Light theme: no compact overrides; use LVGL defaults except item height.
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
}

void apply_list_menu_item(lv_obj_t* item) {
    if (!item) return;
    ThemeId th = get_theme_for(item);
    if (th == ThemeId::Dark) {
        apply_small_text_recursive(item);
        lv_obj_set_style_pad_top(item, 2, 0);
        lv_obj_set_style_pad_bottom(item, 2, 0);
        lv_obj_set_style_pad_left(item, 6, 0);
        lv_obj_set_style_pad_right(item, 6, 0);
        lv_obj_set_style_min_height(item, 16, 0);
    } else {
        // Light theme
        apply_small_text_recursive(item);
        lv_obj_set_style_pad_top(item, 2, 0);
        lv_obj_set_style_pad_bottom(item, 2, 0);
        lv_obj_set_style_pad_left(item, 6, 0);
        lv_obj_set_style_pad_right(item, 6, 0);
        lv_obj_set_style_min_height(item, 16, 0);
    }
}

} // namespace ui_theme
