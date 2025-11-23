#pragma once
#include "window_system.h"
#include "theme.h"
#include <lvgl.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <NimBLEDevice.h>

class AppBluetooth : public IApp {
public:
    AppBluetooth();
    ~AppBluetooth() override;

    const char* title() const override { return "Bluetooth"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Light; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override;
    void onClose() override;

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* label_name_ = nullptr;
    lv_obj_t* ta_name_ = nullptr;
    lv_obj_t* btn_start_ = nullptr;
    lv_obj_t* btn_label_ = nullptr;
    lv_obj_t* label_status_ = nullptr;

    QueueHandle_t event_queue_ = nullptr;

    bool initialized_ = false;
    bool advertising_ = false;
    NimBLEServer* server_ = nullptr;
    NimBLEService* service_ = nullptr;
    NimBLECharacteristic* ch_ = nullptr;

    void start_ble(const char* name);
    void stop_ble();
    void update_ui_running(bool running);
    static void on_btn_event(lv_event_t* e);
public:
    void enqueue_event(int type);

    struct BleEvent { int type; };

    static AppBluetooth* s_instance_;
};
