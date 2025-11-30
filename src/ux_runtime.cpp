#include "ux_runtime.h"
#include <SD.h>
#include "theme.h"

namespace ux {

static lv_obj_t* s_toast = nullptr;

static String trim_ws(const String& s) {
    int i = 0; int j = s.length() - 1;
    while (i <= j && isspace((int)s.charAt(i))) i++;
    while (j >= i && isspace((int)s.charAt(j))) j--;
    if (j < i) return String("");
    return s.substring(i, j + 1);
}

static bool find_key(const String& json, const char* key, int& val_start, int& val_end) {
    String k = String("\"") + key + String("\"");
    int p = json.indexOf(k);
    if (p < 0) return false;
    int colon = json.indexOf(':', p + k.length());
    if (colon < 0) return false;
    int start = colon + 1;
    while (start < (int)json.length() && isspace((int)json.charAt(start))) start++;
    if (start >= (int)json.length()) return false;
    char c = json.charAt(start);
    if (c == '"') {
        int endq = json.indexOf('"', start + 1);
        if (endq < 0) return false;
        val_start = start + 1;
        val_end = endq;
        return true;
    }
    if (c == '[') {
        int depth = 1; int i = start + 1;
        while (i < (int)json.length() && depth > 0) {
            char ch = json.charAt(i);
            if (ch == '[') depth++;
            else if (ch == ']') depth--;
            i++;
        }
        if (depth != 0) return false;
        val_start = start;
        val_end = i;
        return true;
    }
    if (c == '{') {
        int depth = 1; int i = start + 1;
        while (i < (int)json.length() && depth > 0) {
            char ch = json.charAt(i);
            if (ch == '{') depth++;
            else if (ch == '}') depth--;
            i++;
        }
        if (depth != 0) return false;
        val_start = start;
        val_end = i;
        return true;
    }
    int i = start;
    while (i < (int)json.length()) {
        char ch = json.charAt(i);
        if (ch == ',' || ch == '}' || ch == ']') break;
        i++;
    }
    val_start = start;
    val_end = i;
    return true;
}

static String get_str(const String& json, const char* key) {
    int s, e; if (!find_key(json, key, s, e)) return String("");
    return json.substring(s, e);
}

static long get_int(const String& json, const char* key, long defv) {
    int s, e; if (!find_key(json, key, s, e)) return defv;
    String v = trim_ws(json.substring(s, e));
    return v.toInt();
}

static bool get_bool(const String& json, const char* key, bool defv) {
    int s, e; if (!find_key(json, key, s, e)) return defv;
    String v = trim_ws(json.substring(s, e));
    v.toLowerCase();
    if (v == "true") return true;
    if (v == "false") return false;
    return defv;
}

static void apply_align(lv_obj_t* obj, const String& a) {
    String v = a; v.toLowerCase();
    if (v == "center") lv_obj_center(obj);
    else if (v == "top_left") lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 0);
    else if (v == "top_middle") lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, 0);
    else if (v == "top_right") lv_obj_align(obj, LV_ALIGN_TOP_RIGHT, 0, 0);
    else if (v == "bottom_left") lv_obj_align(obj, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    else if (v == "bottom_middle") lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 0, 0);
    else if (v == "bottom_right") lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    else if (v == "left_mid") lv_obj_align(obj, LV_ALIGN_LEFT_MID, 0, 0);
    else if (v == "right_mid") lv_obj_align(obj, LV_ALIGN_RIGHT_MID, 0, 0);
}

void clear_children(lv_obj_t* parent) {
    if (!parent) return;
    uint32_t n = lv_obj_get_child_cnt(parent);
    for (uint32_t i = 0; i < n; ++i) {
        lv_obj_t* c = lv_obj_get_child(parent, 0);
        lv_obj_del(c);
    }
}

