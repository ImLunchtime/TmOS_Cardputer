#include "app_music.h"

static void on_dummy_click(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        // Placeholder: do nothing for now
    }
}

void AppMusic::onOpen(lv_obj_t* window_root) {
    root_ = window_root;

    // Layout: column with spacing
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(root_, 10, 0);

    label_ = lv_label_create(root_);
    lv_label_set_text(label_, "Placeholder Music App\nPress BtnA to exit to Launcher");

    btn_play_ = lv_btn_create(root_);
    lv_obj_t* lbl = lv_label_create(btn_play_);
    lv_label_set_text(lbl, "Play/Pause (placeholder)");
    lv_obj_center(lbl);
    // Ensure the button is focusable from keypad
    lv_obj_add_flag(btn_play_, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_event_cb(btn_play_, on_dummy_click, LV_EVENT_CLICKED, nullptr);
}