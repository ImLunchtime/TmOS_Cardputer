/*
 * @Author: ImLunchtime knoxmedia@yeah.net
 * @Date: 2025-11-30 11:26:38
 * @LastEditors: ImLunchtime knoxmedia@yeah.net
 * @LastEditTime: 2025-11-30 16:45:55
 * @FilePath: \CardputerOS2_LVGL\src\apps/ux_executor/app_ux_executor.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include "window_system.h"
#include "SDFileManager.h"
#include "ux_runtime.h"
#include <vector>
#include <unordered_map>

class AppUXExecutor : public IApp {
public:
    explicit AppUXExecutor(WindowSystem& wm) : wm_(wm) {}
    const char* title() const override { return "UX Executor"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Dark; }
    void onOpen(lv_obj_t* window_root) override;
    void onTick() override {}
    void onClose() override {}
    void refresh_list();
    void run_current();
    void onItemClicked(lv_obj_t* btn);
private:
    WindowSystem& wm_;
    lv_obj_t* root_ = nullptr;
    lv_obj_t* list_ = nullptr;
    SDFileManager fm_;
    String current_path_;
    std::vector<FileInfo> files_;
    std::unordered_map<lv_obj_t*, int> item_index_;
};
