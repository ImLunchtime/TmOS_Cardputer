#pragma once
#include "window_system.h"
#include "wifi_manager.h"
#include <vector>
#include <unordered_map>
#include <lvgl.h>

class AppWiFi : public IApp {
public:
    AppWiFi();
    ~AppWiFi() override;

    const char* title() const override { return "WiFi"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Light; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override;
    void onClose() override;

    static void on_refresh(lv_event_t* e);
    static void on_list_item(lv_event_t* e);
    static void on_connect(lv_event_t* e);
    static void on_cancel(lv_event_t* e);
    static void on_scan_timer(lv_timer_t* t);
    static void on_disc_confirm(lv_event_t* e);
    static void on_disc_cancel(lv_event_t* e);

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* header_ = nullptr;
    lv_obj_t* label_status_ = nullptr;
    lv_obj_t* btn_refresh_ = nullptr;
    lv_obj_t* list_ = nullptr;
    lv_obj_t* panel_connect_ = nullptr;
    lv_obj_t* panel_connected_ = nullptr;
    lv_obj_t* label_ssid_ = nullptr;
    lv_obj_t* label_connected_msg_ = nullptr;
    lv_obj_t* ta_password_ = nullptr;
    lv_obj_t* btn_connect_ = nullptr;
    lv_obj_t* btn_cancel_ = nullptr;
    lv_obj_t* btn_disc_confirm_ = nullptr;
    lv_obj_t* btn_disc_cancel_ = nullptr;

    std::vector<wifi_manager::Network> networks_;
    String selected_ssid_;
    String last_password_;
    bool connecting_ = false;
    std::unordered_map<lv_obj_t*, int> item_index_;
    lv_timer_t* timer_scan_ = nullptr;

    void refresh_scan();
    void build_connect_panel();
    void build_connected_panel();
    void show_connect_panel(const String& ssid, bool secure);
    void hide_connect_panel();
    void show_connected_panel();
    void hide_connected_panel();
    void populate_list();
    void rebuildFocusGroup();
    void add_focusables_recursive(lv_obj_t* node, lv_group_t* group);
    bool is_focusable(lv_obj_t* obj);

};
