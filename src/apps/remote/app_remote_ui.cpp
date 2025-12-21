#include "apps/remote/app_remote.h"
#include <lvgl.h>

LV_IMG_DECLARE(remote_microcar);
LV_IMG_DECLARE(remote_keyboard_tips);

void AppRemote::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(root_, 6, 0);

    page_main_ = lv_obj_create(root_);
    lv_obj_set_size(page_main_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(page_main_, 1);
    lv_obj_set_style_border_width(page_main_, 0, 0);
    lv_obj_set_style_radius(page_main_, 0, 0);
    lv_obj_set_style_bg_opa(page_main_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page_main_, 0, 0);
    lv_obj_set_style_pad_row(page_main_, 6, 0);
    lv_obj_set_flex_flow(page_main_, LV_FLEX_FLOW_COLUMN);

    lv_obj_t* list_row = lv_obj_create(page_main_);
    lv_obj_set_width(list_row, LV_PCT(100));
    lv_obj_set_height(list_row, 74);
    lv_obj_set_style_border_width(list_row, 0, 0);
    lv_obj_set_style_radius(list_row, 0, 0);
    lv_obj_set_style_bg_opa(list_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(list_row, 0, 0);
    lv_obj_set_style_pad_column(list_row, 8, 0);
    lv_obj_set_flex_flow(list_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(list_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(list_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(list_row, LV_DIR_HOR);
    lv_obj_set_scrollbar_mode(list_row, LV_SCROLLBAR_MODE_OFF);

    btn_microcar_ = lv_btn_create(list_row);
    lv_obj_set_size(btn_microcar_, 64, 64);
    lv_obj_add_flag(btn_microcar_, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_set_style_radius(btn_microcar_, 8, 0);
    lv_obj_set_style_bg_color(btn_microcar_, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_bg_opa(btn_microcar_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_microcar_, 0, 0);
    lv_obj_set_style_shadow_width(btn_microcar_, 0, 0);
    lv_obj_set_style_outline_width(btn_microcar_, 0, 0);
    lv_obj_set_style_pad_all(btn_microcar_, 4, 0);
    lv_obj_set_flex_flow(btn_microcar_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_microcar_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_event_cb(btn_microcar_, on_remote_item_clicked, LV_EVENT_CLICKED, this);
    item_index_[btn_microcar_] = 0;
    {
        lv_obj_t* img = lv_img_create(btn_microcar_);
        lv_img_set_src(img, &remote_microcar);
        lv_obj_set_style_pad_bottom(img, 2, 0);

        lv_obj_t* lbl = lv_label_create(btn_microcar_);
        lv_label_set_text(lbl, "ESP-NOW Car");
        lv_obj_set_width(lbl, 60);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xEEEEEE), 0);
    }

    page_microcar_ = lv_obj_create(root_);
    lv_obj_set_size(page_microcar_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(page_microcar_, 1);
    lv_obj_set_style_border_width(page_microcar_, 0, 0);
    lv_obj_set_style_radius(page_microcar_, 0, 0);
    lv_obj_set_style_bg_opa(page_microcar_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page_microcar_, 0, 0);
    lv_obj_set_style_pad_row(page_microcar_, 4, 0);
    lv_obj_set_flex_flow(page_microcar_, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(page_microcar_, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t* top_row = lv_obj_create(page_microcar_);
    lv_obj_set_width(top_row, LV_PCT(100));
    lv_obj_set_height(top_row, 26);
    lv_obj_set_style_border_width(top_row, 0, 0);
    lv_obj_set_style_bg_opa(top_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(top_row, 0, 0);
    lv_obj_set_style_pad_column(top_row, 6, 0);
    lv_obj_set_flex_flow(top_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    btn_back_ = lv_btn_create(top_row);
    lv_obj_set_size(btn_back_, 24, 24);
    lv_obj_add_flag(btn_back_, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_set_style_radius(btn_back_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_back_, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_bg_opa(btn_back_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_back_, 0, 0);
    lv_obj_set_style_shadow_width(btn_back_, 0, 0);
    lv_obj_set_style_outline_width(btn_back_, 0, 0);
    lv_obj_add_event_cb(btn_back_, on_back_clicked, LV_EVENT_CLICKED, this);
    { lv_obj_t* l = lv_label_create(btn_back_); lv_label_set_text(l, LV_SYMBOL_LEFT); lv_obj_center(l); }

    label_status_ = lv_label_create(top_row);
    lv_label_set_text(label_status_, "ESP-NOW: Idle");
    lv_obj_set_style_text_color(label_status_, lv_color_hex(0xEEEEEE), 0);

    {
        lv_obj_t* img_tips = lv_img_create(page_microcar_);
        lv_img_set_src(img_tips, &remote_keyboard_tips);
        lv_obj_set_style_pad_top(img_tips, 0, 0);
        lv_obj_set_style_pad_bottom(img_tips, 2, 0);
        lv_obj_add_flag(img_tips, LV_OBJ_FLAG_USER_1);
    }

    info_box_ = lv_obj_create(page_microcar_);
    lv_obj_set_size(info_box_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(info_box_, 1);
    lv_obj_set_style_border_width(info_box_, 0, 0);
    lv_obj_set_style_bg_opa(info_box_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(info_box_, 6, 0);
    lv_obj_set_style_pad_row(info_box_, 4, 0);
    lv_obj_set_style_pad_column(info_box_, 6, 0);
    lv_obj_set_flex_flow(info_box_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(info_box_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(info_box_, LV_OBJ_FLAG_SCROLLABLE);

    rebuildFocusFor(page_main_, btn_microcar_);
}
