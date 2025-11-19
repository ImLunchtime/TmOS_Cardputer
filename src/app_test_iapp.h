#pragma once
#include "window_system.h"
#include "theme.h"

class AppTest : public IApp {
public:
    AppTest();
    ~AppTest() override;

    const char* title() const override { return "Test"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Light; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override;

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* btn_ = nullptr;
    lv_obj_t* label_ = nullptr;
    static void on_btn_clicked(lv_event_t* e);
};