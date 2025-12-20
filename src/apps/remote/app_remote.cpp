#include "apps/remote/app_remote.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <M5Cardputer.h>
#include "services/wifi_manager.h"

LV_IMG_DECLARE(remote_microcar);

#if 0
void AppRemote::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(root_, 6, 0);

    page_main_ = lv_obj_create(root_);
    lv_obj_set_size(page_main_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(page_main_, 1);
    lv_obj_set_style_border_width(page_main_, 0, 0);
    lv_obj_set_style_radius(page_main_, 0, 0);
    lv_obj_set_style_bg_opa(page_main_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page_main_, 0, 0);
    lv_obj_set_style_pad_row(page_main_, 6, 0);
    lv_obj_set_flex_flow(page_main_, LV_FLEX_FLOW_COLUMN);


    lv_obj_t* list_row = lv_obj_create(page_main_);
    lv_obj_set_width(list_row, LV_PCT(100));
    lv_obj_set_height(list_row, 74);
    lv_obj_set_style_border_width(list_row, 0, 0);
    lv_obj_set_style_radius(list_row, 0, 0);
    lv_obj_set_style_bg_opa(list_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(list_row, 0, 0);
    lv_obj_set_style_pad_column(list_row, 8, 0);
    lv_obj_set_flex_flow(list_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(list_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(list_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(list_row, LV_DIR_HOR);
    lv_obj_set_scrollbar_mode(list_row, LV_SCROLLBAR_MODE_OFF);

    btn_microcar_ = lv_btn_create(list_row);
    lv_obj_set_size(btn_microcar_, 64, 64);
    lv_obj_add_flag(btn_microcar_, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_set_style_radius(btn_microcar_, 8, 0);
    lv_obj_set_style_bg_color(btn_microcar_, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_bg_opa(btn_microcar_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_microcar_, 0, 0);
    lv_obj_set_style_shadow_width(btn_microcar_, 0, 0);
    lv_obj_set_style_outline_width(btn_microcar_, 0, 0);
    lv_obj_set_style_pad_all(btn_microcar_, 4, 0);
    lv_obj_set_flex_flow(btn_microcar_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_microcar_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_event_cb(btn_microcar_, on_remote_item_clicked, LV_EVENT_CLICKED, this);
    item_index_[btn_microcar_] = 0;
    {
        lv_obj_t* img = lv_img_create(btn_microcar_);
        lv_img_set_src(img, &remote_microcar);
        lv_obj_set_style_pad_bottom(img, 2, 0);

        lv_obj_t* lbl = lv_label_create(btn_microcar_);
        lv_label_set_text(lbl, "ESP-NOW Car");
        lv_obj_set_width(lbl, 60);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xEEEEEE), 0);
    }

    page_microcar_ = lv_obj_create(root_);
    lv_obj_set_size(page_microcar_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(page_microcar_, 1);
    lv_obj_set_style_border_width(page_microcar_, 0, 0);
    lv_obj_set_style_radius(page_microcar_, 0, 0);
    lv_obj_set_style_bg_opa(page_microcar_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page_microcar_, 0, 0);
    lv_obj_set_style_pad_row(page_microcar_, 6, 0);
    lv_obj_set_flex_flow(page_microcar_, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(page_microcar_, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t* top_row = lv_obj_create(page_microcar_);
    lv_obj_set_width(top_row, LV_PCT(100));
    lv_obj_set_height(top_row, 26);
    lv_obj_set_style_border_width(top_row, 0, 0);
    lv_obj_set_style_bg_opa(top_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(top_row, 0, 0);
    lv_obj_set_style_pad_column(top_row, 6, 0);
    lv_obj_set_flex_flow(top_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    btn_back_ = lv_btn_create(top_row);
    lv_obj_set_size(btn_back_, 24, 24);
    lv_obj_add_flag(btn_back_, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_set_style_radius(btn_back_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_back_, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_bg_opa(btn_back_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_back_, 0, 0);
    lv_obj_set_style_shadow_width(btn_back_, 0, 0);
    lv_obj_set_style_outline_width(btn_back_, 0, 0);
    lv_obj_add_event_cb(btn_back_, on_back_clicked, LV_EVENT_CLICKED, this);
    { lv_obj_t* l = lv_label_create(btn_back_); lv_label_set_text(l, LV_SYMBOL_LEFT); lv_obj_center(l); }


    info_box_ = lv_obj_create(page_microcar_);
    lv_obj_set_size(info_box_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(info_box_, 1);
    lv_obj_set_style_border_width(info_box_, 0, 0);
    lv_obj_set_style_bg_opa(info_box_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(info_box_, 6, 0);
    lv_obj_set_style_pad_row(info_box_, 6, 0);
    lv_obj_set_style_pad_column(info_box_, 6, 0);
    lv_obj_set_flex_flow(info_box_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(info_box_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(info_box_, LV_OBJ_FLAG_SCROLLABLE);
    {
        label_status_ = lv_label_create(info_box_);
        lv_label_set_text(label_status_, "ESP-NOW: Idle");
        lv_obj_set_style_text_color(label_status_, lv_color_hex(0xEEEEEE), 0);
    }

    rebuildFocusFor(page_main_, btn_microcar_);
}
#endif

#if 0
void AppRemote::onClose() {
    item_index_.clear();
    root_ = nullptr;
    page_main_ = nullptr;
    page_microcar_ = nullptr;
    btn_microcar_ = nullptr;
    btn_back_ = nullptr;
    unregister_controls();
    deinit_microcar_radio();
}

void AppRemote::showMain() {
    if (page_microcar_) lv_obj_add_flag(page_microcar_, LV_OBJ_FLAG_HIDDEN);
    if (page_main_) lv_obj_clear_flag(page_main_, LV_OBJ_FLAG_HIDDEN);
    rebuildFocusFor(page_main_, btn_microcar_);
    unregister_controls();
    kb_set_nav_disabled(false);
}

void AppRemote::showMicrocar() {
    if (page_main_) lv_obj_add_flag(page_main_, LV_OBJ_FLAG_HIDDEN);
    if (page_microcar_) lv_obj_clear_flag(page_microcar_, LV_OBJ_FLAG_HIDDEN);
    rebuildFocusFor(page_microcar_, btn_back_);
    init_microcar_radio();
    register_controls();
    kb_set_nav_disabled(true);
}

void AppRemote::on_remote_item_clicked(lv_event_t* e) {
    auto* app = static_cast<AppRemote*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    lv_obj_t* tgt = lv_event_get_target(e);
    auto it = app->item_index_.find(tgt);
    if (it == app->item_index_.end()) return;
    if (it->second == 0) app->showMicrocar();
}

void AppRemote::on_back_clicked(lv_event_t* e) {
    auto* app = static_cast<AppRemote*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) app->showMain();
}

void AppRemote::onTick() {
    if (!page_microcar_ || lv_obj_has_flag(page_microcar_, LV_OBJ_FLAG_HIDDEN)) return;
    uint8_t mask = compute_keys_mask();
    unsigned long now = millis();
    if (mask != 0) {
        const char* cmd = decide_cmd_from_keys();
        if (strcmp(last_cmd_, cmd) != 0 || now - last_sent_ms_ >= 100) {
            send_cmd(cmd);
        }
    } else {
        if (keys_mask_prev_ != 0) {
            send_cmd("C_ST");
        }
    }
    keys_mask_prev_ = mask;
}

void AppRemote::rebuildFocusFor(lv_obj_t* container, lv_obj_t* preferred_focus) {
    lv_group_t* grp = kb_get_current_group();
    if (!grp || !container) return;
    lv_group_remove_all_objs(grp);
    add_focusables_recursive(container, grp);
    if (preferred_focus && is_focusable(preferred_focus)) {
        lv_group_focus_obj(preferred_focus);
        return;
    }
    lv_obj_t* first = find_first_focusable_local(container);
    if (first) lv_group_focus_obj(first);
}

bool AppRemote::is_focusable(lv_obj_t* obj) {
    if (!lv_obj_is_valid(obj)) return false;
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) return false;
    if (lv_obj_has_class(obj, &lv_btn_class)) return true;
    if (lv_obj_has_class(obj, &lv_textarea_class)) return true;
    if (lv_obj_has_class(obj, &lv_dropdown_class)) return true;
    if (lv_obj_has_class(obj, &lv_checkbox_class)) return true;
    if (lv_obj_has_class(obj, &lv_slider_class)) return true;
    if (lv_obj_has_class(obj, &lv_switch_class)) return true;
    if (lv_obj_has_class(obj, &lv_spinbox_class)) return true;
    if (lv_obj_has_class(obj, &lv_roller_class)) return true;
    if (lv_obj_has_class(obj, &lv_img_class) && lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_1)) return true;
    if (lv_obj_is_editable(obj)) return true;
    return false;
}

void AppRemote::add_focusables_recursive(lv_obj_t* node, lv_group_t* group) {
    if (!node || !group) return;
    if (is_focusable(node)) lv_group_add_obj(group, node);
    uint32_t child_cnt = lv_obj_get_child_cnt(node);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(node, i);
        add_focusables_recursive(child, group);
    }
}

lv_obj_t* AppRemote::find_first_focusable_local(lv_obj_t* node) {
    if (!node) return nullptr;
    if (is_focusable(node)) return node;
    uint32_t child_cnt = lv_obj_get_child_cnt(node);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(node, i);
        lv_obj_t* res = find_first_focusable_local(child);
        if (res) return res;
    }
    return nullptr;
}

void AppRemote::register_controls() {
    kb_register_app_keys(this, {
        { 'E', [this](){ return true; }, [this](){ send_cmd("C_FD"); } },
        { 'e', [this](){ return true; }, [this](){ send_cmd("C_FD"); } },
        { 'A', [this](){ return true; }, [this](){ send_cmd("C_LS"); } },
        { 'a', [this](){ return true; }, [this](){ send_cmd("C_LS"); } },
        { 'S', [this](){ return true; }, [this](){ send_cmd("C_BK"); } },
        { 's', [this](){ return true; }, [this](){ send_cmd("C_BK"); } },
        { 'D', [this](){ return true; }, [this](){ send_cmd("C_RS"); } },
        { 'd', [this](){ return true; }, [this](){ send_cmd("C_RS"); } },
        { 'K', [this](){ return true; }, [this](){ send_cmd("C_TL"); } },
        { 'k', [this](){ return true; }, [this](){ send_cmd("C_TL"); } },
        { 'L', [this](){ return true; }, [this](){ send_cmd("C_TR"); } },
        { 'l', [this](){ return true; }, [this](){ send_cmd("C_TR"); } },
    });
}

void AppRemote::unregister_controls() {
    kb_clear_app_keys(this);
}

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
    if (label_cmd_) {
        char buf[24];
        snprintf(buf, sizeof(buf), "Cmd: %s", cmd ? cmd : "-");
        lv_label_set_text(label_cmd_, buf);
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
#endif
