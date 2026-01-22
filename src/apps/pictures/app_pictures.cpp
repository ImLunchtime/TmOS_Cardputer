#include "apps/pictures/app_pictures.h"
#include <lvgl.h>
#include <M5Cardputer.h>
#include <cstring>
#include <MD5Builder.h>
#include "drivers/input_kb.h"
#include "ui/ui_notify.h"

AppPictures::AppPictures() {}
AppPictures::~AppPictures() {}

void AppPictures::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 6, 0);
    lv_obj_set_style_pad_row(root_, 6, 0);
    lv_obj_set_style_text_font(root_, ui_theme::get_system_font(), 0);

    if (!fm_.initialize()) {
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "SD初始化失败", 2500);
        return;
    }

    files_.clear();
    current_index_ = -1;
    scanning_ = fm_.beginScan("/", ".bin");
    scan_done_ = false;
    if (status_label_) {
        lv_obj_del(status_label_);
        status_label_ = nullptr;
    }
    status_label_ = lv_label_create(root_);
    lv_label_set_text(status_label_, "Scanning Files");
    lv_obj_center(status_label_);
    ui_notify::showSymbol(LV_SYMBOL_REFRESH, "Scanning Files", 1000);
    if (!scanning_) {
        lv_label_set_text(status_label_, "Failed to scan files");
        scan_done_ = true;
    }

    kb_register_app_keys(this, {
        { ';', [this](){ return viewer_ && !lv_obj_has_flag(viewer_, LV_OBJ_FLAG_HIDDEN); }, [this](){ pan_step(0, -12); } },
        { '.', [this](){ return viewer_ && !lv_obj_has_flag(viewer_, LV_OBJ_FLAG_HIDDEN); }, [this](){ pan_step(0, 12); } },
        { ',', [this](){ return viewer_ && !lv_obj_has_flag(viewer_, LV_OBJ_FLAG_HIDDEN); }, [this](){ pan_step(-12, 0); } },
        { '/', [this](){ return viewer_ && !lv_obj_has_flag(viewer_, LV_OBJ_FLAG_HIDDEN); }, [this](){ pan_step(12, 0); } },
        { '-', [this](){ return viewer_ && !lv_obj_has_flag(viewer_, LV_OBJ_FLAG_HIDDEN); }, [this](){ zoom_step(-32); } },
        { '=', [this](){ return viewer_ && !lv_obj_has_flag(viewer_, LV_OBJ_FLAG_HIDDEN); }, [this](){ zoom_step(32); } },
    });
}

void AppPictures::onTick() {
    if (!scanning_) return;
    const int kBatch = 8;
    FileInfo tmp[kBatch];
    int cnt = 0;
    bool cont = fm_.stepScan(tmp, cnt, kBatch);
    for (int i = 0; i < cnt; ++i) {
        if (tmp[i].name.startsWith("i_") && !tmp[i].name.startsWith("iec_")) {
            files_.push_back(tmp[i]);
        }
    }
    if (!cont) {
        scanning_ = false;
        scan_done_ = true;
        if (status_label_) {
            lv_obj_del(status_label_);
            status_label_ = nullptr;
        }
        build_list();
        build_viewer();
        build_password_view();
        lv_obj_add_flag(viewer_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(password_view_, LV_OBJ_FLAG_HIDDEN);
        rebuildFocusGroup();
    }
}

static bool app_pictures_is_focusable(lv_obj_t* obj) {
    if (!lv_obj_is_valid(obj)) return false;
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) return false;
    if (lv_obj_has_class(obj, &lv_btn_class)) return true;
    if (lv_obj_has_class(obj, &lv_textarea_class)) return true;
    if (lv_obj_has_class(obj, &lv_dropdown_class)) return true;
    if (lv_obj_has_class(obj, &lv_checkbox_class)) return true;
    if (lv_obj_has_class(obj, &lv_slider_class)) return true;
    if (lv_obj_has_class(obj, &lv_switch_class)) return true;
    if (lv_obj_has_class(obj, &lv_spinbox_class)) return true;
    if (lv_obj_has_class(obj, &lv_roller_class)) return true;
    if (lv_obj_has_class(obj, &lv_img_class) && lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_1)) return true;
    if (lv_obj_is_editable(obj)) return true;
    return false;
}

