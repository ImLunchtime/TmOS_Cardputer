#pragma once
#include <lvgl.h>
#include <Arduino.h>

namespace ux {

struct BuildResult {
    bool ok;
    String error;
};

BuildResult build_from_string(const String& json, lv_obj_t* root);
BuildResult build_from_file(const String& path, lv_obj_t* root);
void clear_children(lv_obj_t* parent);

}

