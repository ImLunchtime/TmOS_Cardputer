#include "apps/ux_executor/app_ux_executor.h"
#include "apps/ux_runner/app_ux_runner.h"
#include "ui/theme.h"
#include <vector>
#include <unordered_map>
#include "ui/ui_notify.h"
#include "drivers/input_kb.h"

static void on_item_clicked(lv_event_t* e) {
    AppUXExecutor* app = (AppUXExecutor*)lv_event_get_user_data(e);
    if (!app) return;
    lv_obj_t* btn = lv_event_get_target(e);
    app->onItemClicked(btn);
}

static bool app_uxexec_is_focusable(lv_obj_t* obj) {
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

static void app_uxexec_add_focusables_recursive(lv_obj_t* node, lv_group_t* group) {
    if (!node || !group) return;
    if (app_uxexec_is_focusable(node)) lv_group_add_obj(group, node);
    uint32_t child_cnt = lv_obj_get_child_cnt(node);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(node, i);
        app_uxexec_add_focusables_recursive(child, group);
    }
}

static lv_obj_t* app_uxexec_find_first_focusable(lv_obj_t* node) {
    if (!node) return nullptr;
    if (app_uxexec_is_focusable(node)) return node;
    uint32_t child_cnt = lv_obj_get_child_cnt(node);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(node, i);
        lv_obj_t* res = app_uxexec_find_first_focusable(child);
        if (res) return res;
    }
    return nullptr;
}

void AppUXExecutor::refresh_list() {
    if (!fm_.initialize()) { ui_notify::showSymbol(LV_SYMBOL_WARNING, "SD初始化失败", 2500); return; }
    files_.clear();
    item_index_.clear();
    if (!list_) return;
    lv_obj_clean(list_);
    ui_theme::apply_list_menu(list_);
    if (status_label_) {
        lv_obj_del(status_label_);
        status_label_ = nullptr;
    }
    status_label_ = lv_label_create(list_);
    lv_label_set_text(status_label_, "Scanning Files");
    ui_notify::showSymbol(LV_SYMBOL_REFRESH, "Scanning Files", 1000);
    scanning_ = fm_.beginScan("/", ".uxc");
    if (!scanning_) {
        lv_label_set_text(status_label_, "Failed to scan files");
        rebuildFocusGroup();
    }
}

void AppUXExecutor::run_current() { /* 保留接口以兼容，但当前通过列表项点击执行 */ }

void AppUXExecutor::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_set_style_pad_row(root_, 0, 0);
    list_ = lv_list_create(root_);
    lv_obj_set_size(list_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(list_, 1);
    lv_obj_add_flag(list_, LV_OBJ_FLAG_SCROLLABLE);
    ui_theme::apply_list_menu(list_);
    lv_obj_set_style_pad_row(list_, 0, 0);
    refresh_list();
}

void AppUXExecutor::onTick() {
    if (!scanning_) return;
    const int kBatch = 8;
    FileInfo tmp[kBatch];
    int cnt = 0;
    bool cont = fm_.stepScan(tmp, cnt, kBatch);
    for (int i = 0; i < cnt; ++i) {
        files_.push_back(tmp[i]);
    }
    if (!cont) {
        scanning_ = false;
        if (status_label_) {
            lv_obj_del(status_label_);
            status_label_ = nullptr;
        }
        if (!list_) return;
        lv_obj_clean(list_);
        ui_theme::apply_list_menu(list_);
        item_index_.clear();
        for (size_t i = 0; i < files_.size(); ++i) {
            const auto& f = files_[i];
            lv_obj_t* btn = lv_list_add_btn(list_, LV_SYMBOL_FILE, f.name.c_str());
            ui_theme::apply_list_menu_item(btn);
            lv_obj_set_style_min_height(btn, 18, 0);
            lv_obj_set_style_pad_top(btn, 1, 0);
            lv_obj_set_style_pad_bottom(btn, 1, 0);
            lv_obj_set_style_pad_left(btn, 4, 0);
            lv_obj_set_style_pad_right(btn, 4, 0);
            lv_obj_add_event_cb(btn, on_item_clicked, LV_EVENT_CLICKED, this);
            item_index_[btn] = (int)i;
        }
        rebuildFocusGroup();
    }
}

void AppUXExecutor::onItemClicked(lv_obj_t* btn) {
    auto it = item_index_.find(btn);
    if (it == item_index_.end()) return;
    int idx = it->second;
    if (idx < 0 || idx >= (int)files_.size()) return;
    String path = files_[idx].path;
    wm_.openApp(std::unique_ptr<IApp>(new AppUXRunner(wm_, path)));
}

void AppUXExecutor::rebuildFocusGroup() {
    lv_group_t* grp = kb_get_current_group();
    if (!grp) return;
    lv_group_remove_all_objs(grp);
    if (root_) app_uxexec_add_focusables_recursive(root_, grp);
    lv_obj_t* focus = nullptr;
    if (list_ && !lv_obj_has_flag(list_, LV_OBJ_FLAG_HIDDEN)) {
        uint32_t cnt = lv_obj_get_child_cnt(list_);
        for (uint32_t i = 0; i < cnt; ++i) {
            lv_obj_t* c = lv_obj_get_child(list_, i);
            if (app_uxexec_is_focusable(c)) {
                focus = c;
                break;
            }
        }
    }
    if (!focus && root_) focus = app_uxexec_find_first_focusable(root_);
    if (focus) lv_group_focus_obj(focus);
}
