#pragma once
#include "core/window_system.h"
#include "ui/theme.h"

class AppCalculator : public IApp {
public:
    AppCalculator();
    ~AppCalculator() override;

    const char* title() const override { return "Calculator"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Light; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override;

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* ta_history_ = nullptr;
    lv_obj_t* ta_input_ = nullptr;
    lv_obj_t* btn_calc_ = nullptr;

    static void on_btn_clicked(lv_event_t* e);
    static void on_input_ready(lv_event_t* e);
    void calculate();
};
