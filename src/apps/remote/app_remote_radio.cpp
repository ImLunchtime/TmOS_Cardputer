#include "apps/remote/app_remote.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <M5Cardputer.h>
#include "services/wifi_manager.h"

void AppRemote::init_microcar_radio() {
    if (espnow_inited_) return;
    wifi_manager::set_auto_reconnect(false);
    wifi_manager::disconnect();
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_STA);
    delay(50);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    if (esp_now_init() != ESP_OK) {
        update_status("ESP-NOW init failed", last_cmd_);
        return;
    }
    {
        esp_now_peer_info_t peer = {};
        uint8_t bcast[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        memcpy(peer.peer_addr, bcast, 6);
        peer.channel = 1;
        peer.encrypt = false;
        peer.ifidx = WIFI_IF_STA;
        if (esp_now_add_peer(&peer) != ESP_OK) {
            update_status("Peer add failed", last_cmd_);
            esp_now_deinit();
            return;
        }
    }
    espnow_inited_ = true;
    update_status("ESP-NOW ready", "C_ST");
}

void AppRemote::deinit_microcar_radio() {
    if (!espnow_inited_) return;
    {
        uint8_t bcast[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        esp_now_del_peer(bcast);
    }
    esp_now_deinit();
    espnow_inited_ = false;
    update_status("ESP-NOW stopped", "C_ST");
    wifi_manager::set_auto_reconnect(true);
}

void AppRemote::send_cmd(const char* cmd) {
    if (!cmd) return;
    if (!espnow_inited_) init_microcar_radio();
    char payload[8] = {0};
    size_t n = strlen(cmd);
    if (n > 7) n = 7;
    memcpy(payload, cmd, n);
    static uint8_t bcast[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    esp_err_t err = esp_now_send(bcast, (uint8_t*)payload, sizeof(payload));
    last_cmd_ = cmd;
    last_sent_ms_ = millis();
    if (err == ESP_OK) {
        update_status("Sent", cmd);
    } else {
        update_status("Send failed", cmd);
    }
}

void AppRemote::update_status(const char* status, const char* cmd) {
    if (label_status_) {
        if (status && strcmp(status, "Sent") == 0) {
            String keys = pressed_keys_str();
            String txt = "Sent";
            if (keys.length()) { txt += " "; txt += keys; }
            lv_label_set_text(label_status_, txt.c_str());
        } else {
            lv_label_set_text(label_status_, status ? status : "-");
        }
    }
}

const char* AppRemote::decide_cmd_from_keys() const {
    if (!page_microcar_ || lv_obj_has_flag(page_microcar_, LV_OBJ_FLAG_HIDDEN)) return "C_ST";
    if (M5Cardputer.Keyboard.isKeyPressed('E') || M5Cardputer.Keyboard.isKeyPressed('e')) return "C_FD";
    if (M5Cardputer.Keyboard.isKeyPressed('A') || M5Cardputer.Keyboard.isKeyPressed('a')) return "C_LS";
    if (M5Cardputer.Keyboard.isKeyPressed('S') || M5Cardputer.Keyboard.isKeyPressed('s')) return "C_BK";
    if (M5Cardputer.Keyboard.isKeyPressed('D') || M5Cardputer.Keyboard.isKeyPressed('d')) return "C_RS";
    if (M5Cardputer.Keyboard.isKeyPressed('K') || M5Cardputer.Keyboard.isKeyPressed('k')) return "C_TL";
    if (M5Cardputer.Keyboard.isKeyPressed('L') || M5Cardputer.Keyboard.isKeyPressed('l')) return "C_TR";
    return "C_ST";
}

String AppRemote::pressed_keys_str() const {
    String out;
    uint8_t m = compute_keys_mask();
    if (m & 0x01) out += "E";
    if (m & 0x02) out += "A";
    if (m & 0x04) out += "S";
    if (m & 0x08) out += "D";
    if (m & 0x10) out += "K";
    if (m & 0x20) out += "L";
    return out;
}

uint8_t AppRemote::compute_keys_mask() const {
    uint8_t m = 0;
    if (M5Cardputer.Keyboard.isKeyPressed('E') || M5Cardputer.Keyboard.isKeyPressed('e')) m |= 0x01;
    if (M5Cardputer.Keyboard.isKeyPressed('A') || M5Cardputer.Keyboard.isKeyPressed('a')) m |= 0x02;
    if (M5Cardputer.Keyboard.isKeyPressed('S') || M5Cardputer.Keyboard.isKeyPressed('s')) m |= 0x04;
    if (M5Cardputer.Keyboard.isKeyPressed('D') || M5Cardputer.Keyboard.isKeyPressed('d')) m |= 0x08;
    if (M5Cardputer.Keyboard.isKeyPressed('K') || M5Cardputer.Keyboard.isKeyPressed('k')) m |= 0x10;
    if (M5Cardputer.Keyboard.isKeyPressed('L') || M5Cardputer.Keyboard.isKeyPressed('l')) m |= 0x20;
    return m;
}
