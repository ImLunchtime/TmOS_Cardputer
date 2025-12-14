#pragma once
#include <lvgl.h>
#include <functional>
#include "window_system.h"

// App-defined custom key handler
struct AppCustomKey {
    uint16_t key;
    std::function<bool()> is_active;
    std::function<void()> on_press;
};

void kb_init();
lv_indev_t* kb_get_indev();
lv_group_t* kb_get_group();
lv_group_t* kb_get_current_group();
void kb_set_indev_group(lv_group_t* group);
// Navigation control
void kb_set_nav_disabled(bool disabled);
bool kb_is_nav_disabled();
void kb_process_hardware_keys();
bool kb_consume_exit_requested();

// Active app routing and per-app key registration
void kb_set_active_app(IApp* app);
void kb_register_app_keys(IApp* owner, const std::vector<AppCustomKey>& keys);
void kb_clear_app_keys(IApp* owner);