static void app_pictures_add_focusables_recursive(lv_obj_t* node, lv_group_t* group) {
    if (!node || !group) return;
    if (app_pictures_is_focusable(node)) lv_group_add_obj(group, node);
    uint32_t child_cnt = lv_obj_get_child_cnt(node);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(node, i);
        app_pictures_add_focusables_recursive(child, group);
    }
}

static lv_obj_t* app_pictures_find_first_focusable(lv_obj_t* node) {
    if (!node) return nullptr;
    if (app_pictures_is_focusable(node)) return node;
    uint32_t child_cnt = lv_obj_get_child_cnt(node);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(node, i);
        lv_obj_t* res = app_pictures_find_first_focusable(child);
        if (res) return res;
    }
    return nullptr;
}

void AppPictures::rebuildFocusGroup() {
    lv_group_t* grp = kb_get_current_group();
    if (!grp) return;
    lv_group_remove_all_objs(grp);
    if (root_) app_pictures_add_focusables_recursive(root_, grp);
    lv_obj_t* focus = nullptr;
    if (password_view_ && !lv_obj_has_flag(password_view_, LV_OBJ_FLAG_HIDDEN) && password_ta_) {
        focus = password_ta_;
    } else if (viewer_ && !lv_obj_has_flag(viewer_, LV_OBJ_FLAG_HIDDEN) && back_btn_) {
        focus = back_btn_;
    } else if (list_ && !lv_obj_has_flag(list_, LV_OBJ_FLAG_HIDDEN)) {
        uint32_t cnt = lv_obj_get_child_cnt(list_);
        for (uint32_t i = 0; i < cnt; ++i) {
            lv_obj_t* c = lv_obj_get_child(list_, i);
            if (app_pictures_is_focusable(c)) {
                focus = c;
                break;
            }
        }
    }
    if (!focus && root_) focus = app_pictures_find_first_focusable(root_);
    if (focus) lv_group_focus_obj(focus);
}

