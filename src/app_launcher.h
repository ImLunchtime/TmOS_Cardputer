#pragma once
#include "window_system.h"

class AppLauncher : public IApp {
public:
    explicit AppLauncher(WindowSystem& wm) : wm_(wm) {}
    ~AppLauncher() override {}

    const char* title() const override { return "Launcher"; }
    bool isLauncher() const override { return true; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Dark; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override {}

    void launchMusic();
    void launchThemeCenter();
    void launchSettings();
    void launchTest();

private:
    WindowSystem& wm_;
    lv_obj_t* root_ = nullptr;
    lv_obj_t* list_ = nullptr;
};