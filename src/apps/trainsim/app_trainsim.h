#pragma once
#include "window_system.h"
#include "theme.h"

class AppTrainSim : public IApp {
public:
    AppTrainSim();
    ~AppTrainSim() override;

    const char* title() const override { return nullptr; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Dark; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override;

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* speed_gauge_ = nullptr;
    lv_obj_t* throttle_slider_ = nullptr;
    lv_obj_t* brake_slider_ = nullptr;
    lv_obj_t* status_label_ = nullptr;
};
