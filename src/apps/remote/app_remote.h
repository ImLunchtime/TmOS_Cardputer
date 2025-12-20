#pragma once
#include "core/window_system.h"
#include "drivers/input_kb.h"
#include "ui/theme.h"
#include <lvgl.h>
#include <unordered_map>
#include <Arduino.h>

class AppRemote : public IApp {
public:
    AppRemote() = default;
    ~AppRemote() override = default;

    const char* title() const override { return nullptr; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Dark; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override;
    void onClose() override;

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* page_main_ = nullptr;
    lv_obj_t* page_microcar_ = nullptr;
    lv_obj_t* btn_microcar_ = nullptr;
    lv_obj_t* btn_back_ = nullptr;
    lv_obj_t* info_box_ = nullptr;
    lv_obj_t* label_status_ = nullptr;
    lv_obj_t* label_cmd_ = nullptr;
    std::unordered_map<lv_obj_t*, int> item_index_;

    void showMain();
    void showMicrocar();
    void rebuildFocusFor(lv_obj_t* container, lv_obj_t* preferred_focus);
    void add_focusables_recursive(lv_obj_t* node, lv_group_t* group);
    bool is_focusable(lv_obj_t* obj);
    lv_obj_t* find_first_focusable_local(lv_obj_t* node);

    void register_controls();
    void unregister_controls();
    void init_microcar_radio();
    void deinit_microcar_radio();
    void send_cmd(const char* cmd);
    void update_status(const char* status, const char* cmd);
    const char* decide_cmd_from_keys() const;
    String pressed_keys_str() const;
    uint8_t compute_keys_mask() const;

    bool espnow_inited_ = false;
    unsigned long last_sent_ms_ = 0;
    const char* last_cmd_ = "C_ST";
    static constexpr unsigned long kIdleStopTimeoutMs_ = 250;
    uint8_t keys_mask_prev_ = 0;

    static void on_remote_item_clicked(lv_event_t* e);
    static void on_back_clicked(lv_event_t* e);
};
