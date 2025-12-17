/*
 * @Author: ImLunchtime knoxmedia@yeah.net
 * @Date: 2025-11-30 07:52:52
 * @LastEditors: ImLunchtime knoxmedia@yeah.net
 * @LastEditTime: 2025-12-07 10:38:58
 * @FilePath: \CardputerOS2_LVGL\src\apps\circuitsim\app_circuitsim.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include "core/window_system.h"
#include "ui/theme.h"

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