void AppPictures::build_list() {
    list_ = lv_list_create(root_);
    lv_obj_set_size(list_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(list_, 1);
    lv_obj_add_flag(list_, LV_OBJ_FLAG_SCROLLABLE);
    ui_theme::apply_list_menu(list_);

    // Add Decrypt Image button
    decrypt_btn_ = lv_list_add_btn(list_, LV_SYMBOL_EYE_CLOSE, "Decrypt Image");
    ui_theme::apply_list_menu_item(decrypt_btn_);
    lv_obj_add_event_cb(decrypt_btn_, on_decrypt_clicked, LV_EVENT_CLICKED, this);

    item_index_.clear();
    for (size_t i = 0; i < files_.size(); ++i) {
        const auto& f = files_[i];
        lv_obj_t* btn = lv_list_add_btn(list_, LV_SYMBOL_IMAGE, f.name.c_str());
        ui_theme::apply_list_menu_item(btn);
        lv_obj_add_event_cb(btn, on_item_clicked, LV_EVENT_CLICKED, this);
        item_index_[btn] = (int)i;
    }
}

void AppPictures::build_viewer() {
    viewer_ = lv_obj_create(root_);
    lv_obj_set_size(viewer_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(viewer_, 1);
    lv_obj_set_style_bg_opa(viewer_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(viewer_, 0, 0);
    lv_obj_set_style_radius(viewer_, 0, 0);
    lv_obj_set_flex_flow(viewer_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(viewer_, 0, 0);
    lv_obj_set_style_pad_row(viewer_, 0, 0);
    lv_obj_set_scroll_dir(viewer_, LV_DIR_NONE);
    lv_obj_clear_flag(viewer_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(viewer_, LV_SCROLLBAR_MODE_OFF);

    img_area_ = lv_obj_create(viewer_);
    lv_obj_set_size(img_area_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(img_area_, 1);
    lv_obj_set_style_bg_opa(img_area_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(img_area_, 0, 0);
    lv_obj_set_style_radius(img_area_, 0, 0);
    lv_obj_set_scroll_dir(img_area_, LV_DIR_NONE);
    lv_obj_clear_flag(img_area_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(img_area_, LV_SCROLLBAR_MODE_OFF);

    back_btn_ = lv_btn_create(viewer_);
    lv_obj_set_size(back_btn_, 24, 24);
    lv_obj_set_style_radius(back_btn_, 12, 0);
    lv_obj_set_style_bg_opa(back_btn_, LV_OPA_80, 0);
    lv_obj_set_style_bg_color(back_btn_, lv_color_hex(0xDDDDDD), 0);
    lv_obj_set_style_border_width(back_btn_, 0, 0);
    lv_obj_set_style_shadow_opa(back_btn_, LV_OPA_TRANSP, 0);
    lv_obj_set_pos(back_btn_, 4, 4);
    lv_obj_add_flag(back_btn_, LV_OBJ_FLAG_FLOATING);
    lv_obj_move_foreground(back_btn_);
    lv_obj_t* lbl = lv_label_create(back_btn_);
    lv_label_set_text(lbl, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x000000), 0);
    lv_obj_center(lbl);
    lv_obj_add_flag(back_btn_, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_event_cb(back_btn_, on_back_clicked, LV_EVENT_CLICKED, this);

    img_ = lv_img_create(img_area_);
    info_label_ = lv_label_create(img_area_);
    lv_obj_set_style_text_color(info_label_, lv_color_hex(0xEEEEEE), 0);
    lv_obj_set_style_text_font(info_label_, ui_theme::get_system_font(), 0);
    lv_obj_add_flag(info_label_, LV_OBJ_FLAG_HIDDEN);

    toolbar_ = lv_obj_create(viewer_);
    lv_obj_set_size(toolbar_, lv_pct(100), 28);
    lv_obj_set_style_bg_opa(toolbar_, LV_OPA_60, 0);
    lv_obj_set_style_bg_color(toolbar_, lv_color_hex(0x202020), 0);
    lv_obj_set_style_border_width(toolbar_, 0, 0);
    lv_obj_set_style_radius(toolbar_, 0, 0);
    lv_obj_set_style_pad_all(toolbar_, 2, 0);
    lv_obj_set_style_pad_column(toolbar_, 4, 0);
    lv_obj_set_flex_flow(toolbar_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(toolbar_, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(toolbar_, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(toolbar_, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_move_foreground(toolbar_);
    lv_obj_move_foreground(back_btn_);

    btn_zoom_in_ = lv_btn_create(toolbar_);
    ui_theme::apply_button(btn_zoom_in_);
    lv_obj_set_size(btn_zoom_in_, 24, 24);
    lv_obj_t* zi_lbl = lv_label_create(btn_zoom_in_);
    lv_label_set_text(zi_lbl, LV_SYMBOL_PLUS);
    lv_obj_center(zi_lbl);
    lv_obj_add_event_cb(btn_zoom_in_, on_zoom_in_clicked, LV_EVENT_CLICKED, this);

    btn_zoom_out_ = lv_btn_create(toolbar_);
    ui_theme::apply_button(btn_zoom_out_);
    lv_obj_set_size(btn_zoom_out_, 24, 24);
    lv_obj_t* zo_lbl = lv_label_create(btn_zoom_out_);
    lv_label_set_text(zo_lbl, LV_SYMBOL_MINUS);
    lv_obj_center(zo_lbl);
    lv_obj_add_event_cb(btn_zoom_out_, on_zoom_out_clicked, LV_EVENT_CLICKED, this);

    btn_fit_ = lv_btn_create(toolbar_);
    ui_theme::apply_button(btn_fit_);
    lv_obj_set_size(btn_fit_, 24, 24);
    lv_obj_t* ft_lbl = lv_label_create(btn_fit_);
    lv_label_set_text(ft_lbl, LV_SYMBOL_REFRESH);
    lv_obj_center(ft_lbl);
    lv_obj_add_event_cb(btn_fit_, on_fit_clicked, LV_EVENT_CLICKED, this);

    btn_up_ = lv_btn_create(toolbar_);
    ui_theme::apply_button(btn_up_);
    lv_obj_set_size(btn_up_, 24, 24);
    lv_obj_t* up_lbl = lv_label_create(btn_up_);
    lv_label_set_text(up_lbl, LV_SYMBOL_UP);
    lv_obj_center(up_lbl);
    lv_obj_add_event_cb(btn_up_, on_pan_up_clicked, LV_EVENT_CLICKED, this);

    btn_down_ = lv_btn_create(toolbar_);
    ui_theme::apply_button(btn_down_);
    lv_obj_set_size(btn_down_, 24, 24);
    lv_obj_t* dn_lbl = lv_label_create(btn_down_);
    lv_label_set_text(dn_lbl, LV_SYMBOL_DOWN);
    lv_obj_center(dn_lbl);
    lv_obj_add_event_cb(btn_down_, on_pan_down_clicked, LV_EVENT_CLICKED, this);

    btn_left_ = lv_btn_create(toolbar_);
    ui_theme::apply_button(btn_left_);
    lv_obj_set_size(btn_left_, 24, 24);
    lv_obj_t* lf_lbl = lv_label_create(btn_left_);
    lv_label_set_text(lf_lbl, LV_SYMBOL_LEFT);
    lv_obj_center(lf_lbl);
    lv_obj_add_event_cb(btn_left_, on_pan_left_clicked, LV_EVENT_CLICKED, this);

    btn_right_ = lv_btn_create(toolbar_);
    ui_theme::apply_button(btn_right_);
    lv_obj_set_size(btn_right_, 24, 24);
    lv_obj_t* rt_lbl = lv_label_create(btn_right_);
    lv_label_set_text(rt_lbl, LV_SYMBOL_RIGHT);
    lv_obj_center(rt_lbl);
    lv_obj_add_event_cb(btn_right_, on_pan_right_clicked, LV_EVENT_CLICKED, this);
}

void AppPictures::show_image(int idx) {
    if (idx < 0 || idx >= (int)files_.size()) return;
    current_index_ = idx;
    lv_obj_add_flag(list_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(viewer_, LV_OBJ_FLAG_HIDDEN);
    rebuildFocusGroup();

    String p = files_[idx].path;
    if (!p.startsWith("/")) p = String("/") + p;
    File f = SD.open(p.c_str(), FILE_READ);
    if (!f) {
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "无法打开图片文件");
        lv_obj_center(info_label_);
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "无法打开图片文件", 2500);
        return;
    }
    size_t n = f.size();
    if (n < 4) {
        f.close();
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "图片格式错误");
        lv_obj_center(info_label_);
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "图片格式错误", 2500);
        return;
    }
    uint8_t hdr[4];
    size_t rd = f.read(hdr, 4);
    if (rd != 4) {
        f.close();
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "读取图片头失败");
        lv_obj_center(info_label_);
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "读取图片头失败", 2500);
        return;
    }
    uint32_t h32 = (uint32_t)hdr[0] | ((uint32_t)hdr[1] << 8) | ((uint32_t)hdr[2] << 16) | ((uint32_t)hdr[3] << 24);
    uint8_t cf = (uint8_t)(h32 & 0x1F);
    uint16_t w = (uint16_t)((h32 >> 10) & 0x7FF);
    uint16_t h = (uint16_t)((h32 >> 21) & 0x7FF);
    if (w == 0 || h == 0) {
        f.close();
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "图片尺寸无效");
        lv_obj_center(info_label_);
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "图片尺寸无效", 2500);
        return;
    }
    if (cf != LV_IMG_CF_TRUE_COLOR) {
        // 仅支持转换器生成的 TRUE_COLOR (RGB565) 二进制
        f.close();
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "当前仅支持RGB565 TRUE_COLOR");
        lv_obj_center(info_label_);
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "仅支持RGB565 TRUE_COLOR", 2500);
        return;
    }
    size_t data_sz = (size_t)w * (size_t)h * 2;
    if (n != data_sz + 4) {
        f.close();
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "图片数据长度不匹配");
        lv_obj_center(info_label_);
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "图片数据长度不匹配", 2500);
        return;
    }
    img_dsc_.header.always_zero = 0;
    img_dsc_.header.w = w;
    img_dsc_.header.h = h;
    img_dsc_.header.cf = LV_IMG_CF_TRUE_COLOR;
    img_dsc_.data_size = data_sz;
    if (img_buf_) { free(img_buf_); img_buf_ = nullptr; }
    img_buf_ = (uint8_t*)malloc(img_dsc_.data_size);
    if (!img_buf_) {
        f.close();
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "内存不足，无法显示图片");
        lv_obj_center(info_label_);
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "内存不足，无法显示图片", 2500);
        return;
    }
    size_t rimg = f.read(img_buf_, img_dsc_.data_size);
    f.close();
    if (rimg != img_dsc_.data_size) {
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "读取图片数据失败");
        lv_obj_center(info_label_);
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "读取图片数据失败", 2500);
        return;
    }
    img_dsc_.data = (const uint8_t*)img_buf_;

    lv_obj_add_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(img_, LV_OBJ_FLAG_HIDDEN);
    lv_img_set_src(img_, &img_dsc_);

    lv_coord_t vw = lv_obj_get_width(img_area_);
    lv_coord_t vh = lv_obj_get_height(img_area_);
    lv_coord_t iw = img_dsc_.header.w;
    lv_coord_t ih = img_dsc_.header.h;
    if (iw > 0 && ih > 0 && vw > 0 && vh > 0) {
        float rw = (float)vw / (float)iw;
        float rh = (float)vh / (float)ih;
        float r = rw < rh ? rw : rh;
        if (r < 1.0f) {
            zoom_ = (uint16_t)(r * 256.0f);
            if (zoom_ < 1) zoom_ = 1;
        } else {
            zoom_ = 256;
        }
        lv_img_set_zoom(img_, zoom_);
    }
    pan_x_ = 0;
    pan_y_ = 0;
    lv_obj_align(img_, LV_ALIGN_CENTER, 0, 0);
}

