#pragma once
#include "window_system.h"
#include "theme.h"

class AppBrightness : public IApp {
public:
    AppBrightness();
    ~AppBrightness() override;

    const char* title() const override { return "亮度"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Light; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override;

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* slider_ = nullptr;
    lv_obj_t* label_ = nullptr;

    static void on_slider_changed(lv_event_t* e);
};

