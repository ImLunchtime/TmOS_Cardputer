#include "app_ux_editor.h"
#include <Arduino.h>
#include <vector>
#include <unordered_map>

static void on_new(lv_event_t* e) { auto* app = (AppUXEditor*)lv_event_get_user_data(e); if (app) app->create_new(); }
static void on_save(lv_event_t* e) { auto* app = (AppUXEditor*)lv_event_get_user_data(e); if (app) app->save_current(); }
static void on_delete(lv_event_t* e) { auto* app = (AppUXEditor*)lv_event_get_user_data(e); if (app) app->delete_current(); }
static void on_debug(lv_event_t* e) { auto* app = (AppUXEditor*)lv_event_get_user_data(e); if (app) app->debug_current(); }
static void on_list_click(lv_event_t* e) {
    auto* app = (AppUXEditor*)lv_event_get_user_data(e);
    if (!app) return;
    lv_obj_t* target = lv_event_get_target(e);
    app->on_list_item_clicked(target);
}
static void on_fab_new(lv_event_t* e) { auto* app = (AppUXEditor*)lv_event_get_user_data(e); if (!app) return; app->create_new(); app->show_edit_page(); }

void AppUXEditor::refresh_list() {
    if (!fm_.initialize()) return;
    const int MAX = 256;
    files_cache_.clear();
    files_cache_.resize(MAX);
    int cnt = 0;
    fm_.scanAllFiles(files_cache_.data(), cnt, MAX, String(".uxc"));
    files_cache_.resize(cnt);
    current_path_ = "";
    if (list_) {
        while (lv_obj_get_child_cnt(list_) > 0) {
            lv_obj_t* child = lv_obj_get_child(list_, 0);
            lv_obj_del(child);
        }
        item_index_.clear();
        for (int i = 0; i < cnt; ++i) {
            lv_obj_t* it = lv_list_add_btn(list_, NULL, files_cache_[i].name.c_str());
            ui_theme::apply_list_menu_item(it);
            lv_obj_add_event_cb(it, on_list_click, LV_EVENT_CLICKED, this);
            item_index_[it] = i;
            if (current_path_.length() == 0) current_path_ = files_cache_[i].path;
        }
        ui_theme::apply_list_menu(list_);
    }
}

void AppUXEditor::load_selected() {
    if (!fm_.initialize()) return;
    String target = current_path_;
    if (target.length() == 0) return;
    String content = fm_.readFile(target);
    lv_textarea_set_text(editor_, content.c_str());
}

void AppUXEditor::create_new() {
    if (!fm_.initialize()) return;
    String name = String("ux_") + String(millis()) + String(".uxc");
    String path = String("/") + name;
    String tpl = String("{\n")
        + String("  \"title\": \"Demo\",\n")
        + String("  \"layout\": { \"flow\": \"column\", \"pad\": 4 },\n")
        + String("  \"widgets\": [\n")
        + String("    { \"type\": \"label\", \"text\": \"Hello UXC\" },\n")
        + String("    { \"type\": \"button\", \"text\": \"Click\", \"on\": { \"click\": { \"action\": \"toast\", \"text\": \"Clicked!\" } } }\n")
        + String("  ]\n")
        + String("}\n");
    fm_.createFile(path, tpl);
    refresh_list();
    current_path_ = path;
    lv_textarea_set_text(editor_, tpl.c_str());
}

void AppUXEditor::save_current() {
    if (!fm_.initialize()) return;
    String content = String(lv_textarea_get_text(editor_));
    if (current_path_.length() == 0) {
        create_new();
        fm_.writeFile(current_path_, content);
    } else {
        fm_.writeFile(current_path_, content);
    }
}

void AppUXEditor::delete_current() {
    if (!fm_.initialize()) return;
    if (current_path_.length() == 0) return;
    fm_.deletePath(current_path_);
    current_path_ = "";
    lv_textarea_set_text(editor_, "");
    refresh_list();
}

void AppUXEditor::debug_current() {
    lv_obj_t* temp = lv_obj_create(root_);
    lv_obj_add_flag(temp, LV_OBJ_FLAG_HIDDEN);
    auto r = ux::build_from_string(String(lv_textarea_get_text(editor_)), temp);
    lv_obj_del(temp);
    if (status_label_) {
        if (r.ok) {
            lv_label_set_text(status_label_, "Syntax OK");
            lv_obj_set_style_text_color(status_label_, lv_color_hex(0x66CC66), 0);
        } else {
            String msg = String("Error: ") + r.error;
            lv_label_set_text(status_label_, msg.c_str());
            lv_obj_set_style_text_color(status_label_, lv_color_hex(0xFF6666), 0);
        }
    }
}