void AppPictures::onClose() {
    if (img_buf_) { free(img_buf_); img_buf_ = nullptr; }
    kb_clear_app_keys(this);
}

void AppPictures::on_item_clicked(lv_event_t* e) {
    auto* app = static_cast<AppPictures*>(lv_event_get_user_data(e));
    if (!app) return;
    lv_obj_t* tgt = lv_event_get_target(e);
    auto it = app->item_index_.find(tgt);
    if (it == app->item_index_.end()) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) app->show_image(it->second);
}

void AppPictures::on_back_clicked(lv_event_t* e) {
    auto* app = static_cast<AppPictures*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_add_flag(app->viewer_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(app->list_, LV_OBJ_FLAG_HIDDEN);
        app->current_index_ = -1;
        app->rebuildFocusGroup();
    }
}

void AppPictures::fit_to_window() {
    lv_coord_t vw = lv_obj_get_width(img_area_);
    lv_coord_t vh = lv_obj_get_height(img_area_);
    lv_coord_t iw = img_dsc_.header.w;
    lv_coord_t ih = img_dsc_.header.h;
    if (iw <= 0 || ih <= 0 || vw <= 0 || vh <= 0) return;
    float rw = (float)vw / (float)iw;
    float rh = (float)vh / (float)ih;
    float r = rw < rh ? rw : rh;
    if (r < 1.0f) {
        zoom_ = (uint16_t)(r * 256.0f);
        if (zoom_ < 1) zoom_ = 1;
    } else {
        zoom_ = 256;
    }
    lv_img_set_zoom(img_, zoom_);
    pan_x_ = 0;
    pan_y_ = 0;
    lv_obj_align(img_, LV_ALIGN_CENTER, 0, 0);
}

