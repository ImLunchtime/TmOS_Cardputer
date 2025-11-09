#pragma once
#include "window_system.h"

// Forward declarations for apps
class IApp;

class AppLauncher : public IApp {
public:
    explicit AppLauncher(WindowSystem& wm) : wm_(wm) {}
    ~AppLauncher() override {}

    const char* title() const override { return "Launcher"; }
    bool isLauncher() const override { return true; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Dark; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override;
    void onClose() override {}

    // Carousel controls
    void layoutIcons();
    void setFocusedIndex(int idx);

    // App launch methods
    void launchMusic();
    void launchThemeCenter();
    void launchSettings();

private:
    WindowSystem& wm_;
    lv_obj_t* root_ = nullptr;
    
    // Grid layout container
    lv_obj_t* grid_container_ = nullptr;
    
    // App buttons
    lv_obj_t* btn_music_ = nullptr;
    lv_obj_t* btn_theme_center_ = nullptr;
    lv_obj_t* btn_settings_ = nullptr;
    // Label for focused app name
    lv_obj_t* focused_label_ = nullptr;

    // Carousel state
    int focused_index_ = 1; // default to middle icon
    bool layout_initialized_ = false;
};