#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <vector>

namespace wifi_manager {
    enum class State { Idle, Scanning, Connecting, Connected, Failed };
    struct Network {
        String ssid;
        int rssi;
        bool secure;
        int channel;
        wifi_auth_mode_t auth;
    };

    void init();
    void deinit();
    void update();

    bool is_connected();
    State get_state();
    String connected_ssid();
    int connected_rssi();
    String last_error();

    void disconnect();
    void set_auto_reconnect(bool enable);

    void start_scan();
    bool is_scanning();
    std::vector<Network> take_scan_results();
    bool has_scan_results();

    void connect(const char* ssid, const char* password, uint32_t timeout_ms = 20000);
}
