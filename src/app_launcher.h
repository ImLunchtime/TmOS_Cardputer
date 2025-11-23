/*
 * @Author: ImLunchtime knoxmedia@yeah.net
 * @Date: 2025-11-08 18:13:39
 * @LastEditors: ImLunchtime knoxmedia@yeah.net
 * @LastEditTime: 2025-11-20 14:45:18
 * @FilePath: \CardputerOS2_LVGL\src\app_launcher.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
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
    void launchDevices();
    void launchBluetooth();

private:
    WindowSystem& wm_;
    lv_obj_t* root_ = nullptr;
    lv_obj_t* grid_ = nullptr;
};
