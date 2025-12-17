#include "core/globals.h"
#include "services/config_manager.h"

namespace globals {

static int s_brightness_level = 5;

int get_brightness_level() {
    return s_brightness_level;
}

void set_brightness_level(int level) {
    if (level < 0) level = 0;
    if (level > 9) level = 9;
    s_brightness_level = level;
    int raw = level_to_raw(level);
    M5Cardputer.Display.setBrightness(raw);
    config_manager::save_brightness(s_brightness_level);
}

void init_defaults() { M5Cardputer.Display.setBrightness(level_to_raw(s_brightness_level)); }

} // namespace globals
