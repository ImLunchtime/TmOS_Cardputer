#pragma once
#include "core/window_system.h"
#include "storage/SDFileManager.h"
#include "ux/ux_runtime.h"
#include <vector>
#include <unordered_map>

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
    void debug_current();
    void show_edit_page();
    void show_list_page();
    void on_list_item_clicked(lv_obj_t* item);
private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* toolbar_ = nullptr;
    lv_obj_t* page_list_ = nullptr;
    lv_obj_t* list_ = nullptr;
    lv_obj_t* fab_new_ = nullptr;
    lv_obj_t* page_edit_ = nullptr;
    lv_obj_t* btn_new_ = nullptr;
    lv_obj_t* btn_save_ = nullptr;
    lv_obj_t* btn_delete_ = nullptr;
    lv_obj_t* btn_debug_ = nullptr;
    lv_obj_t* editor_ = nullptr;
    lv_obj_t* status_label_ = nullptr;
    SDFileManager fm_;
    String current_path_;
    std::vector<FileInfo> files_cache_;
    std::unordered_map<lv_obj_t*, int> item_index_;
};
