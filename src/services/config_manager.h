#pragma once
namespace config_manager {
bool load_and_apply();
bool save_brightness(int level);
bool save_wifi(const char* ssid, const char* password);
void apply_on_startup();
}
