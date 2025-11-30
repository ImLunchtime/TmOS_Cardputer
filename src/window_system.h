/*
 * @Author: ImLunchtime knoxmedia@yeah.net
 * @Date: 2025-11-08 18:13:39
 * @LastEditors: ImLunchtime knoxmedia@yeah.net
 * @LastEditTime: 2025-11-30 20:51:17
 * @FilePath: \CardputerOS2_LVGL\src\window_system.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include <lvgl.h>
#include <vector>
#include <memory>
#include <functional>
#include "theme.h"

// Base interface for apps hosted by the window system
class IApp {
public:
    virtual ~IApp() {}

    // Title displayed on the window
    virtual const char* title() const = 0;

    // Build app UI into provided window root container
    virtual void onOpen(lv_obj_t* window_root) = 0;

    // Per-loop update for the active app only
    virtual void onTick() {}

    // Called before destroying the window; apps should release resources
    virtual void onClose() {}

    // Whether this app is the launcher (cannot be closed with BtnA)
    virtual bool isLauncher() const { return false; }

    // Preferred theme for this app's window
    virtual ui_theme::ThemeId theme() const { return ui_theme::ThemeId::Light; }
};

// WindowSystem manages window stacking, positioning and focus control
class WindowSystem {
public:
    WindowSystem();
    ~WindowSystem();

    // Open a new app window in front of existing ones
    void openApp(std::unique_ptr<IApp> app);

    // Close the top-most window (if any)
    void closeTop();

    // Drive active app updates and handle close button logic
    void update();

    // Accessor for active app (top-most)
    IApp* activeApp() const;

    // Resize the active window; keeps it within screen bounds
    void resizeActiveWindow(lv_coord_t w, lv_coord_t h);

    // Resize a specific window by its root container
    void resizeWindow(lv_obj_t* root, lv_coord_t w, lv_coord_t h);

private:
    struct WindowEntry {
        std::unique_ptr<IApp> app;
        lv_obj_t* root = nullptr;     // Window container
        lv_group_t* group = nullptr;  // Focus group for this window
        lv_coord_t x = 0;
        lv_coord_t y = 0;
        lv_coord_t w = 0;
        lv_coord_t h = 0;
    };

    std::vector<WindowEntry> stack_;

    // Create a window container and position it based on previous
    lv_obj_t* createWindowContainer(const char* title, lv_coord_t* out_w, lv_coord_t* out_h, lv_coord_t* out_x, lv_coord_t* out_y) const;
    lv_obj_t* createWindowContainer(const char* title, lv_coord_t* out_w, lv_coord_t* out_h, lv_coord_t* out_x, lv_coord_t* out_y, ui_theme::ThemeId theme) const;

    // Build group from focusable children of root
    lv_group_t* buildFocusGroup(lv_obj_t* root) const;

    // Apply active/inactive state and focus routing
    void applyActiveState();
};
