#include "apps/brightness/app_brightness.h"
#include "globals.h"
#include <lvgl.h>
#include <M5Cardputer.h>

AppBrightness::AppBrightness() {}
AppBrightness::~AppBrightness() {}

void AppBrightness::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 6, 0);
    lv_obj_set_style_pad_row(root_, 8, 0);
    ui_theme::apply_small_text_recursive(root_);

    label_ = lv_label_create(root_);
    lv_label_set_text(label_, "屏幕亮度");
    lv_obj_set_style_pad_bottom(label_, 4, 0);

    slider_ = lv_slider_create(root_);
    lv_obj_set_width(slider_, lv_pct(100));
    lv_slider_set_range(slider_, 0, 9);
    int lvl = globals::get_brightness_level();
    lv_slider_set_value(slider_, lvl, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider_, on_slider_changed, LV_EVENT_VALUE_CHANGED, this);
}

void AppBrightness::onClose() {}

void AppBrightness::on_slider_changed(lv_event_t* e) {
    auto* app = static_cast<AppBrightness*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    int lvl = lv_slider_get_value(app->slider_);
    globals::set_brightness_level(lvl);
}

