#pragma once
#include "window_system.h"
#include "theme.h"
#include "SDFileManager.h"
#include <vector>
#include <map>

class AppPictures : public IApp {
public:
    AppPictures();
    ~AppPictures() override;

    const char* title() const override { return "Pictures"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Dark; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override;

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* list_ = nullptr;
    lv_obj_t* viewer_ = nullptr;
    lv_obj_t* img_area_ = nullptr;
    lv_obj_t* img_ = nullptr;
    lv_obj_t* back_btn_ = nullptr;
    lv_obj_t* info_label_ = nullptr;
    lv_obj_t* toolbar_ = nullptr;
    lv_obj_t* btn_zoom_in_ = nullptr;
    lv_obj_t* btn_zoom_out_ = nullptr;
    lv_obj_t* btn_fit_ = nullptr;
    lv_obj_t* btn_up_ = nullptr;
    lv_obj_t* btn_down_ = nullptr;
    lv_obj_t* btn_left_ = nullptr;
    lv_obj_t* btn_right_ = nullptr;
    lv_img_dsc_t img_dsc_{};
    uint8_t* img_buf_ = nullptr;
    uint16_t zoom_ = 256;
    lv_coord_t pan_x_ = 0;
    lv_coord_t pan_y_ = 0;
    
    lv_obj_t* password_view_ = nullptr;
    lv_obj_t* password_ta_ = nullptr;
    
    std::vector<FileInfo> files_;
    std::map<lv_obj_t*, int> item_index_;
    SDFileManager fm_;
    int current_index_ = -1;
    lv_obj_t* decrypt_btn_ = nullptr;

    void build_list();
    void build_viewer();
    void build_password_view();
    void show_image(int idx);
    void check_and_load_encrypted(const char* password);
    void show_encrypted_image(const String& path);
    void fit_to_window();
    void zoom_step(int delta);
    void pan_step(int dx, int dy);
    static void on_item_clicked(lv_event_t* e);
    static void on_back_clicked(lv_event_t* e);
    static void on_zoom_in_clicked(lv_event_t* e);
    static void on_zoom_out_clicked(lv_event_t* e);
    static void on_fit_clicked(lv_event_t* e);
    static void on_pan_up_clicked(lv_event_t* e);
    static void on_pan_down_clicked(lv_event_t* e);
    static void on_pan_left_clicked(lv_event_t* e);
    static void on_pan_right_clicked(lv_event_t* e);
    static void on_decrypt_clicked(lv_event_t* e);
    static void on_password_submit(lv_event_t* e);
    static void on_password_cancel(lv_event_t* e);
};
