#include "app_bluetooth.h"
#include <NimBLEDevice.h>
#include <string.h>

static class BleServerCallbacks : public NimBLEServerCallbacks {
public:
    void onConnect(NimBLEServer* pServer) override {
        if (!AppBluetooth::s_instance_) return;
        AppBluetooth::s_instance_->enqueue_event(1);
    }
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) override {
        if (!AppBluetooth::s_instance_ || !desc) return;
        NimBLEAddress addr(desc->peer_ota_addr);
        AppBluetooth::s_instance_->enqueue_event(1, addr.toString().c_str());
    }
    void onDisconnect(NimBLEServer* pServer) override {
        if (!AppBluetooth::s_instance_) return;
        AppBluetooth::s_instance_->enqueue_event(2);
    }
    
} s_server_callbacks;

AppBluetooth* AppBluetooth::s_instance_ = nullptr;

AppBluetooth::AppBluetooth() {}
AppBluetooth::~AppBluetooth() {}

void AppBluetooth::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 6, 0);
    lv_obj_set_style_pad_row(root_, 4, 0);
    ui_theme::apply_small_text_recursive(root_);

    label_name_ = lv_label_create(root_);
    lv_label_set_text(label_name_, "Device Name");
    lv_obj_set_style_text_font(label_name_, ui_theme::get_system_font(), 0);

    ta_name_ = lv_textarea_create(root_);
    lv_obj_set_width(ta_name_, lv_pct(85));
    lv_textarea_set_one_line(ta_name_, true);
    lv_textarea_set_placeholder_text(ta_name_, "Cardputer");
    lv_obj_set_style_text_font(ta_name_, ui_theme::get_system_font(), 0);

    btn_start_ = lv_btn_create(root_);
    ui_theme::apply_button(btn_start_);
    btn_label_ = lv_label_create(btn_start_);
    lv_label_set_text(btn_label_, "Start");
    lv_obj_center(btn_label_);
    lv_obj_add_event_cb(btn_start_, on_btn_event, LV_EVENT_CLICKED, this);

    label_status_ = lv_label_create(root_);
    lv_label_set_text(label_status_, "Stopped");
    lv_obj_set_style_text_font(label_status_, ui_theme::get_system_font(), 0);

    label_peer_ = lv_label_create(root_);
    lv_label_set_text(label_peer_, "Peer: -");
    lv_obj_set_style_text_font(label_peer_, ui_theme::get_system_font(), 0);

    event_queue_ = xQueueCreate(8, sizeof(BleEvent));
    s_instance_ = this;

    if (!initialized_) {
        NimBLEDevice::init("Cardputer");
        NimBLEDevice::setPower(ESP_PWR_LVL_P9);
        server_ = NimBLEDevice::createServer();
        server_->setCallbacks(&s_server_callbacks);
        service_ = server_->createService("f3bce0f0-0000-4a5f-bad4-12345678abcd");
        ch_ = service_->createCharacteristic("f3bce0f1-0000-4a5f-bad4-12345678abcd", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
        ch_->setValue("CardputerOS");
        service_->start();
        NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
        if (adv) {
            adv->setScanResponse(true);
        }
        initialized_ = true;
    }
}

void AppBluetooth::onTick() {
    if (!event_queue_) return;
    BleEvent ev;
    while (xQueueReceive(event_queue_, &ev, 0) == pdPASS) {
        if (label_status_) {
            if (ev.type == 1) lv_label_set_text(label_status_, "Connected");
            else if (ev.type == 2) lv_label_set_text(label_status_, "Waiting for connection");
        }
        if (label_peer_) {
            if (ev.type == 1) {
                if (ev.peer[0]) {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "Peer: %s", ev.peer);
                    lv_label_set_text(label_peer_, buf);
                } else {
                    lv_label_set_text(label_peer_, "Peer: connected");
                }
            } else if (ev.type == 2) {
                lv_label_set_text(label_peer_, "Peer: -");
            }
        }
    }
}

void AppBluetooth::onClose() {
    if (event_queue_) { vQueueDelete(event_queue_); event_queue_ = nullptr; }
    if (advertising_) {
        NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
        if (adv) adv->stop();
        advertising_ = false;
    }
    NimBLEDevice::deinit(true);
    s_instance_ = nullptr;
}

void AppBluetooth::start_ble(const char* name) {
    if (!initialized_) return;
    NimBLEDevice::setDeviceName(name && *name ? name : "Cardputer");
    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    if (!adv) return;
    adv->stop();
    NimBLEAdvertisementData advData;
    NimBLEAdvertisementData scanData;
    const char* nm = (name && *name) ? name : "Cardputer";
    advData.setFlags(0x06);
    advData.setName(nm);
    scanData.setName(nm);
    adv->setAdvertisementData(advData);
    adv->setScanResponseData(scanData);
    adv->setScanResponse(true);
    adv->start();
    advertising_ = true;
    update_ui_running(true);
    if (label_status_) lv_label_set_text(label_status_, "Waiting for connection");
}

void AppBluetooth::stop_ble() {
    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    if (adv) adv->stop();
    advertising_ = false;
    update_ui_running(false);
    if (label_status_) lv_label_set_text(label_status_, "Stopped");
    if (label_peer_) lv_label_set_text(label_peer_, "Peer: -");
}

void AppBluetooth::on_btn_event(lv_event_t* e) {
    auto* app = static_cast<AppBluetooth*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (app->advertising_) {
        app->stop_ble();
    } else {
        const char* txt = lv_textarea_get_text(app->ta_name_);
        app->start_ble(txt);
    }
}

void AppBluetooth::enqueue_event(int type) {
    if (!event_queue_) return;
    BleEvent ev; ev.type = type;
    ev.peer[0] = '\0';
    xQueueSend(event_queue_, &ev, 0);
}

void AppBluetooth::enqueue_event(int type, const char* peer) {
    if (!event_queue_) return;
    BleEvent ev; ev.type = type;
    if (peer) {
        strncpy(ev.peer, peer, sizeof(ev.peer)-1);
        ev.peer[sizeof(ev.peer)-1] = '\0';
    } else {
        ev.peer[0] = '\0';
    }
    xQueueSend(event_queue_, &ev, 0);
}

void AppBluetooth::update_ui_running(bool running) {
    if (!ta_name_ || !btn_label_) return;
    if (running) {
        lv_obj_add_state(ta_name_, LV_STATE_DISABLED);
        lv_label_set_text(btn_label_, "Stop");
    } else {
        lv_obj_clear_state(ta_name_, LV_STATE_DISABLED);
        lv_label_set_text(btn_label_, "Start");
    }
}
