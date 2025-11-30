#pragma once
#include "window_system.h"
#include "SDFileManager.h"
#include "ux_runtime.h"

class AppUXEditor : public IApp {
public:
    AppUXEditor() {}
    const char* title() const override { return "UX Editor"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Dark; }
    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override {}
    void refresh_list();
    void load_selected();
    void create_new();
    void save_current();
    void delete_current();
    void run_preview();
private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* toolbar_ = nullptr;
    lv_obj_t* dropdown_ = nullptr;
    lv_obj_t* btn_new_ = nullptr;
    lv_obj_t* btn_save_ = nullptr;
    lv_obj_t* btn_delete_ = nullptr;
    lv_obj_t* btn_run_ = nullptr;
    lv_obj_t* editor_ = nullptr;
    lv_obj_t* preview_ = nullptr;
    SDFileManager fm_;
    String current_path_;
};
