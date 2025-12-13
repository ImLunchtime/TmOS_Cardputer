/*
 * @Author: ImLunchtime knoxmedia@yeah.net
 * @Date: 2025-11-07 17:10:20
 * @LastEditors: ImLunchtime knoxmedia@yeah.net
 * @LastEditTime: 2025-11-26 16:12:42
 * @FilePath: \CardputerOS2_LVGL\src\main.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <M5Cardputer.h>
#include <lvgl.h>

#include "lvgl_port.h"
#include "input_kb.h"
#include "window_system.h"
#include "SDFileManager.h"
#include "apps/launcher/app_launcher.h"
#include "theme.h"
#include "globals.h"

static WindowSystem g_wm;

void setup() {
    Serial.begin(115200);
    auto cfg = M5.config();
    cfg.serial_baudrate = 115200;
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Keyboard.begin();
    M5.begin(cfg);

    globals::init_defaults();

    // Initialize SD Card
    static SDFileManager sd_manager;
    if (!sd_manager.initialize()) {
        Serial.println("SD card initialization failed");
    } else {
        Serial.println("SD card initialized");
    }

    lvgl_setup();
    kb_init();
    ui_theme::init();
    // Create wallpaper behind windows
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
