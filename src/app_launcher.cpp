#include "app_launcher.h"
#include "app_music.h"
#include "app_theme_center.h"
#include "app_settings.h"
#include "theme.h"

// Simple animation helpers
static void anim_set_x(void* obj, int32_t v) { lv_obj_set_x(static_cast<lv_obj_t*>(obj), v); }
static void anim_set_y(void* obj, int32_t v) { lv_obj_set_y(static_cast<lv_obj_t*>(obj), v); }
static void anim_set_w(void* obj, int32_t v) { lv_obj_set_width(static_cast<lv_obj_t*>(obj), v); }
static void anim_set_h(void* obj, int32_t v) { lv_obj_set_height(static_cast<lv_obj_t*>(obj), v); }

static void animate_obj_xy(lv_obj_t* obj, int32_t x, int32_t y, uint32_t time) {
    lv_anim_t ax; lv_anim_init(&ax);
    lv_anim_set_var(&ax, obj);
    lv_anim_set_values(&ax, lv_obj_get_x(obj), x);
    lv_anim_set_time(&ax, time);
    lv_anim_set_exec_cb(&ax, anim_set_x);
    lv_anim_start(&ax);

    lv_anim_t ay; lv_anim_init(&ay);
    lv_anim_set_var(&ay, obj);
    lv_anim_set_values(&ay, lv_obj_get_y(obj), y);
    lv_anim_set_time(&ay, time);
    lv_anim_set_exec_cb(&ay, anim_set_y);
    lv_anim_start(&ay);
}

static void animate_obj_wh(lv_obj_t* obj, int32_t w, int32_t h, uint32_t time) {
    lv_anim_t aw; lv_anim_init(&aw);
    lv_anim_set_var(&aw, obj);
    lv_anim_set_values(&aw, lv_obj_get_width(obj), w);
    lv_anim_set_time(&aw, time);
    lv_anim_set_exec_cb(&aw, anim_set_w);
    lv_anim_start(&aw);

    lv_anim_t ah; lv_anim_init(&ah);
    lv_anim_set_var(&ah, obj);
    lv_anim_set_values(&ah, lv_obj_get_height(obj), h);
    lv_anim_set_time(&ah, time);
    lv_anim_set_exec_cb(&ah, anim_set_h);
    lv_anim_start(&ah);
}
// Event handlers for app buttons
static void on_music_btn_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        launcher->launchMusic();
    } else if (lv_event_get_code(e) == LV_EVENT_FOCUSED) {
        launcher->setFocusedIndex(0);
        launcher->layoutIcons();
    }
}

static void on_theme_center_btn_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        launcher->launchThemeCenter();
    } else if (lv_event_get_code(e) == LV_EVENT_FOCUSED) {
        launcher->setFocusedIndex(1);
        launcher->layoutIcons();
    }
}

static void on_settings_btn_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        launcher->launchSettings();
    } else if (lv_event_get_code(e) == LV_EVENT_FOCUSED) {
        launcher->setFocusedIndex(2);
        launcher->layoutIcons();
    }
}

