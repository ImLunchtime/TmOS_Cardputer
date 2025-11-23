#include "app_devices.h"

AppDevices* AppDevices::s_instance_ = nullptr;

AppDevices::AppDevices() {}
AppDevices::~AppDevices() {}

void AppDevices::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 6, 0);
    lv_obj_set_style_pad_row(root_, 6, 0);

    list_ = lv_list_create(root_);
    lv_obj_set_size(list_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(list_, 1);
    ui_theme::apply_list_menu(list_);

    event_queue_ = xQueueCreate(8, sizeof(DeviceEvent));
    s_instance_ = this;

    WiFi.mode(WIFI_STA);
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    esp_now_init();
    esp_now_register_recv_cb(on_recv);
}

void AppDevices::onTick() {
    if (!event_queue_) return;
    DeviceEvent ev;
    while (xQueueReceive(event_queue_, &ev, 0) == pdPASS) {
        handle_event(ev);
    }
}

void AppDevices::onClose() {
    if (event_queue_) { vQueueDelete(event_queue_); event_queue_ = nullptr; }
    esp_now_deinit();
    s_instance_ = nullptr;
}

bool AppDevices::mac_known(const uint8_t* mac) const {
    for (auto& m : macs_) {
        if (memcmp(m.data(), mac, 6) == 0) return true;
    }
    return false;
}

void AppDevices::handle_event(const DeviceEvent& ev) {
    if (!mac_known(ev.mac)) {
        std::array<uint8_t,6> m; memcpy(m.data(), ev.mac, 6);
        macs_.push_back(m);
        names_.emplace_back(ev.name);
        lv_obj_t* item = lv_list_add_btn(list_, NULL, ev.name);
        ui_theme::apply_small_text_recursive(item);
        lv_obj_set_style_min_height(item, 14, 0);
        lv_obj_set_style_pad_top(item, 1, 0);
        lv_obj_set_style_pad_bottom(item, 1, 0);
        items_.push_back(item);
    }
}

void AppDevices::on_recv(const uint8_t* mac, const uint8_t* data, int len) {
    if (!s_instance_ || !s_instance_->event_queue_) return;
    DeviceEvent ev;
    memcpy(ev.mac, mac, 6);
    int n = len;
    if (n > 31) n = 31;
    if (n < 0) n = 0;
    if (n > 0) memcpy(ev.name, data, n);
    ev.name[n] = '\0';
    xQueueSend(s_instance_->event_queue_, &ev, 0);
}
