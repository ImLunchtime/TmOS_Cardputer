#include "apps/remote/app_remote.h"
#include "drivers/input_kb.h"
#include <lvgl.h>
#include <M5Cardputer.h>

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
