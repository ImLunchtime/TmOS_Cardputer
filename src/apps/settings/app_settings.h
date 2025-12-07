#pragma once
#include "window_system.h"
#include "theme.h"

class AppSettings : public IApp {
public:
    AppSettings();
    ~AppSettings() override;

    const char* title() const override { return "Settings"; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override;

    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Light; }

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* info_label_ = nullptr;
    lv_obj_t* close_btn_ = nullptr;

    void buildUI(lv_obj_t* parent);
    static void on_close_btn_event(lv_event_t* e);
};