static void show_toast(lv_obj_t* root, const String& text) {
    if (s_toast) {
        lv_obj_del(s_toast);
        s_toast = nullptr;
    }
    lv_obj_t* cont = lv_obj_create(root);
    lv_obj_set_size(cont, lv_pct(90), LV_SIZE_CONTENT);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_bg_opa(cont, LV_OPA_80, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(cont, 8, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_t* lbl = lv_label_create(cont);
    lv_label_set_text(lbl, text.c_str());
    lv_obj_set_style_text_font(lbl, ui_theme::get_system_font(), 0);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(lbl);
    s_toast = cont;
    lv_obj_add_event_cb(cont, [](lv_event_t* e){ s_toast = nullptr; }, LV_EVENT_DELETE, nullptr);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, cont);
    lv_anim_set_values(&a, 255, 0);
    lv_anim_set_exec_cb(&a, [](void* var, int32_t v){ lv_obj_set_style_opa((lv_obj_t*)var, (lv_opa_t)v, 0); });
    lv_anim_set_time(&a, 600);
    lv_anim_set_delay(&a, 1400);
    lv_anim_set_ready_cb(&a, [](lv_anim_t* aa){ lv_obj_del((lv_obj_t*)aa->var); });
    lv_anim_start(&a);
}

static void build_widget_from_obj(const String& obj, lv_obj_t* parent) {
    String type = get_str(obj, "type");
    type.toLowerCase();
    lv_obj_t* w = nullptr;
    if (type == "label") {
        w = lv_label_create(parent);
        String t = get_str(obj, "text");
        lv_label_set_text(w, t.c_str());
    } else if (type == "button") {
        w = lv_btn_create(parent);
        String t = get_str(obj, "text");
        if (t.length() > 0) { lv_obj_t* l = lv_label_create(w); lv_label_set_text(l, t.c_str()); lv_obj_center(l); }
        String on = get_str(obj, "on");
        if (on.length() > 0) {
            int s,e; if (find_key(on, "click", s, e)) {
                String click = on.substring(s, e);
                String act = get_str(click, "action"); act.toLowerCase();
                if (act == "toast") {
                    String msg = get_str(click, "text");
                    String* msgp = new String(msg);
                    lv_obj_add_event_cb(w, [](lv_event_t* e){
                        String* m = (String*)lv_event_get_user_data(e);
                        lv_obj_t* obj = lv_event_get_target(e);
                        lv_obj_t* root = lv_obj_get_parent(obj);
                        while (root && lv_obj_get_parent(root)) root = lv_obj_get_parent(root);
                        if (m) show_toast(root, *m);
                    }, LV_EVENT_CLICKED, msgp);
                    lv_obj_add_event_cb(w, [](lv_event_t* e){
                        String* m = (String*)lv_event_get_user_data(e);
                        if (m) { delete m; }
                    }, LV_EVENT_DELETE, msgp);
                }
            }
        }
    } else if (type == "textarea") {
        w = lv_textarea_create(parent);
        String ph = get_str(obj, "placeholder");
        if (ph.length() > 0) lv_textarea_set_placeholder_text(w, ph.c_str());
        String t = get_str(obj, "text");
        if (t.length() > 0) lv_textarea_set_text(w, t.c_str());
    } else if (type == "slider") {
        w = lv_slider_create(parent);
        long minv = get_int(obj, "min", 0);
        long maxv = get_int(obj, "max", 100);
        long val = get_int(obj, "value", 0);
        lv_slider_set_range(w, minv, maxv);
        lv_slider_set_value(w, val, LV_ANIM_OFF);
    } else if (type == "switch") {
        w = lv_switch_create(parent);
        bool on = get_bool(obj, "checked", false);
        if (on) lv_obj_add_state(w, LV_STATE_CHECKED);
    } else if (type == "dropdown") {
        w = lv_dropdown_create(parent);
        String opts = get_str(obj, "options");
        if (opts.length() > 0) lv_dropdown_set_options(w, opts.c_str());
    } else if (type == "image") {
        struct ImgMem { lv_img_dsc_t dsc; uint8_t* buf; };
        String p = get_str(obj, "src");
        if (p.length() > 0) {
            if (!p.startsWith("/")) p = String("/") + p;
            File f = SD.open(p.c_str(), FILE_READ);
            if (!f) {
                w = lv_label_create(parent);
                lv_label_set_text(w, "图片加载失败");
                lv_obj_set_style_text_color(w, lv_color_hex(0xEEEEEE), 0);
                lv_obj_set_style_text_font(w, ui_theme::get_system_font(), 0);
            } else {
                size_t n = f.size();
                if (n < 4) {
                    f.close();
                    w = lv_label_create(parent);
                    lv_label_set_text(w, "图片加载失败");
                    lv_obj_set_style_text_color(w, lv_color_hex(0xEEEEEE), 0);
                    lv_obj_set_style_text_font(w, ui_theme::get_system_font(), 0);
                } else {
                    uint8_t hdr[4];
                    size_t rd = f.read(hdr, 4);
                    if (rd != 4) {
                        f.close();
                        w = lv_label_create(parent);
                        lv_label_set_text(w, "图片加载失败");
                        lv_obj_set_style_text_color(w, lv_color_hex(0xEEEEEE), 0);
                        lv_obj_set_style_text_font(w, ui_theme::get_system_font(), 0);
                    } else {
                        uint32_t h32 = (uint32_t)hdr[0] | ((uint32_t)hdr[1] << 8) | ((uint32_t)hdr[2] << 16) | ((uint32_t)hdr[3] << 24);
                        uint8_t cf = (uint8_t)(h32 & 0x1F);
                        uint16_t iw = (uint16_t)((h32 >> 10) & 0x7FF);
                        uint16_t ih = (uint16_t)((h32 >> 21) & 0x7FF);
                        if (iw == 0 || ih == 0 || cf != LV_IMG_CF_TRUE_COLOR) {
                            f.close();
                            w = lv_label_create(parent);
                            lv_label_set_text(w, "图片加载失败");
                            lv_obj_set_style_text_color(w, lv_color_hex(0xEEEEEE), 0);
                            lv_obj_set_style_text_font(w, ui_theme::get_system_font(), 0);
                        } else {
                            size_t data_sz = (size_t)iw * (size_t)ih * 2;
                            if (n != data_sz + 4) {
                                f.close();
                                w = lv_label_create(parent);
                                lv_label_set_text(w, "图片加载失败");
                                lv_obj_set_style_text_color(w, lv_color_hex(0xEEEEEE), 0);
                                lv_obj_set_style_text_font(w, ui_theme::get_system_font(), 0);
                            } else {
                                ImgMem* mem = (ImgMem*)malloc(sizeof(ImgMem));
                                if (!mem) {
                                    f.close();
                                    w = lv_label_create(parent);
                                    lv_label_set_text(w, "图片加载失败");
                                    lv_obj_set_style_text_color(w, lv_color_hex(0xEEEEEE), 0);
                                    lv_obj_set_style_text_font(w, ui_theme::get_system_font(), 0);
                                } else {
                                    mem->dsc.header.always_zero = 0;
                                    mem->dsc.header.w = iw;
                                    mem->dsc.header.h = ih;
                                    mem->dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
                                    mem->dsc.data_size = data_sz;
                                    mem->buf = (uint8_t*)malloc(data_sz);
                                    if (!mem->buf) {
                                        f.close();
                                        free(mem);
                                        w = lv_label_create(parent);
                                        lv_label_set_text(w, "图片加载失败");
                                        lv_obj_set_style_text_color(w, lv_color_hex(0xEEEEEE), 0);
                                        lv_obj_set_style_text_font(w, ui_theme::get_system_font(), 0);
                                    } else {
                                        size_t rimg = f.read(mem->buf, data_sz);
                                        f.close();
                                        if (rimg != data_sz) {
                                            free(mem->buf);
                                            free(mem);
                                            w = lv_label_create(parent);
                                            lv_label_set_text(w, "图片加载失败");
                                            lv_obj_set_style_text_color(w, lv_color_hex(0xEEEEEE), 0);
                                            lv_obj_set_style_text_font(w, ui_theme::get_system_font(), 0);
                                        } else {
                                            mem->dsc.data = (const uint8_t*)mem->buf;
                                            w = lv_img_create(parent);
                                            lv_img_set_src(w, &mem->dsc);
                                            lv_obj_add_flag(w, LV_OBJ_FLAG_CLICKABLE);
                                            lv_obj_add_flag(w, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
                                            lv_obj_add_flag(w, LV_OBJ_FLAG_USER_1);
                                            lv_obj_add_event_cb(w, [](lv_event_t* e){
                                                ImgMem* m = (ImgMem*)lv_event_get_user_data(e);
                                                if (m) {
                                                    if (m->buf) free(m->buf);
                                                    free(m);
                                                }
                                            }, LV_EVENT_DELETE, mem);
                                            lv_obj_align(w, LV_ALIGN_CENTER, 0, 0);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else {
            w = lv_label_create(parent);
            lv_label_set_text(w, "图片加载失败");
            lv_obj_set_style_text_color(w, lv_color_hex(0xEEEEEE), 0);
            lv_obj_set_style_text_font(w, ui_theme::get_system_font(), 0);
        }
    }
    if (!w) return;
    long wv = get_int(obj, "w", -1);
    long hv = get_int(obj, "h", -1);
    if (wv > 0 || hv > 0) lv_obj_set_size(w, wv > 0 ? (lv_coord_t)wv : LV_SIZE_CONTENT, hv > 0 ? (lv_coord_t)hv : LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(w, 2, 0);
    lv_obj_set_style_pad_row(w, 2, 0);
    lv_obj_set_style_pad_column(w, 2, 0);
    String align = get_str(obj, "align");
    if (align.length() > 0) apply_align(w, align);
}

static void build_widgets_array(const String& arr, lv_obj_t* parent) {
    int i = 0; int n = arr.length();
    while (i < n) {
        int obj_start = arr.indexOf('{', i);
        if (obj_start < 0) break;
        int depth = 1; int j = obj_start + 1;
        while (j < n && depth > 0) {
            char ch = arr.charAt(j);
            if (ch == '{') depth++;
            else if (ch == '}') depth--;
            j++;
        }
        if (depth != 0) break;
        String obj = arr.substring(obj_start, j);
        build_widget_from_obj(obj, parent);
        i = j;
    }
}

BuildResult build_from_string(const String& json, lv_obj_t* root) {
    BuildResult r; r.ok = false; r.error = String("");
    if (!root) { r.error = String("no root"); return r; }
    clear_children(root);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root, 2, 0);
    lv_obj_set_style_pad_row(root, 2, 0);
    lv_obj_set_style_pad_column(root, 2, 0);
    int s,e; if (!find_key(json, "widgets", s, e)) { r.ok = true; return r; }
    String arr = json.substring(s, e);
    build_widgets_array(arr, root);
    r.ok = true;
    return r;
}

BuildResult build_from_file(const String& path, lv_obj_t* root) {
    BuildResult r; r.ok = false; r.error = String("");
    File f = SD.open(path);
    if (!f) { r.error = String("open fail"); return r; }
    String j = "";
    while (f.available()) j += (char)f.read();
    f.close();
    return build_from_string(j, root);
}

}
static lv_obj_t* s_toast = nullptr;
