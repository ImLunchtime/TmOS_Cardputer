/*
 * @Author: ImLunchtime knoxmedia@yeah.net
 * @Date: 2025-11-23 16:46:49
 * @LastEditors: ImLunchtime knoxmedia@yeah.net
 * @LastEditTime: 2025-11-23 19:53:19
 * @FilePath: \CardputerOS2_LVGL\src\apps/bluetooth/app_bluetooth.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once
#include "core/window_system.h"
#include "ui/theme.h"
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
    lv_obj_t* label_peer_ = nullptr;

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
    void enqueue_event(int type, const char* peer);

    struct BleEvent { int type; char peer[32]; };

    static AppBluetooth* s_instance_;
};