void AppLauncher::onOpen(lv_obj_t* window_root) {
    root_ = window_root;

    // Main container: no scroll, no extra padding
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_set_style_pad_row(root_, 0, 0);
    lv_obj_set_scroll_dir(root_, LV_DIR_NONE);

    // Grid container for app icons
    grid_container_ = lv_obj_create(root_);
    // Fill entire window and manually position icons (no flex layout)
    lv_obj_set_size(grid_container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(grid_container_, 0, 0);
    lv_obj_set_style_bg_opa(grid_container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(grid_container_, 0, 0);
    lv_obj_set_scroll_dir(grid_container_, LV_DIR_NONE);

    // Create app buttons in grid
    const int btn_width = 40;
    const int btn_height = 40;

    // Music App Button
    btn_music_ = lv_btn_create(grid_container_);
    lv_obj_set_size(btn_music_, btn_width, btn_height);
    lv_obj_set_style_radius(btn_music_, 0, 0); // Square button
    lv_obj_set_style_bg_color(btn_music_, lv_color_hex(0x2E7D32), 0); // Green background
    lv_obj_set_style_bg_opa(btn_music_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_music_, 0, 0);
    lv_obj_set_style_shadow_width(btn_music_, 0, 0);
    
    // Music icon only
    lv_obj_t* music_icon = lv_label_create(btn_music_);
    lv_label_set_text(music_icon, LV_SYMBOL_AUDIO);
    lv_obj_set_style_text_font(music_icon, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(music_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(music_icon);

    lv_obj_add_event_cb(btn_music_, on_music_btn_event, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(btn_music_, on_music_btn_event, LV_EVENT_FOCUSED, this);

    // Theme Center Button
    btn_theme_center_ = lv_btn_create(grid_container_);
    lv_obj_set_size(btn_theme_center_, btn_width, btn_height);
    lv_obj_set_style_radius(btn_theme_center_, 0, 0); // Square button
    lv_obj_set_style_bg_color(btn_theme_center_, lv_color_hex(0xEF6C00), 0); // Orange background
    lv_obj_set_style_bg_opa(btn_theme_center_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_theme_center_, 0, 0);
    lv_obj_set_style_shadow_width(btn_theme_center_, 0, 0);
    
    // Theme icon only
    lv_obj_t* theme_icon = lv_label_create(btn_theme_center_);
    lv_label_set_text(theme_icon, LV_SYMBOL_EDIT);
    lv_obj_set_style_text_font(theme_icon, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(theme_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(theme_icon);

    lv_obj_add_event_cb(btn_theme_center_, on_theme_center_btn_event, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(btn_theme_center_, on_theme_center_btn_event, LV_EVENT_FOCUSED, this);

    // Settings Button
    btn_settings_ = lv_btn_create(grid_container_);
    lv_obj_set_size(btn_settings_, btn_width, btn_height);
    lv_obj_set_style_radius(btn_settings_, 0, 0); // Square button
    lv_obj_set_style_bg_color(btn_settings_, lv_color_hex(0x1565C0), 0); // Blue background
    lv_obj_set_style_bg_opa(btn_settings_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_settings_, 0, 0);
    lv_obj_set_style_shadow_width(btn_settings_, 0, 0);
    
    // Settings icon only
    lv_obj_t* settings_icon = lv_label_create(btn_settings_);
    lv_label_set_text(settings_icon, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_font(settings_icon, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(settings_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(settings_icon);

    lv_obj_add_event_cb(btn_settings_, on_settings_btn_event, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(btn_settings_, on_settings_btn_event, LV_EVENT_FOCUSED, this);

    // Focused app name label (initially empty)
    focused_label_ = lv_label_create(grid_container_);
    lv_label_set_text(focused_label_, "");
    lv_obj_set_style_text_font(focused_label_, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(focused_label_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(focused_label_, LV_OPA_TRANSP, 0);
}

void AppLauncher::onTick() {
    if (!layout_initialized_) {
        // Center the default icon and set focus to it
        layoutIcons();
        lv_group_focus_obj(btn_theme_center_);
        layout_initialized_ = true;
    }
}

void AppLauncher::setFocusedIndex(int idx) {
    focused_index_ = idx;
}

void AppLauncher::layoutIcons() {
    if (!grid_container_) return;
    // Ensure layout sizes are up-to-date
    lv_obj_update_layout(grid_container_);
    int w = lv_obj_get_width(grid_container_);
    int h = lv_obj_get_height(grid_container_);
    if (w <= 0 || h <= 0) { w = 240; h = 135; }
    // Target sizes
    const int focused_size = 52;
    const int side_size = 36;
    const int spacing = 12;

    // Center positions depend on sizes
    int center_x = (w - focused_size) / 2;
    int center_y = (h - focused_size) / 2 - 10;
    int left_x = center_x - spacing - side_size;
    int left_y = (h - side_size) / 2 - 10;
    int right_x = center_x + focused_size + spacing;
    int right_y = (h - side_size) / 2 - 10;

    int n = 3;
    int left_idx = (focused_index_ + n - 1) % n;
    int right_idx = (focused_index_ + 1) % n;

    // Apply positions and sizes, animated after initialization
    uint32_t time = layout_initialized_ ? 180 : 0;
    // Left
    switch (left_idx) {
        case 0: if (time) { animate_obj_xy(btn_music_, left_x, left_y, time); animate_obj_wh(btn_music_, side_size, side_size, time); } else { lv_obj_set_pos(btn_music_, left_x, left_y); lv_obj_set_size(btn_music_, side_size, side_size);} break;
        case 1: if (time) { animate_obj_xy(btn_theme_center_, left_x, left_y, time); animate_obj_wh(btn_theme_center_, side_size, side_size, time); } else { lv_obj_set_pos(btn_theme_center_, left_x, left_y); lv_obj_set_size(btn_theme_center_, side_size, side_size);} break;
        case 2: if (time) { animate_obj_xy(btn_settings_, left_x, left_y, time); animate_obj_wh(btn_settings_, side_size, side_size, time); } else { lv_obj_set_pos(btn_settings_, left_x, left_y); lv_obj_set_size(btn_settings_, side_size, side_size);} break;
    }
    // Center (focused)
    switch (focused_index_) {
        case 0: if (time) { animate_obj_xy(btn_music_, center_x, center_y, time); animate_obj_wh(btn_music_, focused_size, focused_size, time); } else { lv_obj_set_pos(btn_music_, center_x, center_y); lv_obj_set_size(btn_music_, focused_size, focused_size);} break;
        case 1: if (time) { animate_obj_xy(btn_theme_center_, center_x, center_y, time); animate_obj_wh(btn_theme_center_, focused_size, focused_size, time); } else { lv_obj_set_pos(btn_theme_center_, center_x, center_y); lv_obj_set_size(btn_theme_center_, focused_size, focused_size);} break;
        case 2: if (time) { animate_obj_xy(btn_settings_, center_x, center_y, time); animate_obj_wh(btn_settings_, focused_size, focused_size, time); } else { lv_obj_set_pos(btn_settings_, center_x, center_y); lv_obj_set_size(btn_settings_, focused_size, focused_size);} break;
    }
    // Right
    switch (right_idx) {
        case 0: if (time) { animate_obj_xy(btn_music_, right_x, right_y, time); animate_obj_wh(btn_music_, side_size, side_size, time); } else { lv_obj_set_pos(btn_music_, right_x, right_y); lv_obj_set_size(btn_music_, side_size, side_size);} break;
        case 1: if (time) { animate_obj_xy(btn_theme_center_, right_x, right_y, time); animate_obj_wh(btn_theme_center_, side_size, side_size, time); } else { lv_obj_set_pos(btn_theme_center_, right_x, right_y); lv_obj_set_size(btn_theme_center_, side_size, side_size);} break;
        case 2: if (time) { animate_obj_xy(btn_settings_, right_x, right_y, time); animate_obj_wh(btn_settings_, side_size, side_size, time); } else { lv_obj_set_pos(btn_settings_, right_x, right_y); lv_obj_set_size(btn_settings_, side_size, side_size);} break;
    }

    // Update focused app label under the centered icon
    lv_obj_t* fb = nullptr;
    const char* name = "";
    switch (focused_index_) {
        case 0: fb = btn_music_; name = "Music"; break;
        case 1: fb = btn_theme_center_; name = "Themes"; break;
        case 2: fb = btn_settings_; name = "Settings"; break;
    }
    if (focused_label_ && fb) {
        lv_label_set_text(focused_label_, name);
        lv_obj_update_layout(focused_label_);
        int label_w = lv_obj_get_width(focused_label_);
        int target_x = center_x + focused_size / 2 - label_w / 2;
        int target_y = center_y + focused_size + 4;
        if (time) {
            animate_obj_xy(focused_label_, target_x, target_y, time);
        } else {
            lv_obj_set_pos(focused_label_, target_x, target_y);
        }
    }
}

void AppLauncher::launchMusic() {
    wm_.openApp(std::unique_ptr<IApp>(new AppMusic()));
}

void AppLauncher::launchThemeCenter() {
    wm_.openApp(std::unique_ptr<IApp>(new AppThemeCenter()));
}

void AppLauncher::launchSettings() {
    wm_.openApp(std::unique_ptr<IApp>(new AppSettings()));
}