#pragma once
#include "window_system.h"

class AppLauncher : public IApp {
public:
    explicit AppLauncher(WindowSystem& wm) : wm_(wm) {}
    ~AppLauncher() override {}

    const char* title() const override { return "Launcher"; }
    bool isLauncher() const override { return true; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override {}

    // Expose action to open Music app
    void launchMusic();

private:
    WindowSystem& wm_;
    lv_obj_t* root_ = nullptr;
    lv_obj_t* btn_music_ = nullptr;
};