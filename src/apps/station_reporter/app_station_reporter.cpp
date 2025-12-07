#include "app_station_reporter.h"
#include <M5Cardputer.h>
#include <SD.h>
#include <lvgl.h>

AppStationReporter::AppStationReporter() {
}

AppStationReporter::~AppStationReporter() {
}

void AppStationReporter::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    
    // Remove padding/margin from root
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_set_style_border_width(root_, 0, 0);

    // Create Tabview
    tabview_ = lv_tabview_create(root_, LV_DIR_TOP, 25);
    
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
    // Top: Route Name
    // Middle: Current Station -> Next Station
    // Bottom: Status Label
    // Buttons: Prev | Action (Depart/Arrive) | Emergency

    // Use a flex layout for vertical arrangement
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(parent, 5, 0);
    lv_obj_set_style_pad_row(parent, 5, 0);

    // Route Name
    label_route_name_ = lv_label_create(parent);
    lv_label_set_text(label_route_name_, "Loading...");
    lv_obj_set_style_text_font(label_route_name_, &lv_font_montserrat_14, 0);

    // Station Info Container
    lv_obj_t* station_cont = lv_obj_create(parent);
    lv_obj_set_size(station_cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(station_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(station_cont, 5, 0);
    
    label_curr_station_ = lv_label_create(station_cont);
    lv_label_set_text(label_curr_station_, "Curr: --");
    
    label_next_station_ = lv_label_create(station_cont);
    lv_label_set_text(label_next_station_, "Next: --");

    // Status
    label_status_ = lv_label_create(parent);
    lv_label_set_text(label_status_, "Status: Not Started");
    lv_obj_set_style_text_color(label_status_, lv_palette_main(LV_PALETTE_GREY), 0);

    // Buttons Container
    lv_obj_t* btn_cont = lv_obj_create(parent);
    lv_obj_set_size(btn_cont, LV_PCT(100), 40);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(btn_cont, 0, 0);
    lv_obj_set_style_border_width(btn_cont, 0, 0);

    // Prev Button
    btn_prev_ = lv_btn_create(btn_cont);
    lv_obj_set_size(btn_prev_, 40, 30);
    lv_obj_add_event_cb(btn_prev_, event_handler, LV_EVENT_CLICKED, this);
    lv_obj_t* l_prev = lv_label_create(btn_prev_);
    lv_label_set_text(l_prev, "<");
    lv_obj_center(l_prev);
    
    // Action Button
    btn_action_ = lv_btn_create(btn_cont);
    lv_obj_set_flex_grow(btn_action_, 1);
    lv_obj_set_height(btn_action_, 30);
    lv_obj_add_event_cb(btn_action_, event_handler, LV_EVENT_CLICKED, this);
    label_btn_action_ = lv_label_create(btn_action_);
    lv_label_set_text(label_btn_action_, "Start");
    lv_obj_center(label_btn_action_);

    // Emergency Button
    btn_emergency_ = lv_btn_create(btn_cont);
    lv_obj_set_size(btn_emergency_, 40, 30);
    lv_obj_set_style_bg_color(btn_emergency_, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_add_event_cb(btn_emergency_, event_handler, LV_EVENT_CLICKED, this);
    lv_obj_t* l_em = lv_label_create(btn_emergency_);
    lv_label_set_text(l_em, "!");
    lv_obj_center(l_em);
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

    String curr = "Curr: " + stations_[current_station_idx_].name;
    lv_label_set_text(label_curr_station_, curr.c_str());

    String next = "Next: --";
    if (current_station_idx_ + 1 < stations_.size()) {
        next = "Next: " + stations_[current_station_idx_ + 1].name;
    } else {
        next = "Next: End of Line";
    }
    lv_label_set_text(label_next_station_, next.c_str());

    // Update Action Button and Status Label
    switch (state_) {
        case NOT_STARTED:
            lv_label_set_text(label_btn_action_, "Start Run");
            lv_label_set_text(label_status_, "Status: Not Started");
            lv_obj_set_style_bg_color(btn_action_, lv_palette_main(LV_PALETTE_BLUE), 0);
            break;
        case MOVING:
            lv_label_set_text(label_btn_action_, "Arrive");
            lv_label_set_text(label_status_, "Status: Moving");
             lv_obj_set_style_bg_color(btn_action_, lv_palette_main(LV_PALETTE_GREEN), 0);
            break;
        case IN_STATION:
            lv_label_set_text(label_btn_action_, "Depart");
            lv_label_set_text(label_status_, "Status: Stopped");
             lv_obj_set_style_bg_color(btn_action_, lv_palette_main(LV_PALETTE_ORANGE), 0);
            break;
        case EMERGENCY:
            lv_label_set_text(label_btn_action_, "Resume");
            lv_label_set_text(label_status_, "Status: EMERGENCY");
            lv_obj_set_style_bg_color(btn_action_, lv_palette_main(LV_PALETTE_RED), 0);
            break;
    }
}

void AppStationReporter::onTick() {
    // Maybe blink if emergency?
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
