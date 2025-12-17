#include "app_station_reporter.h"
#include <M5Cardputer.h>
#include <SD.h>
#include <lvgl.h>
#include "ui/theme.h"

AppStationReporter::AppStationReporter() {
}

AppStationReporter::~AppStationReporter() {
}

void AppStationReporter::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    
    // Remove padding/margin from root
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_set_style_border_width(root_, 0, 0);

    // Window background: vertical gradient (top 0x6A98CC -> bottom 0x3D5C80)
    lv_obj_set_style_bg_color(root_, lv_color_hex(0x6A98CC), 0);
    lv_obj_set_style_bg_grad_color(root_, lv_color_hex(0x3D5C80), 0);
    lv_obj_set_style_bg_grad_dir(root_, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(root_, LV_OPA_COVER, 0);

    // Create Tabview
    tabview_ = lv_tabview_create(root_, LV_DIR_TOP, 25);
    // Let root gradient be visible under content
    lv_obj_set_style_bg_opa(tabview_, LV_OPA_TRANSP, 0);
    lv_obj_t* tab_content = lv_tabview_get_content(tabview_);
    lv_obj_set_style_bg_opa(tab_content, LV_OPA_TRANSP, 0);

    // Tab bar background: vertical gradient (top 0xB5D8FF -> bottom 0x54A4FF)
    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview_);
    lv_obj_set_style_bg_color(tab_btns, lv_color_hex(0xB5D8FF), 0);
    lv_obj_set_style_bg_grad_color(tab_btns, lv_color_hex(0x54A4FF), 0);
    lv_obj_set_style_bg_grad_dir(tab_btns, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(tab_btns, LV_OPA_COVER, 0);
    
    lv_obj_t* t1 = lv_tabview_add_tab(tabview_, "Control");
    lv_obj_t* t2 = lv_tabview_add_tab(tabview_, "Route");
    lv_obj_t* t3 = lv_tabview_add_tab(tabview_, "Settings");
    lv_obj_t* t4 = lv_tabview_add_tab(tabview_, "Tips");
    
    buildTab1(t1);
    buildTab2(t2);
    buildTab3(t3);
    buildTab4(t4);

    loadRouteList();

    // Try to load default route
    loadRoute("/bus_routes/line1.json");
    
    updateUI();
    initializeAudioTask();
    sendAudioCommand(AUDIO_CMD_VOLUME, 1);
}

/* UI functions moved to app_station_reporter_ui.cpp */

void AppStationReporter::onTick() {
    if (state_ == MOVING) {
        if (millis() - last_blink_time_ > 500) {
            last_blink_time_ = millis();
            blink_toggle_ = !blink_toggle_;
            if (blink_toggle_) {
                // Yellow
                lv_obj_set_style_text_color(label_next_station_, lv_color_hex(0xFFFF00), 0);
            } else {
                // White
                lv_obj_set_style_text_color(label_next_station_, lv_color_hex(0xFFFFFF), 0);
            }
        }
    }
}

void AppStationReporter::onClose() {
    cleanupAudioTask();
}

/* UI functions moved to app_station_reporter_ui.cpp */
