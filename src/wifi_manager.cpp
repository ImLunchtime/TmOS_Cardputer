#include "wifi_manager.h"
#include <vector>
#include <esp_wifi.h>

namespace wifi_manager {
    static State s_state = State::Idle;
    static String s_connected_ssid;
    static int s_connected_rssi = 0;
    static String s_last_error;
    static uint32_t s_deadline = 0;
    static std::vector<Network> s_scan_results;
    static bool s_scanning = false;
    static uint32_t s_scan_deadline = 0;

    static void hard_reset_wifi() {
        esp_wifi_stop();
        esp_wifi_deinit();
        WiFi.disconnect(true, true);
        WiFi.mode(WIFI_MODE_NULL);
        delay(50);
    }
 
    static void ensure_sta_mode() {
        wifi_mode_t mode = WiFi.getMode();
        if (mode != WIFI_MODE_STA) {
            hard_reset_wifi();
        }
        WiFi.mode(WIFI_STA);
    }

    void init() {
        hard_reset_wifi();
        ensure_sta_mode();
        WiFi.setAutoReconnect(true);
        s_state = State::Idle;
        s_connected_ssid = "";
        s_connected_rssi = 0;
        s_last_error = "";
        s_scanning = false;
        s_scan_results.clear();
    }

    void deinit() {
        WiFi.disconnect(true);
        s_state = State::Idle;
        s_connected_ssid = "";
        s_connected_rssi = 0;
        s_last_error = "";
        s_scanning = false;
        s_scan_results.clear();
    }

    

    bool is_connected() { return s_state == State::Connected; }
    State get_state() { return s_state; }
    String connected_ssid() { return s_connected_ssid; }
    int connected_rssi() { return s_connected_rssi; }
    String last_error() { return s_last_error; }

    void disconnect() {
        WiFi.disconnect(true);
        s_state = State::Idle;
        s_connected_ssid = "";
        s_connected_rssi = 0;
    }

    void set_auto_reconnect(bool enable) { WiFi.setAutoReconnect(enable); }

    void start_scan() {
        ensure_sta_mode();
        s_scanning = true;
        s_state = State::Scanning;
        s_scan_results.clear();
        WiFi.scanDelete();
        int n = WiFi.scanNetworks();
        s_scan_results.clear();
        if (n > 0) {
            for (int i = 0; i < n; ++i) {
                Network nw;
                nw.ssid = WiFi.SSID(i);
                nw.rssi = WiFi.RSSI(i);
                nw.channel = WiFi.channel(i);
                nw.auth = WiFi.encryptionType(i);
                nw.secure = (nw.auth != WIFI_AUTH_OPEN);
                s_scan_results.push_back(nw);
            }
        }
        WiFi.scanDelete();
        s_scanning = false;
        wl_status_t st = WiFi.status();
        if (st == WL_CONNECTED) {
            s_state = State::Connected;
            s_connected_ssid = WiFi.SSID();
            s_connected_rssi = WiFi.RSSI();
            s_last_error = "";
        } else {
            s_state = State::Idle;
        }
        if (n < 0) s_last_error = "Scan failed";
    }

    bool is_scanning() { return s_scanning; }
    bool has_scan_results() { return !s_scan_results.empty(); }
    std::vector<Network> take_scan_results() {
        std::vector<Network> out = s_scan_results;
        s_scan_results.clear();
        return out;
    }

    void connect(const char* ssid, const char* password, uint32_t timeout_ms) {
        if (!ssid || !*ssid) return;
        ensure_sta_mode();
        WiFi.disconnect(true);
        wl_status_t st = WiFi.status();
        if (st == WL_CONNECTED) WiFi.disconnect(true);
        WiFi.begin(ssid, password ? password : "");
        s_state = State::Connecting;
        s_deadline = millis() + timeout_ms;
        s_connected_ssid = "";
        s_connected_rssi = 0;
        s_last_error = "";
    }

    void update() {
        if (s_state == State::Connecting) {
            wl_status_t st = WiFi.status();
            if (st == WL_CONNECTED) {
                s_state = State::Connected;
                s_connected_ssid = WiFi.SSID();
                s_connected_rssi = WiFi.RSSI();
                s_last_error = "";
            } else {
                if (millis() > s_deadline) {
                    s_state = State::Failed;
                    if (st == WL_NO_SSID_AVAIL) s_last_error = "SSID not available";
                    else if (st == WL_CONNECT_FAILED) s_last_error = "Connect failed";
                    else s_last_error = "Timeout";
                }
            }
        } else if (s_state == State::Connected) {
            wl_status_t st = WiFi.status();
            if (st != WL_CONNECTED) {
                s_state = State::Failed;
                s_last_error = "Disconnected";
            } else {
                s_connected_rssi = WiFi.RSSI();
            }
        }
    }
}
