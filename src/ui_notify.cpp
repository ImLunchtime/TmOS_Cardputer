#include "ui_notify.h"
#include "theme.h"

static lv_obj_t* s_cont = nullptr;
static lv_timer_t* s_timer = nullptr;

static void ensure_container() {
    if (s_cont && lv_obj_is_valid(s_cont)) return;
    s_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_cont, 120, 40);
    lv_obj_set_style_radius(s_cont, 0, 0);
    lv_obj_set_style_bg_color(s_cont, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(s_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_cont, 1, 0);
    lv_obj_set_style_border_color(s_cont, lv_color_hex(0x888888), 0);
    lv_obj_set_style_outline_width(s_cont, 0, 0);
    lv_obj_set_style_shadow_width(s_cont, 0, 0);
    lv_obj_set_style_pad_all(s_cont, 4, 0);
    lv_obj_set_style_pad_row(s_cont, 2, 0);
    lv_obj_set_style_pad_column(s_cont, 4, 0);
    lv_obj_set_flex_flow(s_cont, LV_FLEX_FLOW_ROW);
    lv_obj_align(s_cont, LV_ALIGN_BOTTOM_RIGHT, -2, -2);
    ui_theme::apply_small_text_recursive(s_cont);
    lv_obj_move_foreground(s_cont);
}

static void schedule_hide(uint32_t ms) {
    if (s_timer) {
        lv_timer_del(s_timer);
        s_timer = nullptr;
    }
    s_timer = lv_timer_create([](lv_timer_t* t){
        ui_notify::hide();
    }, ms, nullptr);
}

void ui_notify::hide() {
    if (s_timer) {
        lv_timer_del(s_timer);
        s_timer = nullptr;
    }
    if (s_cont) {
        lv_obj_del(s_cont);
        s_cont = nullptr;
    }
}

void ui_notify::showText(const char* text, uint32_t duration_ms) {
    ensure_container();
    lv_obj_clean(s_cont);
    lv_obj_set_flex_flow(s_cont, LV_FLEX_FLOW_ROW);
    lv_obj_t* lbl = lv_label_create(s_cont);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x000000), 0);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
    lv_obj_set_width(lbl, LV_PCT(100));
    lv_obj_set_flex_grow(lbl, 1);
    lv_label_set_text(lbl, text ? text : "");
    lv_obj_move_foreground(s_cont);
    schedule_hide(duration_ms);
}

void ui_notify::showSymbol(const char* symbol, const char* text, uint32_t duration_ms) {
    ensure_container();
    lv_obj_clean(s_cont);
    lv_obj_set_flex_flow(s_cont, LV_FLEX_FLOW_ROW);
    lv_obj_t* sym = lv_label_create(s_cont);
    lv_obj_set_style_text_color(sym, lv_color_hex(0x000000), 0);
    lv_label_set_text(sym, symbol ? symbol : "");
    lv_obj_set_style_pad_right(sym, 4, 0);
    lv_obj_t* lbl = lv_label_create(s_cont);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x000000), 0);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
    lv_obj_set_flex_grow(lbl, 1);
    lv_label_set_text(lbl, text ? text : "");
    lv_obj_move_foreground(s_cont);
    schedule_hide(duration_ms);
}

void ui_notify::showImage(const lv_img_dsc_t* img, const char* text, uint32_t duration_ms) {
    ensure_container();
    lv_obj_clean(s_cont);
    lv_obj_set_flex_flow(s_cont, LV_FLEX_FLOW_ROW);
    lv_obj_t* icon = lv_img_create(s_cont);
    lv_img_set_src(icon, img);
    lv_obj_set_style_pad_right(icon, 4, 0);
    lv_obj_t* lbl = lv_label_create(s_cont);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x000000), 0);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
    lv_obj_set_flex_grow(lbl, 1);
    lv_label_set_text(lbl, text ? text : "");
    lv_obj_move_foreground(s_cont);
    schedule_hide(duration_ms);
}
