#pragma once
#include <lvgl.h>

void kb_init();
lv_indev_t* kb_get_indev();
lv_group_t* kb_get_group();
void kb_process_hardware_keys();