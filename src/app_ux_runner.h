#pragma once
#include "window_system.h"
#include "ux_runtime.h"

class AppUXRunner : public IApp {
public:
    explicit AppUXRunner(const String& path) : path_(path) {}
    const char* title() const override { return "UX Player"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Dark; }
    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override {}
private:
    String path_;
    lv_obj_t* root_ = nullptr;
};

