#pragma once
#include "window_system.h"
#include "theme.h"

class AppRadioSim : public IApp {
public:
    AppRadioSim();
    ~AppRadioSim() override;

    const char* title() const override { return ""; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Light; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override;

private:
    lv_obj_t* root_ = nullptr;
};
