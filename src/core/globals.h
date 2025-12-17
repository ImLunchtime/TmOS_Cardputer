#pragma once
#include <Arduino.h>
#include <M5Cardputer.h>

namespace globals {

int get_brightness_level();
void set_brightness_level(int level);

inline int level_to_raw(int level) {
    if (level < 0) level = 0;
    if (level > 9) level = 9;
    return (level * 255 + 4) / 9;
}

inline int raw_to_level(int raw) {
    if (raw < 0) raw = 0;
    if (raw > 255) raw = 255;
    return (raw * 9 + 127) / 255;
}

void init_defaults();

} // namespace globals
