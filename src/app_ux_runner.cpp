#include "app_ux_runner.h"

void AppUXRunner::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    {
        lv_coord_t scr_w = lv_disp_get_hor_res(NULL);
        lv_coord_t scr_h = lv_disp_get_ver_res(NULL);
        lv_coord_t w = (scr_w * 7) / 10;
        lv_coord_t h = (scr_h * 7) / 10;
        wm_.resizeWindow(root_, w, h);
    }
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 2, 0);
    lv_obj_set_style_pad_row(root_, 2, 0);
    lv_obj_set_style_pad_column(root_, 2, 0);
    auto r = ux::build_from_file(path_, root_);
    if (!r.ok) {
        lv_obj_t* lbl = lv_label_create(root_);
        String msg = String("Error: ") + r.error;
        lv_label_set_text(lbl, msg.c_str());
    }
}
