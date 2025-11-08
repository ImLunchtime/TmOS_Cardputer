#include <M5Cardputer.h>
#include <lvgl.h>

#include "lvgl_port.h"
#include "input_kb.h"
#include "window_system.h"
#include "app_launcher.h"
#include "theme.h"

static WindowSystem g_wm;

void setup() {
    Serial.begin(115200);
    auto cfg = M5.config();
    cfg.serial_baudrate = 115200;
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Keyboard.begin();
    M5.begin(cfg);

    lvgl_setup();
    kb_init();
    ui_theme::init();
    // Create wallpaper (solid black) behind windows
    ui_theme::create_wallpaper();
    start_lvgl_tasks();

    // Initialize window system and auto-open launcher
    g_wm.openApp(std::unique_ptr<IApp>(new AppLauncher(g_wm)));
}

void loop() {
    kb_process_hardware_keys();
    // Drive window system: only active app runs
    // Update the window manager (handles BtnA close for non-launcher)
    g_wm.update();
    delay(5);
    yield();
}
