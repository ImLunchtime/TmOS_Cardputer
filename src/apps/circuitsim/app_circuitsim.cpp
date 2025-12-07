/*
 * @Author: ImLunchtime knoxmedia@yeah.net
 * @Date: 2025-11-30 07:52:58
 * @LastEditors: ImLunchtime knoxmedia@yeah.net
 * @LastEditTime: 2025-12-07 11:24:26
 * @FilePath: \CardputerOS2_LVGL\src\app_circuitsim.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "apps/circuitsim/app_circuitsim.h"
LV_IMG_DECLARE(shield2);

AppCircuitSim::AppCircuitSim() {}
AppCircuitSim::~AppCircuitSim() {}

void AppCircuitSim::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_set_style_pad_row(root_, 0, 0);
    lv_obj_set_style_pad_column(root_, 0, 0);
    lv_obj_set_style_bg_opa(root_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(root_, 0, 0);
    lv_obj_set_style_outline_width(root_, 0, 0);
    lv_obj_set_style_shadow_width(root_, 0, 0);
    lv_obj_set_style_radius(root_, 0, 0);
    lv_obj_set_scroll_dir(root_, LV_DIR_NONE);
    uint32_t child_cnt = lv_obj_get_child_cnt(root_);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(root_, i);
        if (lv_obj_has_class(child, &lv_label_class)) {
            lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
        }
    }
    lv_obj_t* panel = lv_obj_create(root_);
    lv_obj_set_size(panel, shield2.header.w, 18);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x333333), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x888888), 0);
    lv_obj_set_style_border_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(panel, 0, 0);
    lv_obj_set_scroll_dir(panel, LV_DIR_NONE);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t* title = lv_label_create(panel);
    lv_label_set_text(title, "Circuit Sim");
    lv_obj_center(title);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t* img = lv_img_create(root_);
    lv_img_set_src(img, &shield2);
}

void AppCircuitSim::onClose() {}
