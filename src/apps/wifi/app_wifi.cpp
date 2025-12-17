#include "apps/wifi/app_wifi.h"
#include "ui/theme.h"
#include "ui/ui_notify.h"
#include "drivers/input_kb.h"
#include "services/config_manager.h"

AppWiFi::AppWiFi() {}
AppWiFi::~AppWiFi() {}

static void on_refresh_event(lv_event_t* e) { AppWiFi* app = (AppWiFi*)lv_event_get_user_data(e); if (app) AppWiFi::on_refresh(e); }
static void on_list_item_event(lv_event_t* e) { AppWiFi* app = (AppWiFi*)lv_event_get_user_data(e); if (app) AppWiFi::on_list_item(e); }
static void on_connect_event(lv_event_t* e) { AppWiFi* app = (AppWiFi*)lv_event_get_user_data(e); if (app) AppWiFi::on_connect(e); }
static void on_cancel_event(lv_event_t* e) { AppWiFi* app = (AppWiFi*)lv_event_get_user_data(e); if (app) AppWiFi::on_cancel(e); }

void AppWiFi::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 6, 0);
    lv_obj_set_style_pad_row(root_, 4, 0);
    ui_theme::apply_small_text_recursive(root_);

    label_status_ = lv_label_create(root_);
    lv_obj_add_flag(label_status_, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(label_status_, LV_ALIGN_TOP_RIGHT, -4, 4);
    lv_obj_set_style_text_font(label_status_, ui_theme::get_system_font(), 0);
    lv_label_set_text(label_status_, "WiFi: Idle");

    list_ = lv_list_create(root_);
    lv_obj_set_size(list_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(list_, 1);
    ui_theme::apply_list_menu(list_);

    build_connect_panel();
    build_connected_panel();

    btn_refresh_ = lv_btn_create(root_);
    lv_obj_add_flag(btn_refresh_, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(btn_refresh_, LV_ALIGN_BOTTOM_RIGHT, -4, -4);
    lv_obj_set_size(btn_refresh_, 24, 24);
    lv_obj_set_style_radius(btn_refresh_, LV_RADIUS_CIRCLE, 0);
    ui_theme::apply_button(btn_refresh_);
    { lv_obj_t* l = lv_label_create(btn_refresh_); lv_label_set_text(l, LV_SYMBOL_REFRESH); lv_obj_center(l); }
    lv_obj_add_event_cb(btn_refresh_, on_refresh_event, LV_EVENT_CLICKED, this);

    if (wifi_manager::is_connected()) {
        String msg = String("Connected: ") + wifi_manager::connected_ssid();
        lv_label_set_text(label_status_, msg.c_str());
        show_connected_panel();
    } else {
        lv_label_set_text(label_status_, "Scanning...");
        timer_scan_ = lv_timer_create(AppWiFi::on_scan_timer, 10, this);
    }
}

void AppWiFi::onTick() {
    auto st = wifi_manager::get_state();
    if (st == wifi_manager::State::Connected) {
        String msg = String("Connected: ") + wifi_manager::connected_ssid();
        lv_label_set_text(label_status_, msg.c_str());
        if (connecting_) {
            ui_notify::showSymbol(LV_SYMBOL_OK, "WiFi connected", 2000);
            config_manager::save_wifi(wifi_manager::connected_ssid().c_str(), last_password_.c_str());
            connecting_ = false;
        }
    } else if (st == wifi_manager::State::Failed) {
        String msg = String("Failed: ") + wifi_manager::last_error();
        lv_label_set_text(label_status_, msg.c_str());
        if (connecting_) {
            ui_notify::showSymbol(LV_SYMBOL_WARNING, msg.c_str(), 2500);
            connecting_ = false;
        }
    } else if (st == wifi_manager::State::Connecting) {
        lv_label_set_text(label_status_, "Connecting...");
    } else if (st == wifi_manager::State::Scanning) {
        lv_label_set_text(label_status_, "Scanning...");
    } else {
        if (wifi_manager::is_connected()) {
            String msg = String("Connected: ") + wifi_manager::connected_ssid();
            lv_label_set_text(label_status_, msg.c_str());
        } else {
            lv_label_set_text(label_status_, "WiFi: Idle");
        }
    }

    (void)st;
}

void AppWiFi::onClose() {
    kb_set_nav_disabled(false);
}

void AppWiFi::refresh_scan() {
    lv_label_set_text(label_status_, "Scanning...");
    wifi_manager::start_scan();
    networks_ = wifi_manager::take_scan_results();
    populate_list();
    char buf[32]; snprintf(buf, sizeof(buf), "Found %u", (unsigned)networks_.size());
    lv_label_set_text(label_status_, buf);
}

void AppWiFi::build_connect_panel() {
    panel_connect_ = lv_obj_create(root_);
    lv_obj_set_size(panel_connect_, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(panel_connect_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(panel_connect_, 0, 0);
    lv_obj_set_style_pad_all(panel_connect_, 0, 0);
    lv_obj_set_style_pad_row(panel_connect_, 4, 0);
    lv_obj_set_flex_flow(panel_connect_, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(panel_connect_, LV_OBJ_FLAG_HIDDEN);

    label_ssid_ = lv_label_create(panel_connect_);
    lv_obj_set_style_text_font(label_ssid_, ui_theme::get_system_font(), 0);
    lv_label_set_text(label_ssid_, "");

    ta_password_ = lv_textarea_create(panel_connect_);
    lv_obj_set_width(ta_password_, LV_PCT(100));
    lv_textarea_set_one_line(ta_password_, true);
    lv_textarea_set_password_mode(ta_password_, true);
    lv_textarea_set_placeholder_text(ta_password_, "Password");
    lv_obj_set_style_text_font(ta_password_, ui_theme::get_system_font(), 0);

    lv_obj_t* row = lv_obj_create(panel_connect_);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_column(row, 6, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);

    btn_connect_ = lv_btn_create(row);
    ui_theme::apply_button(btn_connect_);
    { lv_obj_t* l = lv_label_create(btn_connect_); lv_label_set_text(l, "Connect"); lv_obj_center(l); }
    lv_obj_add_event_cb(btn_connect_, on_connect_event, LV_EVENT_CLICKED, this);

    btn_cancel_ = lv_btn_create(row);
    ui_theme::apply_button(btn_cancel_);
    { lv_obj_t* l = lv_label_create(btn_cancel_); lv_label_set_text(l, "Cancel"); lv_obj_center(l); }
    lv_obj_add_event_cb(btn_cancel_, on_cancel_event, LV_EVENT_CLICKED, this);
}

void AppWiFi::build_connected_panel() {
    panel_connected_ = lv_obj_create(root_);
    lv_obj_set_size(panel_connected_, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(panel_connected_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(panel_connected_, 0, 0);
    lv_obj_set_style_pad_all(panel_connected_, 0, 0);
    lv_obj_set_style_pad_row(panel_connected_, 4, 0);
    lv_obj_set_flex_flow(panel_connected_, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(panel_connected_, LV_OBJ_FLAG_HIDDEN);

    label_connected_msg_ = lv_label_create(panel_connected_);
    lv_obj_set_style_text_font(label_connected_msg_, ui_theme::get_system_font(), 0);
    lv_label_set_text(label_connected_msg_, "WiFi is already connected, do you want to disconnect first?");

    lv_obj_t* row = lv_obj_create(panel_connected_);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_column(row, 6, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);

    btn_disc_confirm_ = lv_btn_create(row);
    ui_theme::apply_button(btn_disc_confirm_);
    { lv_obj_t* l = lv_label_create(btn_disc_confirm_); lv_label_set_text(l, "Disconnect"); lv_obj_center(l); }
    lv_obj_add_event_cb(btn_disc_confirm_, on_disc_confirm, LV_EVENT_CLICKED, this);

    btn_disc_cancel_ = lv_btn_create(row);
    ui_theme::apply_button(btn_disc_cancel_);
    { lv_obj_t* l = lv_label_create(btn_disc_cancel_); lv_label_set_text(l, "Cancel"); lv_obj_center(l); }
    lv_obj_add_event_cb(btn_disc_cancel_, on_disc_cancel, LV_EVENT_CLICKED, this);
}

void AppWiFi::show_connect_panel(const String& ssid, bool secure) {
    selected_ssid_ = ssid;
    String t = String("SSID: ") + ssid;
    lv_label_set_text(label_ssid_, t.c_str());
    if (secure) {
        lv_textarea_set_text(ta_password_, "");
        lv_obj_clear_flag(panel_connect_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(panel_connect_, LV_OBJ_FLAG_FLOATING);
        lv_obj_align(panel_connect_, LV_ALIGN_TOP_MID, 0, 18);
        lv_obj_move_foreground(panel_connect_);
        kb_set_nav_disabled(true);
        rebuildFocusGroup();
        lv_group_focus_obj(ta_password_);
    } else {
        lv_obj_add_flag(panel_connect_, LV_OBJ_FLAG_HIDDEN);
    }
}

void AppWiFi::hide_connect_panel() {
    lv_obj_add_flag(panel_connect_, LV_OBJ_FLAG_HIDDEN);
    kb_set_nav_disabled(false);
}

void AppWiFi::show_connected_panel() {
    lv_obj_clear_flag(panel_connected_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(panel_connected_, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(panel_connected_, LV_ALIGN_TOP_MID, 0, 18);
    lv_obj_move_foreground(panel_connected_);
    rebuildFocusGroup();
    if (btn_disc_confirm_) lv_group_focus_obj(btn_disc_confirm_);
}

void AppWiFi::hide_connected_panel() {
    lv_obj_add_flag(panel_connected_, LV_OBJ_FLAG_HIDDEN);
    rebuildFocusGroup();
}

void AppWiFi::populate_list() {
    if (!list_) return;
    lv_obj_clean(list_);
    ui_theme::apply_list_menu(list_);
    item_index_.clear();
    for (size_t i = 0; i < networks_.size(); ++i) {
        const auto& n = networks_[i];
        String txt = n.ssid.length() ? n.ssid : String("<hidden>");
        char rbuf[8]; snprintf(rbuf, sizeof(rbuf), " %ddBm", n.rssi);
        txt += rbuf;
        lv_obj_t* btn = lv_list_add_btn(list_, LV_SYMBOL_WIFI, txt.c_str());
        ui_theme::apply_list_menu_item(btn);
        lv_obj_set_style_min_height(btn, 18, 0);
        lv_obj_set_style_pad_top(btn, 1, 0);
        lv_obj_set_style_pad_bottom(btn, 1, 0);
        lv_obj_set_style_pad_left(btn, 4, 0);
        lv_obj_set_style_pad_right(btn, 4, 0);
        lv_obj_add_event_cb(btn, on_list_item_event, LV_EVENT_CLICKED, this);
        item_index_[btn] = (int)i;
    }
    rebuildFocusGroup();
}

void AppWiFi::on_refresh(lv_event_t* e) {
    AppWiFi* app = (AppWiFi*)lv_event_get_user_data(e);
    if (!app) return;
    app->hide_connect_panel();
    if (wifi_manager::is_connected()) {
        String msg = String("Connected: ") + wifi_manager::connected_ssid();
        lv_label_set_text(app->label_status_, msg.c_str());
        app->show_connected_panel();
    } else {
        lv_label_set_text(app->label_status_, "Scanning...");
        app->refresh_scan();
    }
}

void AppWiFi::on_scan_timer(lv_timer_t* t) {
    AppWiFi* app = (AppWiFi*)t->user_data;
    if (!app) return;
    if (app->timer_scan_) {
        lv_timer_del(app->timer_scan_);
        app->timer_scan_ = nullptr;
    }
    app->refresh_scan();
}

void AppWiFi::on_list_item(lv_event_t* e) {
    AppWiFi* app = (AppWiFi*)lv_event_get_user_data(e);
    if (!app) return;
    lv_obj_t* btn = lv_event_get_target(e);
    auto it = app->item_index_.find(btn);
    if (it == app->item_index_.end()) return;
    int idx = it->second;
    if (idx < 0 || idx >= (int)app->networks_.size()) return;
    const auto& n = app->networks_[idx];
    if (!n.ssid.length()) { ui_notify::showSymbol(LV_SYMBOL_WARNING, "Hidden SSID", 2000); return; }
    app->show_connect_panel(n.ssid, n.secure);
    if (!n.secure) {
        wifi_manager::connect(n.ssid.c_str(), "");
        app->last_password_ = "";
        app->connecting_ = true;
        ui_notify::showSymbol(LV_SYMBOL_WIFI, "Connecting...", 1500);
    }
}

void AppWiFi::on_connect(lv_event_t* e) {
    AppWiFi* app = (AppWiFi*)lv_event_get_user_data(e);
    if (!app) return;
    const char* pw = lv_textarea_get_text(app->ta_password_);
    app->last_password_ = pw ? pw : "";
    wifi_manager::connect(app->selected_ssid_.c_str(), pw);
    app->connecting_ = true;
    app->hide_connect_panel();
    ui_notify::showSymbol(LV_SYMBOL_WIFI, "Connecting...", 1500);
}

void AppWiFi::on_cancel(lv_event_t* e) {
    AppWiFi* app = (AppWiFi*)lv_event_get_user_data(e);
    if (!app) return;
    app->hide_connect_panel();
}

void AppWiFi::on_disc_confirm(lv_event_t* e) {
    AppWiFi* app = (AppWiFi*)lv_event_get_user_data(e);
    if (!app) return;
    wifi_manager::disconnect();
    app->hide_connected_panel();
    lv_label_set_text(app->label_status_, "Scanning...");
    app->refresh_scan();
}

void AppWiFi::on_disc_cancel(lv_event_t* e) {
    AppWiFi* app = (AppWiFi*)lv_event_get_user_data(e);
    if (!app) return;
    app->hide_connected_panel();
    String msg = String("Connected: ") + wifi_manager::connected_ssid();
    lv_label_set_text(app->label_status_, msg.c_str());
}

static lv_obj_t* find_first_focusable_local(lv_obj_t* root) {
    if (!root) return nullptr;
    if (lv_obj_is_valid(root) && !lv_obj_has_flag(root, LV_OBJ_FLAG_HIDDEN)) {
        if (lv_obj_has_class(root, &lv_btn_class)) return root;
        if (lv_obj_has_class(root, &lv_textarea_class)) return root;
        if (lv_obj_has_class(root, &lv_dropdown_class)) return root;
        if (lv_obj_has_class(root, &lv_checkbox_class)) return root;
        if (lv_obj_has_class(root, &lv_slider_class)) return root;
        if (lv_obj_has_class(root, &lv_switch_class)) return root;
        if (lv_obj_has_class(root, &lv_spinbox_class)) return root;
        if (lv_obj_has_class(root, &lv_roller_class)) return root;
        if (lv_obj_has_class(root, &lv_img_class) && lv_obj_has_flag(root, LV_OBJ_FLAG_USER_1)) return root;
        if (lv_obj_is_editable(root)) return root;
    }
    uint32_t child_cnt = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(root, i);
        lv_obj_t* res = find_first_focusable_local(child);
        if (res) return res;
    }
    return nullptr;
}

void AppWiFi::rebuildFocusGroup() {
    lv_group_t* grp = kb_get_current_group();
    if (!grp) return;
    lv_group_remove_all_objs(grp);
    lv_obj_t* focus = nullptr;
    bool connect_visible = panel_connect_ && !lv_obj_has_flag(panel_connect_, LV_OBJ_FLAG_HIDDEN);
    bool connected_visible = panel_connected_ && !lv_obj_has_flag(panel_connected_, LV_OBJ_FLAG_HIDDEN);
    if (connect_visible) {
        add_focusables_recursive(panel_connect_, grp);
        if (ta_password_ && is_focusable(ta_password_)) focus = ta_password_;
        if (!focus) focus = find_first_focusable_local(panel_connect_);
    } else if (connected_visible) {
        add_focusables_recursive(panel_connected_, grp);
        if (btn_disc_confirm_ && is_focusable(btn_disc_confirm_)) focus = btn_disc_confirm_;
        if (!focus) focus = find_first_focusable_local(panel_connected_);
    } else if (list_) {
        add_focusables_recursive(list_, grp);
        uint32_t cnt = lv_obj_get_child_cnt(list_);
        for (uint32_t i = 0; i < cnt; ++i) {
            lv_obj_t* c = lv_obj_get_child(list_, i);
            if (is_focusable(c)) { focus = c; break; }
        }
        if (!focus) focus = find_first_focusable_local(list_);
    } else if (root_) {
        add_focusables_recursive(root_, grp);
        focus = find_first_focusable_local(root_);
    }
    if (focus) lv_group_focus_obj(focus);
}

bool AppWiFi::is_focusable(lv_obj_t* obj) {
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

void AppWiFi::add_focusables_recursive(lv_obj_t* node, lv_group_t* group) {
    if (!node || !group) return;
    if (is_focusable(node)) lv_group_add_obj(group, node);
    uint32_t child_cnt = lv_obj_get_child_cnt(node);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(node, i);
        add_focusables_recursive(child, group);
    }
}
