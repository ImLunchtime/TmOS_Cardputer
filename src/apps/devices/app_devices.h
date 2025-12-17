#pragma once
#include <lvgl.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <vector>
#include <array>
#include <string>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "core/window_system.h"
#include "ui/theme.h"

class AppDevices : public IApp {
public:
    AppDevices();
    ~AppDevices() override;

    const char* title() const override { return "Devices"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Light; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override;
    void onClose() override;

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* list_ = nullptr;

    struct DeviceEvent { uint8_t mac[6]; char name[32]; };
    QueueHandle_t event_queue_ = nullptr;
    std::vector<std::array<uint8_t,6>> macs_;
    std::vector<std::string> names_;
    std::vector<lv_obj_t*> items_;

    static AppDevices* s_instance_;
    static void on_recv(const uint8_t* mac, const uint8_t* data, int len);
    void handle_event(const DeviceEvent& ev);
    bool mac_known(const uint8_t* mac) const;
};

