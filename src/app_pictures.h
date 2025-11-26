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
    lv_obj_t* img_ = nullptr;
    lv_obj_t* back_btn_ = nullptr;
    lv_obj_t* info_label_ = nullptr;
    lv_img_dsc_t img_dsc_{};
    uint8_t* img_buf_ = nullptr;
    
    std::vector<FileInfo> files_;
    std::map<lv_obj_t*, int> item_index_;
    SDFileManager fm_;
    int current_index_ = -1;

    void build_list();
    void build_viewer();
    void show_image(int idx);
    static void on_item_clicked(lv_event_t* e);
    static void on_back_clicked(lv_event_t* e);
};
