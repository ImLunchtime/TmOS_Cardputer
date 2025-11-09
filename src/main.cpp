#include <M5Cardputer.h>
#include <lvgl.h>

#include "lvgl_port.h"
#include "input_kb.h"
#include "window_system.h"
#include "app_launcher.h"
#include "app_theme_center.h"
#include "app_settings.h"
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
    // Drive LVGL from loop() to avoid cross-thread races

    // Initialize window system and auto-open launcher
    g_wm.openApp(std::unique_ptr<IApp>(new AppLauncher(g_wm)));
}

void loop() {
    kb_process_hardware_keys();
    // Run LVGL timers on the main thread with dynamic tick
    static uint32_t last_ms = millis();
    uint32_t now = millis();
    uint32_t elapsed = now - last_ms;
    if (elapsed > 0) {
        lv_tick_inc(elapsed);
        last_ms = now;
    }
    lv_timer_handler();
    // Drive window system: only active app runs
    g_wm.update();
    // Relax the loop to reduce CPU usage
    delay(5);
    yield();
}
