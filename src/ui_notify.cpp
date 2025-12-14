#include "ui_notify.h"
#include "theme.h"

static lv_obj_t* s_cont = nullptr;
static lv_timer_t* s_timer = nullptr;

static void anim_exec_x(void* obj, int32_t v) { lv_obj_set_x((lv_obj_t*)obj, v); }
static void anim_exec_y(void* obj, int32_t v) { lv_obj_set_y((lv_obj_t*)obj, v); }

static void ensure_container() {
    if (s_cont && lv_obj_is_valid(s_cont)) return;
    s_cont = lv_obj_create(lv_layer_top());
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

static void start_appear_anim(uint32_t time_ms = 180) {
    lv_obj_align(s_cont, LV_ALIGN_BOTTOM_RIGHT, -2, -2);
    int tx = lv_obj_get_x(s_cont);
    int ty = lv_obj_get_y(s_cont);
    lv_coord_t w = lv_obj_get_width(s_cont);
    lv_coord_t h = lv_obj_get_height(s_cont);
    lv_obj_set_pos(s_cont, tx + w + 10, ty + h + 10);
    lv_anim_t ax;
    lv_anim_init(&ax);
    lv_anim_set_var(&ax, s_cont);
    lv_anim_set_values(&ax, tx + w + 10, tx);
    lv_anim_set_time(&ax, time_ms);
    lv_anim_set_exec_cb(&ax, anim_exec_x);
    lv_anim_set_path_cb(&ax, lv_anim_path_ease_out);
    lv_anim_start(&ax);
    lv_anim_t ay;
    lv_anim_init(&ay);
    lv_anim_set_var(&ay, s_cont);
    lv_anim_set_values(&ay, ty + h + 10, ty);
    lv_anim_set_time(&ay, time_ms);
    lv_anim_set_exec_cb(&ay, anim_exec_y);
    lv_anim_set_path_cb(&ay, lv_anim_path_ease_out);
    lv_anim_start(&ay);
}

static void destroy_container() {
    if (s_cont) {
        lv_obj_del(s_cont);
        s_cont = nullptr;
    }
}

static void start_disappear_anim(uint32_t time_ms = 150) {
    if (s_timer) {
        lv_timer_del(s_timer);
        s_timer = nullptr;
    }
    if (!s_cont) return;
    int sx = lv_obj_get_x(s_cont);
    int sy = lv_obj_get_y(s_cont);
    lv_coord_t hor = lv_disp_get_hor_res(NULL);
    lv_coord_t ver = lv_disp_get_ver_res(NULL);
    lv_anim_t ax;
    lv_anim_init(&ax);
    lv_anim_set_var(&ax, s_cont);
    lv_anim_set_values(&ax, sx, hor + 10);
    lv_anim_set_time(&ax, time_ms);
    lv_anim_set_exec_cb(&ax, anim_exec_x);
    lv_anim_set_path_cb(&ax, lv_anim_path_ease_in);
    lv_anim_start(&ax);
    lv_anim_t ay;
    lv_anim_init(&ay);
    lv_anim_set_var(&ay, s_cont);
    lv_anim_set_values(&ay, sy, ver + 10);
    lv_anim_set_time(&ay, time_ms);
    lv_anim_set_exec_cb(&ay, anim_exec_y);
    lv_anim_set_path_cb(&ay, lv_anim_path_ease_in);
    lv_anim_set_ready_cb(&ay, [](lv_anim_t* a){ destroy_container(); });
    lv_anim_start(&ay);
}

static void schedule_hide(uint32_t ms) {
    if (s_timer) {
        lv_timer_del(s_timer);
        s_timer = nullptr;
    }
    s_timer = lv_timer_create([](lv_timer_t* t){
        start_disappear_anim();
    }, ms, nullptr);
}

void ui_notify::hide() {
    if (s_timer) {
        lv_timer_del(s_timer);
        s_timer = nullptr;
    }
    start_disappear_anim();
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
    start_appear_anim();
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
    start_appear_anim();
    schedule_hide(duration_ms);
}

void ui_notify::showImage(const lv_img_dsc_t* img, const char* text, uint32_t duration_ms) {
    ensure_container();
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
