#pragma once
#include "window_system.h"
#include <vector>
#include <ArduinoJson.h>

struct Station {
    String name;
    String id;
};

struct RouteInfo {
    String id;
    String name;
};

class AppStationReporter : public IApp {
public:
    AppStationReporter();
    ~AppStationReporter() override;

    const char* title() const override { return "Station Report"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Light; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override;
    void onClose() override;

private:
    void loadRoute(const String& filename);
    void updateUI();
    void buildTab1(lv_obj_t* parent);
    void buildTab2(lv_obj_t* parent);
    void buildTab3(lv_obj_t* parent);
    void refreshStationList();
    void loadRouteList();

    static void event_handler(lv_event_t* e);

    lv_obj_t* root_ = nullptr;
    lv_obj_t* tabview_ = nullptr;
    
    // Tab 1 Controls
    lv_obj_t* label_route_name_ = nullptr;
    lv_obj_t* label_curr_station_ = nullptr;
    lv_obj_t* label_next_station_ = nullptr;
    lv_obj_t* label_status_ = nullptr;
    
    lv_obj_t* btn_action_ = nullptr; // Main action button (Depart/Arrive)
    lv_obj_t* label_btn_action_ = nullptr;
    
    lv_obj_t* btn_prev_ = nullptr;
    lv_obj_t* btn_emergency_ = nullptr;

    // Tab 2 Controls
    lv_obj_t* list_stations_ = nullptr;

    // Tab 3 Controls
    lv_obj_t* list_routes_ = nullptr;

    // Logic
    std::vector<Station> stations_;
    std::vector<RouteInfo> available_routes_;
    int current_station_idx_ = 0;
    String route_name_ = "Unknown Route";
    
    enum State {
        NOT_STARTED,
        MOVING,     // Moving to next station
        IN_STATION, // Stopped at station
        EMERGENCY   // Emergency stop
    };
    State state_ = NOT_STARTED;
};