void AppUXEditor::show_edit_page() {
    if (page_list_ && page_edit_) {
        lv_obj_add_flag(page_list_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(page_edit_, LV_OBJ_FLAG_HIDDEN);
    }
    if (toolbar_) lv_obj_move_foreground(toolbar_);
    if (status_label_) lv_obj_move_foreground(status_label_);
}

void AppUXEditor::show_list_page() {
    if (page_list_ && page_edit_) {
        lv_obj_add_flag(page_edit_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(page_list_, LV_OBJ_FLAG_HIDDEN);
    }
}

void AppUXEditor::on_list_item_clicked(lv_obj_t* target) {
    auto it = item_index_.find(target);
    if (it == item_index_.end()) return;
    int idx = it->second;
    if (idx < 0 || idx >= (int)files_cache_.size()) return;
    current_path_ = files_cache_[idx].path;
    load_selected();
    show_edit_page();
}

void AppUXEditor::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_set_style_pad_row(root_, 0, 0);

    page_list_ = lv_obj_create(root_);
    lv_obj_set_size(page_list_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_border_width(page_list_, 0, 0);
    lv_obj_set_style_bg_opa(page_list_, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(page_list_, LV_FLEX_FLOW_COLUMN);

    list_ = lv_list_create(page_list_);
    lv_obj_set_size(list_, lv_pct(100), lv_pct(100));
    ui_theme::apply_list_menu(list_);

    fab_new_ = lv_btn_create(page_list_);
    lv_obj_set_size(fab_new_, 28, 28);
    lv_obj_add_flag(fab_new_, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(fab_new_, LV_ALIGN_BOTTOM_RIGHT, -6, -6);
    lv_obj_set_style_radius(fab_new_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(fab_new_, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_bg_opa(fab_new_, LV_OPA_COVER, 0);
    { lv_obj_t* l = lv_label_create(fab_new_); lv_label_set_text(l, LV_SYMBOL_PLUS); lv_obj_center(l); }
    lv_obj_add_event_cb(fab_new_, on_fab_new, LV_EVENT_CLICKED, this);

    page_edit_ = lv_obj_create(root_);
    lv_obj_set_size(page_edit_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_border_width(page_edit_, 0, 0);
    lv_obj_set_style_bg_opa(page_edit_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page_edit_, 0, 0);
    lv_obj_set_flex_flow(page_edit_, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(page_edit_, LV_OBJ_FLAG_HIDDEN);

    toolbar_ = lv_obj_create(page_edit_);
    lv_obj_set_style_border_width(toolbar_, 0, 0);
    lv_obj_set_style_bg_opa(toolbar_, LV_OPA_60, 0);
    lv_obj_set_style_bg_color(toolbar_, lv_color_hex(0x1E1E1E), 0);
    lv_obj_set_style_pad_row(toolbar_, 0, 0);
    lv_obj_set_style_pad_column(toolbar_, 4, 0);
    lv_obj_set_style_pad_all(toolbar_, 4, 0);
    lv_obj_add_flag(toolbar_, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(toolbar_, LV_ALIGN_TOP_LEFT, 2, 2);
    lv_obj_set_flex_flow(toolbar_, LV_FLEX_FLOW_ROW);
    lv_obj_move_foreground(toolbar_);

    btn_new_ = lv_btn_create(toolbar_);
    lv_obj_set_size(btn_new_, 24, 24);
    lv_obj_set_style_radius(btn_new_, LV_RADIUS_CIRCLE, 0);
    { lv_obj_t* l = lv_label_create(btn_new_); lv_label_set_text(l, LV_SYMBOL_PLUS); lv_obj_center(l); }
    btn_save_ = lv_btn_create(toolbar_);
    lv_obj_set_size(btn_save_, 24, 24);
    lv_obj_set_style_radius(btn_save_, LV_RADIUS_CIRCLE, 0);
    { lv_obj_t* l = lv_label_create(btn_save_); lv_label_set_text(l, LV_SYMBOL_DOWNLOAD); lv_obj_center(l); }
    btn_delete_ = lv_btn_create(toolbar_);
    lv_obj_set_size(btn_delete_, 24, 24);
    lv_obj_set_style_radius(btn_delete_, LV_RADIUS_CIRCLE, 0);
    { lv_obj_t* l = lv_label_create(btn_delete_); lv_label_set_text(l, LV_SYMBOL_TRASH); lv_obj_center(l); }
    btn_debug_ = lv_btn_create(toolbar_);
    lv_obj_set_size(btn_debug_, 24, 24);
    lv_obj_set_style_radius(btn_debug_, LV_RADIUS_CIRCLE, 0);
    { lv_obj_t* l = lv_label_create(btn_debug_); lv_label_set_text(l, LV_SYMBOL_WARNING); lv_obj_center(l); }

    lv_obj_add_event_cb(btn_new_, on_new, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(btn_save_, on_save, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(btn_delete_, on_delete, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(btn_debug_, on_debug, LV_EVENT_CLICKED, this);

    status_label_ = lv_label_create(page_edit_);
    lv_obj_add_flag(status_label_, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(status_label_, LV_ALIGN_TOP_RIGHT, -4, 4);
    lv_label_set_text(status_label_, "");
    lv_obj_move_foreground(status_label_);

    editor_ = lv_textarea_create(page_edit_);
    lv_obj_set_size(editor_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(editor_, 1);
    lv_textarea_set_placeholder_text(editor_, "在此编辑UXC JSON...");
    lv_obj_set_style_bg_color(editor_, lv_color_hex(0x252525), 0);
    lv_obj_set_style_bg_opa(editor_, LV_OPA_80, 0);
    lv_obj_set_style_border_width(editor_, 1, 0);
    lv_obj_set_style_border_color(editor_, lv_color_hex(0x444444), 0);
    lv_obj_set_style_text_color(editor_, lv_color_hex(0xEEEEEE), 0);

    refresh_list();
    lv_obj_move_foreground(toolbar_);
    lv_obj_move_foreground(status_label_);
}
