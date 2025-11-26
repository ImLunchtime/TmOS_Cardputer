#include "app_pictures.h"
#include <lvgl.h>
#include <M5Cardputer.h>
#include <cstring>

AppPictures::AppPictures() {}
AppPictures::~AppPictures() {}

void AppPictures::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 6, 0);
    lv_obj_set_style_pad_row(root_, 6, 0);
    lv_obj_set_style_text_font(root_, ui_theme::get_system_font(), 0);

    fm_.initialize();

    files_.clear();
    const int kMax = 256;
    std::vector<FileInfo> tmp(kMax);
    int cnt = 0;
    fm_.scanAllFiles(tmp.data(), cnt, kMax, ".bin");
    for (int i = 0; i < cnt; ++i) {
        if (tmp[i].name.startsWith("i_")) files_.push_back(tmp[i]);
    }

    build_list();
    build_viewer();
    lv_obj_add_flag(viewer_, LV_OBJ_FLAG_HIDDEN);
}

void AppPictures::build_list() {
    list_ = lv_list_create(root_);
    lv_obj_set_size(list_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(list_, 1);
    ui_theme::apply_list_menu(list_);

    item_index_.clear();
    for (size_t i = 0; i < files_.size(); ++i) {
        const auto& f = files_[i];
        lv_obj_t* item = lv_btn_create(list_);
        ui_theme::apply_list_menu_item(item);
        lv_obj_t* label = lv_label_create(item);
        lv_label_set_text(label, f.name.c_str());
        lv_obj_center(label);
        lv_obj_add_event_cb(item, on_item_clicked, LV_EVENT_CLICKED, this);
        item_index_[item] = (int)i;
    }
}

void AppPictures::build_viewer() {
    viewer_ = lv_obj_create(root_);
    lv_obj_set_size(viewer_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(viewer_, 1);
    lv_obj_set_style_bg_opa(viewer_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(viewer_, 0, 0);
    lv_obj_set_style_radius(viewer_, 0, 0);

    back_btn_ = lv_btn_create(viewer_);
    ui_theme::apply_button(back_btn_);
    lv_obj_set_pos(back_btn_, 4, 4);
    lv_obj_t* lbl = lv_label_create(back_btn_);
    lv_label_set_text(lbl, "Back");
    lv_obj_center(lbl);
    lv_obj_add_event_cb(back_btn_, on_back_clicked, LV_EVENT_CLICKED, this);

    img_ = lv_img_create(viewer_);
    info_label_ = lv_label_create(viewer_);
    lv_obj_set_style_text_color(info_label_, lv_color_hex(0xEEEEEE), 0);
    lv_obj_set_style_text_font(info_label_, ui_theme::get_system_font(), 0);
    lv_obj_add_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
}

void AppPictures::show_image(int idx) {
    if (idx < 0 || idx >= (int)files_.size()) return;
    current_index_ = idx;
    lv_obj_add_flag(list_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(viewer_, LV_OBJ_FLAG_HIDDEN);

    String p = files_[idx].path;
    if (!p.startsWith("/")) p = String("/") + p;
    File f = SD.open(p.c_str(), FILE_READ);
    if (!f) {
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "无法打开图片文件");
        lv_obj_center(info_label_);
        return;
    }
    size_t n = f.size();
    if (n < 4) {
        f.close();
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "图片格式错误");
        lv_obj_center(info_label_);
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
        // 仅支持转换器生成的 TRUE_COLOR (RGB565) 二进制
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
        return;
    }
    size_t rimg = f.read(img_buf_, img_dsc_.data_size);
    f.close();
    if (rimg != img_dsc_.data_size) {
        lv_obj_add_flag(img_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(info_label_, "读取图片数据失败");
        lv_obj_center(info_label_);
        return;
    }
    img_dsc_.data = (const uint8_t*)img_buf_;

    lv_obj_add_flag(info_label_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(img_, LV_OBJ_FLAG_HIDDEN);
    lv_img_set_src(img_, &img_dsc_);

    lv_coord_t vw = lv_obj_get_width(viewer_);
    lv_coord_t vh = lv_obj_get_height(viewer_);
    lv_coord_t iw = lv_obj_get_width(img_);
    lv_coord_t ih = lv_obj_get_height(img_);
    if (iw > 0 && ih > 0 && vw > 0 && vh > 0) {
        float rw = (float)vw / (float)iw;
        float rh = (float)vh / (float)ih;
        float r = rw < rh ? rw : rh;
        if (r < 1.0f) {
            uint16_t zoom = (uint16_t)(r * 256.0f);
            if (zoom < 1) zoom = 1;
            lv_img_set_zoom(img_, zoom);
        } else {
            lv_img_set_zoom(img_, 256);
        }
    }
    lv_obj_center(img_);
}

void AppPictures::onClose() {
    if (img_buf_) { free(img_buf_); img_buf_ = nullptr; }
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
    }
}
