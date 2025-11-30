#pragma once
#include "window_system.h"
#include "theme.h"

class AppCircuitSim : public IApp {
public:
    AppCircuitSim();
    ~AppCircuitSim() override;

    const char* title() const override { return "CircuitSim"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Light; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override;

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* label_ = nullptr;
};
