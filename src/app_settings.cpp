#include "app_settings.h"
#include "theme.h"

AppSettings::AppSettings() {}

AppSettings::~AppSettings() {}

void AppSettings::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    buildUI(root_);
}

void AppSettings::buildUI(lv_obj_t* parent) {
    // Set up the main container layout
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 10, 0);
    lv_obj_set_style_pad_row(parent, 15, 0);

    // Title
    title_label_ = lv_label_create(parent);
    lv_label_set_text(title_label_, "Settings");
    lv_obj_set_style_text_font(title_label_, ui_theme::get_system_font(), 0);
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0xEEEEEE), 0);

    // Info label
    info_label_ = lv_label_create(parent);
    lv_label_set_text(info_label_, "System settings coming soon!\nThis is a placeholder for the settings app.");
    lv_obj_set_style_text_font(info_label_, ui_theme::get_system_font(), 0);
    lv_obj_set_style_text_color(info_label_, lv_color_hex(0xBBBBBB), 0);
    lv_obj_set_style_text_align(info_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(info_label_, lv_pct(90));

    // Close button
    close_btn_ = lv_btn_create(parent);
    lv_obj_t* close_label = lv_label_create(close_btn_);
    lv_label_set_text(close_label, "Close");
    lv_obj_center(close_label);
    ui_theme::apply_button(close_btn_);
    lv_obj_add_event_cb(close_btn_, on_close_btn_event, LV_EVENT_CLICKED, this);
}

void AppSettings::onClose() {
    // Cleanup is handled by WindowSystem
}

void AppSettings::on_close_btn_event(lv_event_t* e) {
    auto* app = static_cast<AppSettings*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        // The WindowSystem will handle the actual window closing
        // when BtnA is pressed, but we can trigger it here if needed
    }
}