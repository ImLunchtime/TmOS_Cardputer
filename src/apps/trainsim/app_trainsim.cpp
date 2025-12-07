#include "apps/trainsim/app_trainsim.h"
#include <cstdio>

LV_FONT_DECLARE(simhei_12);

AppTrainSim::AppTrainSim() {}
AppTrainSim::~AppTrainSim() {}

static void throttle_event_cb(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    lv_obj_t* label = (lv_obj_t*)lv_event_get_user_data(e);
    int val = lv_slider_get_value(slider) * 10;
    char buf[32];
    std::sprintf(buf, "%d%%", val);
    lv_label_set_text(label, buf);
}

static void brake_event_cb(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    lv_obj_t* label = (lv_obj_t*)lv_event_get_user_data(e);
    int val = lv_slider_get_value(slider) * 10;
    char buf[32];
    std::sprintf(buf, "%d%%", val);
    lv_label_set_text(label, buf);
}

void AppTrainSim::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    
    // Maximize window to full screen
    lv_obj_set_size(root_, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_set_pos(root_, 0, 0);

    // Main Container - Dark Blue Theme
    lv_obj_set_style_bg_color(root_, lv_color_hex(0x001040), 0);
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_set_style_border_width(root_, 0, 0);
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_ROW);
    lv_obj_set_scroll_dir(root_, LV_DIR_NONE);

    // --- LEFT SIDE: SLIDERS ---
    lv_obj_t* sliders_panel = lv_obj_create(root_);
    lv_obj_set_size(sliders_panel, LV_SIZE_CONTENT, lv_pct(100));
    lv_obj_set_style_bg_opa(sliders_panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(sliders_panel, 0, 0);
    lv_obj_set_style_pad_all(sliders_panel, 2, 0);
    lv_obj_set_style_pad_gap(sliders_panel, 2, 0); // Very narrow gap between sliders
    lv_obj_set_flex_flow(sliders_panel, LV_FLEX_FLOW_ROW);
    lv_obj_clear_flag(sliders_panel, LV_OBJ_FLAG_SCROLLABLE);

    // Throttle Group
    lv_obj_t* throttle_group = lv_obj_create(sliders_panel);
    lv_obj_set_size(throttle_group, 20, lv_pct(100));
    lv_obj_set_style_bg_opa(throttle_group, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(throttle_group, 0, 0);
    lv_obj_set_style_pad_all(throttle_group, 0, 0);
    lv_obj_set_flex_flow(throttle_group, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(throttle_group, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(throttle_group, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* throttle_label = lv_label_create(throttle_group);
    lv_label_set_text(throttle_label, "0%");
    lv_obj_set_style_text_font(throttle_label, &simhei_12, 0);
    lv_obj_set_style_text_color(throttle_label, lv_color_hex(0x00FF00), 0);
    
    throttle_slider_ = lv_slider_create(throttle_group);
    lv_obj_set_size(throttle_slider_, 10, 70); // Slightly shorter to fit text
    lv_slider_set_range(throttle_slider_, 0, 10);
    lv_obj_set_style_bg_color(throttle_slider_, lv_color_hex(0x202020), LV_PART_MAIN);
    lv_obj_set_style_bg_color(throttle_slider_, lv_color_hex(0x00FF00), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(throttle_slider_, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_add_event_cb(throttle_slider_, throttle_event_cb, LV_EVENT_VALUE_CHANGED, throttle_label);

    lv_obj_t* t_title = lv_label_create(throttle_group);
    lv_label_set_text(t_title, "P");
    lv_obj_set_style_text_font(t_title, &simhei_12, 0);
    lv_obj_set_style_text_color(t_title, lv_color_hex(0x00FF00), 0);

    // Brake Group
    lv_obj_t* brake_group = lv_obj_create(sliders_panel);
    lv_obj_set_size(brake_group, 20, lv_pct(100));
    lv_obj_set_style_bg_opa(brake_group, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(brake_group, 0, 0);
    lv_obj_set_style_pad_all(brake_group, 0, 0);
    lv_obj_set_flex_flow(brake_group, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(brake_group, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(brake_group, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* brake_label = lv_label_create(brake_group);
    lv_label_set_text(brake_label, "0%");
    lv_obj_set_style_text_font(brake_label, &simhei_12, 0);
    lv_obj_set_style_text_color(brake_label, lv_color_hex(0xFF0000), 0);

    brake_slider_ = lv_slider_create(brake_group);
    lv_obj_set_size(brake_slider_, 10, 70);
    lv_slider_set_range(brake_slider_, 0, 10);
    lv_obj_set_style_bg_color(brake_slider_, lv_color_hex(0x202020), LV_PART_MAIN);
    lv_obj_set_style_bg_color(brake_slider_, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(brake_slider_, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_add_event_cb(brake_slider_, brake_event_cb, LV_EVENT_VALUE_CHANGED, brake_label);

    lv_obj_t* b_title = lv_label_create(brake_group);
    lv_label_set_text(b_title, "B");
    lv_obj_set_style_text_font(b_title, &simhei_12, 0);
    lv_obj_set_style_text_color(b_title, lv_color_hex(0xFF0000), 0);


    // --- RIGHT SIDE: CONTROLS ---
    lv_obj_t* controls_panel = lv_obj_create(root_);
    lv_obj_set_flex_grow(controls_panel, 1);
    lv_obj_set_height(controls_panel, lv_pct(100));
    lv_obj_set_style_bg_opa(controls_panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(controls_panel, 0, 0);
    lv_obj_set_style_pad_all(controls_panel, 2, 0);
    lv_obj_set_flex_flow(controls_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(controls_panel, LV_OBJ_FLAG_SCROLLABLE);

    // Speed Display Area
    lv_obj_t* speed_box = lv_obj_create(controls_panel);
    lv_obj_set_width(speed_box, lv_pct(100));
    lv_obj_set_height(speed_box, 45); // Compact height
    lv_obj_set_style_bg_color(speed_box, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_color(speed_box, lv_color_hex(0x404080), 0);
    lv_obj_set_style_border_width(speed_box, 1, 0);
    lv_obj_set_style_radius(speed_box, 4, 0);
    lv_obj_set_style_pad_all(speed_box, 0, 0);
    lv_obj_clear_flag(speed_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* speed_val = lv_label_create(speed_box);
    lv_label_set_text(speed_val, "000");
    // Use smaller font that is definitely available
    lv_obj_set_style_text_font(speed_val, &lv_font_montserrat_16, 0); 
    lv_obj_set_style_text_color(speed_val, lv_color_hex(0x00FFFF), 0);
    lv_obj_align(speed_val, LV_ALIGN_CENTER, 0, -4);

    lv_obj_t* kmh_label = lv_label_create(speed_box);
    lv_label_set_text(kmh_label, "km/h");
    lv_obj_set_style_text_font(kmh_label, &simhei_12, 0);
    lv_obj_set_style_text_color(kmh_label, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(kmh_label, LV_ALIGN_BOTTOM_MID, 0, 0);

    // Control Buttons Grid
    lv_obj_t* btn_grid = lv_obj_create(controls_panel);
    lv_obj_set_size(btn_grid, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(btn_grid, 1);
    lv_obj_set_style_bg_opa(btn_grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_grid, 0, 0);
    lv_obj_set_style_pad_all(btn_grid, 0, 0);
    lv_obj_set_style_pad_gap(btn_grid, 2, 0);
    lv_obj_set_flex_flow(btn_grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_clear_flag(btn_grid, LV_OBJ_FLAG_SCROLLABLE);

    const char* btns[] = {"DL", "DR", "HORN", "LIT", "EMG", "ANN"};
    for(int i=0; i<6; i++) {
        lv_obj_t* btn = lv_btn_create(btn_grid);
        lv_obj_set_width(btn, lv_pct(31)); // 3 per row approx
        lv_obj_set_height(btn, 25);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x303060), 0);
        lv_obj_set_style_radius(btn, 2, 0);
        lv_obj_set_style_pad_all(btn, 0, 0);
        
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, btns[i]);
        lv_obj_set_style_text_font(lbl, &simhei_12, 0);
        lv_obj_center(lbl);
    }
}

void AppTrainSim::onClose() {
    // Clean up if needed
}