void AppPictures::zoom_step(int delta) {
    int z = (int)zoom_ + delta;
    if (z < 16) z = 16;
    if (z > 1024) z = 1024;
    zoom_ = (uint16_t)z;
    lv_img_set_zoom(img_, zoom_);
}

void AppPictures::pan_step(int dx, int dy) {
    pan_x_ += dx;
    pan_y_ += dy;
    lv_obj_align(img_, LV_ALIGN_CENTER, pan_x_, pan_y_);
}

void AppPictures::on_zoom_in_clicked(lv_event_t* e) {
    auto* app = static_cast<AppPictures*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) app->zoom_step(32);
}

void AppPictures::on_zoom_out_clicked(lv_event_t* e) {
    auto* app = static_cast<AppPictures*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) app->zoom_step(-32);
}

void AppPictures::on_fit_clicked(lv_event_t* e) {
    auto* app = static_cast<AppPictures*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) app->fit_to_window();
}

void AppPictures::on_pan_up_clicked(lv_event_t* e) {
    auto* app = static_cast<AppPictures*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) app->pan_step(0, -12);
}

void AppPictures::on_pan_down_clicked(lv_event_t* e) {
    auto* app = static_cast<AppPictures*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) app->pan_step(0, 12);
}

void AppPictures::on_pan_left_clicked(lv_event_t* e) {
    auto* app = static_cast<AppPictures*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) app->pan_step(-12, 0);
}

