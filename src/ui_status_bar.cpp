#include "ui_status_bar.h"
#include <M5Cardputer.h>
#include "theme.h"

namespace ui_status_bar {
static lv_obj_t* s_bar = nullptr;
static lv_obj_t* s_icon = nullptr;
static lv_obj_t* s_label = nullptr;
static lv_timer_t* s_timer = nullptr;

static void update() {
    int lvl = M5Cardputer.Power.getBatteryLevel();
    int mv = M5Cardputer.Power.getBatteryVoltage();
    if (s_label) {
        lv_label_set_text_fmt(s_label, "%d%% %dmV", lvl, mv);
    }
    if (s_icon) {
        lv_label_set_text(s_icon, LV_SYMBOL_BATTERY_FULL);
    }
}

static void on_timer(lv_timer_t* t) {
    (void)t;
    update();
}

void init() {
    if (s_bar && lv_obj_is_valid(s_bar)) return;
    lv_obj_t* root = lv_layer_top();
    s_bar = lv_obj_create(root);
    lv_obj_set_size(s_bar, 144, 16);
    lv_obj_set_style_radius(s_bar, 0, 0);
    lv_obj_set_style_border_width(s_bar, 0, 0);
    lv_obj_set_style_bg_color(s_bar, lv_color_hex(0x202020), 0);
    lv_obj_set_style_bg_opa(s_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(s_bar, 0, 0);
    lv_obj_set_style_pad_row(s_bar, 0, 0);
    lv_obj_set_style_pad_column(s_bar, 4, 0);
    lv_obj_set_flex_flow(s_bar, LV_FLEX_FLOW_ROW);
    lv_obj_add_flag(s_bar, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(s_bar, LV_ALIGN_TOP_RIGHT, 0, 0);
    s_icon = lv_label_create(s_bar);
    lv_obj_set_style_text_color(s_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_icon, ui_theme::get_system_font(), 0);
    s_label = lv_label_create(s_bar);
    lv_obj_set_style_text_color(s_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_label, ui_theme::get_system_font(), 0);
    lv_obj_set_flex_grow(s_label, 1);
    update();
    s_timer = lv_timer_create(on_timer, 1000, NULL);
    lv_obj_move_foreground(s_bar);
}

void deinit() {
    if (s_timer) {
        lv_timer_del(s_timer);
        s_timer = nullptr;
    }
    if (s_bar) {
        lv_obj_del(s_bar);
        s_bar = nullptr;
        s_icon = nullptr;
        s_label = nullptr;
    }
}
}
