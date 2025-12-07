#include "app_station_reporter.h"
#include <M5Cardputer.h>
#include <SD.h>
#include <lvgl.h>
#include "theme.h"

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
    
    buildTab1(t1);
    buildTab2(t2);
    buildTab3(t3);

    loadRouteList();

    // Try to load default route
    loadRoute("/bus_routes/line1.json");
    
    updateUI();
}

void AppStationReporter::buildTab1(lv_obj_t* parent) {
    // Layout: 
    // Header (Row): Route Name | Status
    // Middle: Current Station -> Next Station
    // Bottom: Buttons

    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 2, 0);
    lv_obj_set_style_pad_gap(parent, 2, 0);

    // Header Container
    lv_obj_t* header = lv_obj_create(parent);
    lv_obj_set_size(header, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(header, 0, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, 0);

    // Route Name
    label_route_name_ = lv_label_create(header);
    lv_label_set_text(label_route_name_, "Loading...");
    lv_obj_set_style_text_font(label_route_name_, ui_theme::get_system_font(), 0);
    lv_obj_set_flex_grow(label_route_name_, 1);
    lv_label_set_long_mode(label_route_name_, LV_LABEL_LONG_SCROLL_CIRCULAR);

    // Status
    label_status_ = lv_label_create(header);
    lv_label_set_text(label_status_, "--");
    lv_obj_set_style_text_font(label_status_, ui_theme::get_system_font(), 0);
    lv_obj_set_style_text_color(label_status_, lv_color_hex(0xFFFFFF), 0);

    // Station Info Container
    lv_obj_t* station_cont = lv_obj_create(parent);
    lv_obj_set_width(station_cont, LV_PCT(100));
    lv_obj_set_flex_grow(station_cont, 1);
    lv_obj_set_flex_flow(station_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(station_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(station_cont, 2, 0);
    lv_obj_set_style_pad_gap(station_cont, 5, 0);
    lv_obj_set_style_bg_opa(station_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(station_cont, 0, 0);
    
    label_curr_station_ = lv_label_create(station_cont);
    lv_label_set_text(label_curr_station_, "--");
    lv_obj_set_style_text_font(label_curr_station_, ui_theme::get_system_font(), 0);
    lv_obj_set_flex_grow(label_curr_station_, 1);
    lv_label_set_long_mode(label_curr_station_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(label_curr_station_, LV_TEXT_ALIGN_CENTER, 0);
    
    label_next_station_ = lv_label_create(station_cont);
    lv_label_set_text(label_next_station_, "--");
    lv_obj_set_style_text_font(label_next_station_, ui_theme::get_system_font(), 0);
    lv_obj_set_flex_grow(label_next_station_, 1);
    lv_label_set_long_mode(label_next_station_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(label_next_station_, LV_TEXT_ALIGN_CENTER, 0);

    // Buttons Container
    lv_obj_t* btn_cont = lv_obj_create(parent);
    lv_obj_set_size(btn_cont, LV_PCT(100), 32);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(btn_cont, 0, 0);
    lv_obj_set_style_border_width(btn_cont, 0, 0);
    lv_obj_set_style_bg_opa(btn_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_gap(btn_cont, 5, 0);

    // Prev Button
    btn_prev_ = lv_btn_create(btn_cont);
    lv_obj_set_size(btn_prev_, 40, 28);
    lv_obj_add_event_cb(btn_prev_, event_handler, LV_EVENT_CLICKED, this);
    lv_obj_t* l_prev = lv_label_create(btn_prev_);
    lv_label_set_text(l_prev, "<");
    lv_obj_center(l_prev);
    lv_obj_set_style_text_font(l_prev, ui_theme::get_system_font(), 0);
    
    // Action Button
    btn_action_ = lv_btn_create(btn_cont);
    lv_obj_set_flex_grow(btn_action_, 1);
    lv_obj_set_height(btn_action_, 28);
    lv_obj_add_event_cb(btn_action_, event_handler, LV_EVENT_CLICKED, this);
    label_btn_action_ = lv_label_create(btn_action_);
    lv_label_set_text(label_btn_action_, "Start");
    lv_obj_center(label_btn_action_);
    lv_obj_set_style_text_font(label_btn_action_, ui_theme::get_system_font(), 0);

    // Emergency Button
    btn_emergency_ = lv_btn_create(btn_cont);
    lv_obj_set_size(btn_emergency_, 40, 28);
    lv_obj_set_style_bg_color(btn_emergency_, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_add_event_cb(btn_emergency_, event_handler, LV_EVENT_CLICKED, this);
    lv_obj_t* l_em = lv_label_create(btn_emergency_);
    lv_label_set_text(l_em, "!");
    lv_obj_center(l_em);
    lv_obj_set_style_text_font(l_em, ui_theme::get_system_font(), 0);
}

void AppStationReporter::buildTab2(lv_obj_t* parent) {
    list_stations_ = lv_list_create(parent);
    lv_obj_set_size(list_stations_, LV_PCT(100), LV_PCT(100));
}

void AppStationReporter::buildTab3(lv_obj_t* parent) {
    list_routes_ = lv_list_create(parent);
    lv_obj_set_size(list_routes_, LV_PCT(100), LV_PCT(100));
}

void AppStationReporter::loadRouteList() {
    available_routes_.clear();
    lv_obj_clean(list_routes_);

    if (!SD.exists("/bus_routes/routes.json")) return;

    File file = SD.open("/bus_routes/routes.json");
    if (!file) return;

    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) return;

    JsonArray routes = doc["routes"];
    for (JsonObject r : routes) {
        RouteInfo info;
        info.id = r["id"].as<String>();
        info.name = r["name"].as<String>();
        available_routes_.push_back(info);

        lv_obj_t* btn = lv_list_add_btn(list_routes_, NULL, info.name.c_str());
        lv_obj_set_style_text_font(btn, ui_theme::get_system_font(), 0);
        lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, this);
    }
}

void AppStationReporter::loadRoute(const String& filename) {
    if (!SD.exists(filename)) {
        lv_label_set_text(label_route_name_, "File not found");
        return;
    }

    File file = SD.open(filename);
    if (!file) {
        lv_label_set_text(label_route_name_, "Read error");
        return;
    }

    // Using a safe size for ESP32
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        String err = "JSON Error: ";
        err += error.c_str();
        lv_label_set_text(label_route_name_, err.c_str());
        return;
    }

    route_name_ = doc["route_name"].as<String>();
    stations_.clear();
    JsonArray stationsJson = doc["stations"];
    for (JsonObject s : stationsJson) {
        Station station;
        station.name = s["name"].as<String>();
        station.id = s["id"].as<String>();
        stations_.push_back(station);
    }

    current_station_idx_ = 0;
    state_ = NOT_STARTED;
    refreshStationList();
}

void AppStationReporter::refreshStationList() {
    lv_obj_clean(list_stations_);
    for (int i = 0; i < stations_.size(); i++) {
        lv_obj_t* btn = lv_list_add_btn(list_stations_, NULL, stations_[i].name.c_str());
        lv_obj_set_style_text_font(btn, ui_theme::get_system_font(), 0);
        if (i == current_station_idx_) {
            lv_obj_add_state(btn, LV_STATE_CHECKED); // Highlight current
        }
    }
}

void AppStationReporter::updateUI() {
    lv_label_set_text(label_route_name_, route_name_.c_str());

    if (stations_.empty()) {
        lv_label_set_text(label_curr_station_, "No stations");
        lv_label_set_text(label_next_station_, "");
        return;
    }

    String curr = stations_[current_station_idx_].name;
    lv_label_set_text(label_curr_station_, curr.c_str());

    String next = "--";
    if (current_station_idx_ + 1 < stations_.size()) {
        next = stations_[current_station_idx_ + 1].name;
    } else {
        next = "End";
    }
    lv_label_set_text(label_next_station_, next.c_str());

    // Update Action Button and Status Label
    switch (state_) {
        case NOT_STARTED:
            lv_label_set_text(label_btn_action_, "Start Run");
            lv_label_set_text(label_status_, "Not Started");
            lv_obj_set_style_bg_color(btn_action_, lv_palette_main(LV_PALETTE_BLUE), 0);
            // Text colors unified to white when not started
            lv_obj_set_style_text_color(label_curr_station_, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_text_color(label_next_station_, lv_color_hex(0xFFFFFF), 0);
            break;
        case MOVING:
            lv_label_set_text(label_btn_action_, "Arrive");
            lv_label_set_text(label_status_, "Moving");
             lv_obj_set_style_bg_color(btn_action_, lv_palette_main(LV_PALETTE_GREEN), 0);
             // Previous/Current station -> White
             lv_obj_set_style_text_color(label_curr_station_, lv_color_hex(0xFFFFFF), 0);
             // Next station blinking handled in onTick, default to white here
             lv_obj_set_style_text_color(label_next_station_, lv_color_hex(0xFFFFFF), 0);
            break;
        case IN_STATION:
            lv_label_set_text(label_btn_action_, "Depart");
            lv_label_set_text(label_status_, "Stopped");
             lv_obj_set_style_bg_color(btn_action_, lv_palette_main(LV_PALETTE_ORANGE), 0);
             
             // Current station -> Dark Green
             lv_obj_set_style_text_color(label_curr_station_, lv_color_hex(0x006400), 0); // Dark Green
             // Next station -> White (unify grey to white)
             lv_obj_set_style_text_color(label_next_station_, lv_color_hex(0xFFFFFF), 0);
            break;
        case EMERGENCY:
            lv_label_set_text(label_btn_action_, "Resume");
            lv_label_set_text(label_status_, "EMERGENCY");
            lv_obj_set_style_bg_color(btn_action_, lv_palette_main(LV_PALETTE_RED), 0);
            break;
    }
}

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
    // Cleanup if needed
}

void AppStationReporter::event_handler(lv_event_t* e) {
    AppStationReporter* app = (AppStationReporter*)lv_event_get_user_data(e);
    lv_obj_t* target = lv_event_get_target(e);

    if (target == app->btn_action_) {
        if (app->stations_.empty()) return;

        switch (app->state_) {
            case NOT_STARTED:
                app->state_ = IN_STATION; // Start at first station
                break;
            case IN_STATION:
                // Depart
                if (app->current_station_idx_ < app->stations_.size() - 1) {
                    app->state_ = MOVING;
                } else {
                    // End of line
                    app->state_ = NOT_STARTED;
                    app->current_station_idx_ = 0;
                }
                break;
            case MOVING:
                // Arrive at next station
                app->current_station_idx_++;
                app->state_ = IN_STATION;
                break;
            case EMERGENCY:
                // Resume to previous state? assume moving for simplicity or remember last state
                app->state_ = MOVING; 
                break;
        }
        app->updateUI();
        app->refreshStationList();
    } else if (target == app->btn_prev_) {
        if (app->current_station_idx_ > 0) {
            app->current_station_idx_--;
            app->state_ = IN_STATION; // Reset to stopped at prev station
            app->updateUI();
            app->refreshStationList();
        }
    } else if (target == app->btn_emergency_) {
        app->state_ = EMERGENCY;
        app->updateUI();
    } else {
        // Check if it's a route selection
        if (lv_obj_get_parent(target) == app->list_routes_) {
             const char* txt = lv_list_get_btn_text(app->list_routes_, target);
             if (txt) {
                 for (const auto& r : app->available_routes_) {
                     if (r.name == txt) {
                         String path = "/bus_routes/" + r.id + ".json";
                         app->loadRoute(path);
                         app->updateUI();
                         lv_tabview_set_act(app->tabview_, 0, LV_ANIM_ON);
                         break;
                     }
                 }
             }
        }
    }
}
