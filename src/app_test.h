/*
 * @Author: ImLunchtime knoxmedia@yeah.net
 * @Date: 2025-11-07 17:44:07
 * @LastEditors: ImLunchtime knoxmedia@yeah.net
 * @LastEditTime: 2025-11-23 07:36:25
 * @FilePath: \CardputerOS2_LVGL\src\app_test.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include "window_system.h"
#include "theme.h"

class AppTest : public IApp {
public:
    AppTest();
    ~AppTest() override;

    const char* title() const override { return "Test"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Light; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override;

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* btn_ = nullptr;
    lv_obj_t* label_ = nullptr;
    static void on_btn_clicked(lv_event_t* e);
};
