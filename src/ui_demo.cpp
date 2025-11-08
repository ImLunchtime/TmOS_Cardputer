#include <lvgl.h>

LV_IMG_DECLARE(testimg);

void ui_build_demo(lv_group_t* kb_group) {
    lv_obj_t * page = lv_obj_create(lv_scr_act());
    lv_obj_set_size(page, 240, 135);
    lv_obj_set_scroll_dir(page, LV_DIR_VER);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(page, 6, 0);
    lv_obj_set_style_pad_row(page, 8, 0);

    lv_obj_t * btn1 = lv_btn_create(page);
    lv_obj_t * label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);
    lv_group_add_obj(kb_group, btn1);
    lv_obj_add_flag(btn1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    lv_obj_t * ta = lv_textarea_create(page);
    lv_obj_set_size(ta, 220, 90);
    lv_textarea_set_placeholder_text(ta, "Type here...");
    lv_group_add_obj(kb_group, ta);
    lv_group_focus_obj(ta);
    lv_obj_add_flag(ta, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    lv_obj_t * img = lv_img_create(page);
    lv_img_set_src(img, &testimg);
    lv_group_add_obj(kb_group, img);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(img, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
}