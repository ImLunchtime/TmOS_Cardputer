#pragma once
#include "window_system.h"

class AppMusic : public IApp {
public:
    AppMusic() {}
    ~AppMusic() override {}

    const char* title() const override { return "Music"; }
    bool isLauncher() const override { return false; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override {}

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* label_ = nullptr;
    lv_obj_t* btn_play_ = nullptr;
};