#include "app_test_iapp.h"

AppTest::AppTest() {}
AppTest::~AppTest() {}

void AppTest::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 10, 0);
    lv_obj_set_style_pad_row(root_, 10, 0);

    btn_ = lv_btn_create(root_);
    ui_theme::apply_button(btn_);
    label_ = lv_label_create(btn_);
    lv_label_set_text(label_, "Click Me");
    lv_obj_center(label_);
    lv_obj_add_event_cb(btn_, on_btn_clicked, LV_EVENT_CLICKED, this);
}

void AppTest::onClose() {}

void AppTest::on_btn_clicked(lv_event_t* e) {
    auto* app = static_cast<AppTest*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        if (app->label_) {
            lv_label_set_text(app->label_, "Pressed!");
        }
    }
}