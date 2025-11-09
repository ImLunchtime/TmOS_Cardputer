#pragma once
#include "window_system.h"

class AppThemeCenter : public IApp {
public:
    AppThemeCenter();
    ~AppThemeCenter() override;

    const char* title() const override { return "Theme Center"; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override;

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* info_label_ = nullptr;
    lv_obj_t* close_btn_ = nullptr;

    void buildUI(lv_obj_t* parent);
    static void on_close_btn_event(lv_event_t* e);
};