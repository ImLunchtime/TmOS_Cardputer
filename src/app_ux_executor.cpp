#include "app_ux_executor.h"
#include "app_ux_runner.h"
#include "theme.h"
#include <vector>
#include <unordered_map>

static void on_item_clicked(lv_event_t* e) {
    AppUXExecutor* app = (AppUXExecutor*)lv_event_get_user_data(e);
    if (!app) return;
    lv_obj_t* btn = lv_event_get_target(e);
    app->onItemClicked(btn);
}

void AppUXExecutor::refresh_list() {
    if (!fm_.initialize()) return;
    files_.clear();
    const int MAX = 256;
    std::vector<FileInfo> tmp(MAX);
    int cnt = 0;
    fm_.scanAllFiles(tmp.data(), cnt, MAX, String(".uxc"));
    for (int i = 0; i < cnt; ++i) files_.push_back(tmp[i]);
    item_index_.clear();
    if (!list_) return;
    lv_obj_clean(list_);
    ui_theme::apply_list_menu(list_);
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

void AppUXExecutor::onItemClicked(lv_obj_t* btn) {
    auto it = item_index_.find(btn);
    if (it == item_index_.end()) return;
    int idx = it->second;
    if (idx < 0 || idx >= (int)files_.size()) return;
    String path = files_[idx].path;
    wm_.openApp(std::unique_ptr<IApp>(new AppUXRunner(path)));
}
