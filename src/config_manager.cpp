#include "config_manager.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "ui_notify.h"
#include "SDFileManager.h"
#include "globals.h"
#include "wifi_manager.h"
#include <lvgl.h>

namespace config_manager {

static const char* cfg_path() { return "/tmos_config.json"; }

static bool read_config(DynamicJsonDocument& doc, SDFileManager& fm, bool& has_file) {
    has_file = fm.exists(cfg_path());
    if (!has_file) return false;
    String content = fm.readFile(cfg_path());
    if (content.length() == 0) return false;
    DeserializationError err = deserializeJson(doc, content);
    if (err) return false;
    return true;
}

static bool write_config(DynamicJsonDocument& doc, SDFileManager& fm) {
    String out;
    serializeJson(doc, out);
    return fm.writeFile(cfg_path(), out);
}

bool load_and_apply() {
    apply_on_startup();
    return true;
}

bool save_brightness(int level) {
    SDFileManager fm;
    if (!fm.initialize()) {
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "SD card not available", 2500);
        return false;
    }
    DynamicJsonDocument doc(512);
    bool has_file = false;
    read_config(doc, fm, has_file);
    doc["brightness_level"] = level;
    if (!write_config(doc, fm)) {
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "Failed to write config", 2500);
        return false;
    }
    return true;
}

bool save_wifi(const char* ssid, const char* password) {
    SDFileManager fm;
    if (!fm.initialize()) {
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "SD card not available", 2500);
        return false;
    }
    DynamicJsonDocument doc(512);
    bool has_file = false;
    read_config(doc, fm, has_file);
    JsonObject wifi = doc["wifi"].isNull() ? doc.createNestedObject("wifi") : doc["wifi"].as<JsonObject>();
    wifi["ssid"] = ssid ? ssid : "";
    wifi["password"] = password ? password : "";
    if (!write_config(doc, fm)) {
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "Failed to write config", 2500);
        return false;
    }
    return true;
}

void apply_on_startup() {
    SDFileManager fm;
    if (!fm.initialize()) {
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "SD card not available", 2500);
        return;
    }
    DynamicJsonDocument doc(512);
    bool has_file = false;
    if (!read_config(doc, fm, has_file)) {
        if (!has_file) {
            ui_notify::showSymbol(LV_SYMBOL_WARNING, "Config file not found", 2500);
        } else {
            ui_notify::showSymbol(LV_SYMBOL_WARNING, "Invalid config JSON", 2500);
        }
        return;
    }
    ui_notify::showSymbol(LV_SYMBOL_OK, "Config loaded", 2000);
    if (doc.containsKey("brightness_level")) {
        int lvl = doc["brightness_level"].as<int>();
        globals::set_brightness_level(lvl);
    }
    JsonObject wifi = doc["wifi"];
    if (!wifi.isNull()) {
        const char* ssid = wifi["ssid"] | "";
        const char* password = wifi["password"] | "";
        if (ssid && ssid[0]) {
            String msg = String("Connecting WiFi ") + ssid;
            ui_notify::showSymbol(LV_SYMBOL_WIFI, msg.c_str(), 2500);
            wifi_manager::connect(ssid, password);
        }
    }
}

}
