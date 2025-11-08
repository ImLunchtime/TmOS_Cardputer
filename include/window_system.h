#pragma once
#include <lvgl.h>
#include <vector>
#include <memory>
#include <functional>

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

    // Build group from focusable children of root
    lv_group_t* buildFocusGroup(lv_obj_t* root) const;

    // Apply active/inactive state and focus routing
    void applyActiveState();
};