void AppPictures::on_pan_right_clicked(lv_event_t* e) {
    auto* app = static_cast<AppPictures*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) app->pan_step(12, 0);
}

void AppPictures::build_password_view() {
    password_view_ = lv_obj_create(root_);
    lv_obj_set_size(password_view_, LV_SIZE_CONTENT, lv_pct(100));
    lv_obj_set_flex_grow(password_view_, 1);
    lv_obj_set_flex_flow(password_view_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(password_view_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_center(password_view_);
    lv_obj_set_style_max_width(password_view_, 200, 0);
    lv_obj_set_style_pad_all(password_view_, 2, 0);
    lv_obj_set_style_pad_row(password_view_, 4, 0);

    password_ta_ = lv_textarea_create(password_view_);
    lv_textarea_set_one_line(password_ta_, true);
    lv_textarea_set_password_mode(password_ta_, true);
    lv_textarea_set_placeholder_text(password_ta_, "Password");
    lv_obj_set_width(password_ta_, 180);
    
    lv_obj_t* btn_cont = lv_obj_create(password_view_);
    lv_obj_set_size(btn_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(btn_cont, 10, 0);
    lv_obj_set_style_bg_opa(btn_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_cont, 0, 0);
    lv_obj_set_style_pad_all(btn_cont, 0, 0);

    lv_obj_t* btn_ok = lv_btn_create(btn_cont);
    ui_theme::apply_button(btn_ok);
    lv_obj_set_height(btn_ok, 28);
    lv_obj_set_width(btn_ok, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_hor(btn_ok, 6, 0);
    lv_obj_t* lbl_ok = lv_label_create(btn_ok);
    lv_label_set_text(lbl_ok, LV_SYMBOL_OK " OK");
    lv_obj_center(lbl_ok);
    lv_obj_add_event_cb(btn_ok, on_password_submit, LV_EVENT_CLICKED, this);

    lv_obj_t* btn_cancel = lv_btn_create(btn_cont);
    ui_theme::apply_button(btn_cancel);
    lv_obj_set_height(btn_cancel, 28);
    lv_obj_set_width(btn_cancel, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_hor(btn_cancel, 6, 0);
    lv_obj_t* lbl_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(lbl_cancel, LV_SYMBOL_CLOSE " Cancel");
    lv_obj_center(lbl_cancel);
    lv_obj_add_event_cb(btn_cancel, on_password_cancel, LV_EVENT_CLICKED, this);
}

void AppPictures::on_decrypt_clicked(lv_event_t* e) {
    auto* app = static_cast<AppPictures*>(lv_event_get_user_data(e));
    if (!app) return;
    lv_obj_add_flag(app->list_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(app->password_view_, LV_OBJ_FLAG_HIDDEN);
    lv_textarea_set_text(app->password_ta_, "");
    app->rebuildFocusGroup();
}

void AppPictures::on_password_submit(lv_event_t* e) {
    auto* app = static_cast<AppPictures*>(lv_event_get_user_data(e));
    if (!app) return;
    const char* pwd = lv_textarea_get_text(app->password_ta_);
    app->check_and_load_encrypted(pwd);
}

void AppPictures::on_password_cancel(lv_event_t* e) {
    auto* app = static_cast<AppPictures*>(lv_event_get_user_data(e));
    if (!app) return;
    lv_obj_add_flag(app->password_view_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(app->list_, LV_OBJ_FLAG_HIDDEN);
}

void AppPictures::check_and_load_encrypted(const char* password) {
    MD5Builder md5;
    md5.begin();
    md5.add(String(password));
    md5.calculate();
    String hash = md5.toString();
    hash.toUpperCase();
    String short_hash = hash.substring(8, 24);
    String filename = "iec_" + short_hash + ".bin";
    String path = "/" + filename;

    if (SD.exists(path)) {
        lv_obj_add_flag(password_view_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(viewer_, LV_OBJ_FLAG_HIDDEN);
        show_encrypted_image(path);
    } else {
        static const char* btns[] = {"OK", ""};
        lv_obj_t* mbox = lv_msgbox_create(root_, "Error", "Incorrect Password or File Not Found", btns, true);
        lv_obj_center(mbox);
        lv_obj_add_event_cb(mbox, [](lv_event_t* e) {
            lv_msgbox_close(lv_event_get_current_target(e));
        }, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

void AppPictures::show_encrypted_image(const String& path) {
    current_index_ = -1;
    
    String p = path;
    if (!p.startsWith("/")) p = String("/") + p;
    File f = SD.open(p.c_str(), FILE_READ);
    if (!f) {
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "无法打开图片文件");
        lv_obj_center(info_label_);
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "无法打开图片文件", 2500);
        return;
    }
    size_t n = f.size();
    if (n < 4) {
        f.close();
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "图片格式错误");
        lv_obj_center(info_label_);
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "图片格式错误", 2500);
        return;
    }
    uint8_t hdr[4];
    size_t rd = f.read(hdr, 4);
    if (rd != 4) {
        f.close();
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "读取图片头失败");
        lv_obj_center(info_label_);
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "读取图片头失败", 2500);
        return;
    }
    uint32_t h32 = (uint32_t)hdr[0] | ((uint32_t)hdr[1] << 8) | ((uint32_t)hdr[2] << 16) | ((uint32_t)hdr[3] << 24);
    uint8_t cf = (uint8_t)(h32 & 0x1F);
    uint16_t w = (uint16_t)((h32 >> 10) & 0x7FF);
    uint16_t h = (uint16_t)((h32 >> 21) & 0x7FF);
    if (w == 0 || h == 0) {
        f.close();
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "图片尺寸无效");
        lv_obj_center(info_label_);
        return;
    }
    if (cf != LV_IMG_CF_TRUE_COLOR) {
        f.close();
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "当前仅支持RGB565 TRUE_COLOR");
        lv_obj_center(info_label_);
        return;
    }
    size_t data_sz = (size_t)w * (size_t)h * 2;
    if (n != data_sz + 4) {
        f.close();
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "图片数据长度不匹配");
        lv_obj_center(info_label_);
        return;
    }
    img_dsc_.header.always_zero = 0;
    img_dsc_.header.w = w;
    img_dsc_.header.h = h;
    img_dsc_.header.cf = LV_IMG_CF_TRUE_COLOR;
    img_dsc_.data_size = data_sz;
    if (img_buf_) { free(img_buf_); img_buf_ = nullptr; }
    img_buf_ = (uint8_t*)malloc(img_dsc_.data_size);
    if (!img_buf_) {
        f.close();
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "内存不足，无法显示图片");
        lv_obj_center(info_label_);
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "内存不足，无法显示图片", 2500);
        return;
    }
    size_t rimg = f.read(img_buf_, img_dsc_.data_size);
    f.close();
    if (rimg != img_dsc_.data_size) {
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "读取图片数据失败");
        lv_obj_center(info_label_);
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "读取图片数据失败", 2500);
        return;
    }
    img_dsc_.data = (const uint8_t*)img_buf_;

    lv_obj_add_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(img_, LV_OBJ_FLAG_HIDDEN);
    lv_img_set_src(img_, &img_dsc_);

    lv_coord_t vw = lv_obj_get_width(img_area_);
    lv_coord_t vh = lv_obj_get_height(img_area_);
    lv_coord_t iw = img_dsc_.header.w;
    lv_coord_t ih = img_dsc_.header.h;
    if (iw > 0 && ih > 0 && vw > 0 && vh > 0) {
        float rw = (float)vw / (float)iw;
        float rh = (float)vh / (float)ih;
        float r = rw < rh ? rw : rh;
        if (r < 1.0f) {
            zoom_ = (uint16_t)(r * 256.0f);
            if (zoom_ < 1) zoom_ = 1;
        } else {
            zoom_ = 256;
        }
        lv_img_set_zoom(img_, zoom_);
    }
    pan_x_ = 0;
    pan_y_ = 0;
    lv_obj_align(img_, LV_ALIGN_CENTER, 0, 0);
}
