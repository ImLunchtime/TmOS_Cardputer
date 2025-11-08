#include "app_test.h"
#include "input_kb.h"

TestApp::TestApp() {}
TestApp::~TestApp() {}

void TestApp::onCreate() {
    // No-op: hardware init handled in main setup
}

void TestApp::onOpen() {
    // Build a simple UI
    page_ = lv_obj_create(lv_scr_act());
    lv_obj_set_size(page_, 240, 135);
    lv_obj_set_scroll_dir(page_, LV_DIR_VER);
    lv_obj_set_flex_flow(page_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(page_, 6, 0);
    lv_obj_set_style_pad_row(page_, 8, 0);

    // Label
    label_ = lv_label_create(page_);
    lv_label_set_text(label_, "Mooncake Test App");
    lv_obj_center(label_);

    // Textarea to test keyboard input
    ta_ = lv_textarea_create(page_);
    lv_obj_set_size(ta_, 220, 90);
    lv_textarea_set_placeholder_text(ta_, "Type here...");

    // Join keyboard group for focus navigation
    auto kb_group = kb_get_group();
    if (kb_group) {
        lv_group_add_obj(kb_group, ta_);
        lv_group_focus_obj(ta_);
        lv_obj_add_flag(ta_, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_group_add_obj(kb_group, label_);
    }
}

void TestApp::onRunning() {
    // Could update UI or handle app-specific logic here
}

void TestApp::onClose() {
    // Clean up created UI objects
    if (page_) {
        lv_obj_del(page_);
        page_ = nullptr;
        label_ = nullptr;
        ta_ = nullptr;
    }
}

void TestApp::onDestroy() {
    onClose();
}