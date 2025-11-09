#include "app_launcher.h"
#include "app_music.h"
#include "app_theme_center.h"
#include "app_settings.h"
#include "theme.h"

// Event handlers for app buttons
static void on_music_btn_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        launcher->launchMusic();
    }
}

static void on_theme_center_btn_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        launcher->launchThemeCenter();
    }
}

static void on_settings_btn_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        launcher->launchSettings();
    }
}

void AppLauncher::onOpen(lv_obj_t* window_root) {
    root_ = window_root;

    // Create main container with grid layout
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 10, 0);
    lv_obj_set_style_pad_row(root_, 10, 0);

    // Title label
    lv_obj_t* title_label = lv_label_create(root_);
    lv_label_set_text(title_label, "Apps");
    lv_obj_set_style_text_font(title_label, ui_theme::get_system_font(), 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xEEEEEE), 0);

    // Grid container for app icons
    grid_container_ = lv_obj_create(root_);
    lv_obj_set_size(grid_container_, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(grid_container_, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(grid_container_, 5, 0);
    lv_obj_set_style_pad_gap(grid_container_, 10, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(grid_container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(grid_container_, 0, 0);

    // Create app buttons in grid
    const int btn_width = 70;
    const int btn_height = 60;

    // Music App Button
    btn_music_ = lv_btn_create(grid_container_);
    lv_obj_set_size(btn_music_, btn_width, btn_height);
    lv_obj_set_style_radius(btn_music_, 0, 0); // Square button
    lv_obj_set_style_bg_color(btn_music_, lv_color_hex(0x2E7D32), 0); // Green background
    lv_obj_set_style_bg_opa(btn_music_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_music_, 0, 0);
    lv_obj_set_style_shadow_width(btn_music_, 0, 0);
    
    // Music button content container
    lv_obj_t* music_cont = lv_obj_create(btn_music_);
    lv_obj_set_size(music_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(music_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(music_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(music_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(music_cont, 0, 0);
    lv_obj_set_style_pad_ver(music_cont, 8, 0); // Padding for top/bottom

    // Music icon at top
    lv_obj_t* music_icon = lv_label_create(music_cont);
    lv_label_set_text(music_icon, "♪");
    lv_obj_set_style_text_font(music_icon, ui_theme::get_system_font(), 0);
    lv_obj_set_style_text_color(music_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_flex_grow(music_icon, 1); // Take available space

    // Music label at bottom
    lv_obj_t* music_label = lv_label_create(music_cont);
    lv_label_set_text(music_label, "Music");
    lv_obj_set_style_text_font(music_label, ui_theme::get_system_font(), 0);
    lv_obj_set_style_text_color(music_label, lv_color_hex(0xFFFFFF), 0);

    lv_obj_add_event_cb(btn_music_, on_music_btn_event, LV_EVENT_CLICKED, this);

    // Theme Center Button
    btn_theme_center_ = lv_btn_create(grid_container_);
    lv_obj_set_size(btn_theme_center_, btn_width, btn_height);
    lv_obj_set_style_radius(btn_theme_center_, 0, 0); // Square button
    lv_obj_set_style_bg_color(btn_theme_center_, lv_color_hex(0xEF6C00), 0); // Orange background
    lv_obj_set_style_bg_opa(btn_theme_center_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_theme_center_, 0, 0);
    lv_obj_set_style_shadow_width(btn_theme_center_, 0, 0);
    
    // Theme button content container
    lv_obj_t* theme_cont = lv_obj_create(btn_theme_center_);
    lv_obj_set_size(theme_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(theme_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(theme_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(theme_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(theme_cont, 0, 0);
    lv_obj_set_style_pad_ver(theme_cont, 8, 0); // Padding for top/bottom

    // Theme icon at top
    lv_obj_t* theme_icon = lv_label_create(theme_cont);
    lv_label_set_text(theme_icon, "🎨");
    lv_obj_set_style_text_font(theme_icon, ui_theme::get_system_font(), 0);
    lv_obj_set_style_text_color(theme_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_flex_grow(theme_icon, 1); // Take available space

    // Theme label at bottom
    lv_obj_t* theme_label = lv_label_create(theme_cont);
    lv_label_set_text(theme_label, "Themes");
    lv_obj_set_style_text_font(theme_label, ui_theme::get_system_font(), 0);
    lv_obj_set_style_text_color(theme_label, lv_color_hex(0xFFFFFF), 0);

    lv_obj_add_event_cb(btn_theme_center_, on_theme_center_btn_event, LV_EVENT_CLICKED, this);

    // Settings Button
    btn_settings_ = lv_btn_create(grid_container_);
    lv_obj_set_size(btn_settings_, btn_width, btn_height);
    lv_obj_set_style_radius(btn_settings_, 0, 0); // Square button
    lv_obj_set_style_bg_color(btn_settings_, lv_color_hex(0x1565C0), 0); // Blue background
    lv_obj_set_style_bg_opa(btn_settings_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_settings_, 0, 0);
    lv_obj_set_style_shadow_width(btn_settings_, 0, 0);
    
    // Settings button content container
    lv_obj_t* settings_cont = lv_obj_create(btn_settings_);
    lv_obj_set_size(settings_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(settings_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(settings_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(settings_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(settings_cont, 0, 0);
    lv_obj_set_style_pad_ver(settings_cont, 8, 0); // Padding for top/bottom

    // Settings icon at top
    lv_obj_t* settings_icon = lv_label_create(settings_cont);
    lv_label_set_text(settings_icon, "⚙");
    lv_obj_set_style_text_font(settings_icon, ui_theme::get_system_font(), 0);
    lv_obj_set_style_text_color(settings_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_flex_grow(settings_icon, 1); // Take available space

    // Settings label at bottom
    lv_obj_t* settings_label = lv_label_create(settings_cont);
    lv_label_set_text(settings_label, "Settings");
    lv_obj_set_style_text_font(settings_label, ui_theme::get_system_font(), 0);
    lv_obj_set_style_text_color(settings_label, lv_color_hex(0xFFFFFF), 0);

    lv_obj_add_event_cb(btn_settings_, on_settings_btn_event, LV_EVENT_CLICKED, this);
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