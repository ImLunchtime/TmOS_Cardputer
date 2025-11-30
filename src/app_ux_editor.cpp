#include "app_ux_editor.h"
#include <Arduino.h>

static void on_new(lv_event_t* e) { auto* app = (AppUXEditor*)lv_event_get_user_data(e); if (app) app->create_new(); }
static void on_save(lv_event_t* e) { auto* app = (AppUXEditor*)lv_event_get_user_data(e); if (app) app->save_current(); }
static void on_delete(lv_event_t* e) { auto* app = (AppUXEditor*)lv_event_get_user_data(e); if (app) app->delete_current(); }
static void on_run(lv_event_t* e) { auto* app = (AppUXEditor*)lv_event_get_user_data(e); if (app) app->run_preview(); }
static void on_select(lv_event_t* e) { auto* app = (AppUXEditor*)lv_event_get_user_data(e); if (app) app->load_selected(); }

void AppUXEditor::refresh_list() {
    if (!fm_.initialize()) return;
    const int MAX = 128; FileInfo files[MAX]; int cnt = 0;
    fm_.scanAllFiles(files, cnt, MAX, String(".uxc"));
    String opts = ""; current_path_ = "";
    for (int i = 0; i < cnt; ++i) {
        String name = files[i].name;
        if (opts.length() > 0) opts += "\n";
        opts += name;
        if (current_path_.length() == 0) current_path_ = files[i].path;
    }
    lv_dropdown_set_options(dropdown_, opts.c_str());
}

void AppUXEditor::load_selected() {
    if (!fm_.initialize()) return;
    int sel_idx = lv_dropdown_get_selected(dropdown_);
    const int MAX = 128; FileInfo files[MAX]; int cnt = 0;
    fm_.scanAllFiles(files, cnt, MAX, String(".uxc"));
    String target = "";
    if (sel_idx >= 0 && sel_idx < cnt) target = files[sel_idx].path;
    if (target.length() == 0) target = current_path_;
    if (target.length() == 0) return;
    current_path_ = target;
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

void AppUXEditor::run_preview() {
    ux::clear_children(preview_);
    auto r = ux::build_from_string(String(lv_textarea_get_text(editor_)), preview_);
    if (!r.ok) {
        lv_obj_t* lbl = lv_label_create(preview_);
        String msg = String("Error: ") + r.error;
        lv_label_set_text(lbl, msg.c_str());
    }
}

void AppUXEditor::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(root_, 6, 0);
    toolbar_ = lv_obj_create(root_);
    lv_obj_set_size(toolbar_, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_border_width(toolbar_, 0, 0);
    lv_obj_set_style_bg_opa(toolbar_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(toolbar_, 0, 0);
    lv_obj_set_flex_flow(toolbar_, LV_FLEX_FLOW_ROW);
    btn_new_ = lv_btn_create(toolbar_);
    { lv_obj_t* l = lv_label_create(btn_new_); lv_label_set_text(l, "新建"); lv_obj_center(l); }
    btn_save_ = lv_btn_create(toolbar_);
    { lv_obj_t* l = lv_label_create(btn_save_); lv_label_set_text(l, "保存"); lv_obj_center(l); }
    btn_delete_ = lv_btn_create(toolbar_);
    { lv_obj_t* l = lv_label_create(btn_delete_); lv_label_set_text(l, "删除"); lv_obj_center(l); }
    btn_run_ = lv_btn_create(toolbar_);
    { lv_obj_t* l = lv_label_create(btn_run_); lv_label_set_text(l, "运行"); lv_obj_center(l); }
    dropdown_ = lv_dropdown_create(toolbar_);
    lv_obj_set_width(dropdown_, 120);
    lv_obj_add_event_cb(btn_new_, on_new, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(btn_save_, on_save, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(btn_delete_, on_delete, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(btn_run_, on_run, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(dropdown_, on_select, LV_EVENT_VALUE_CHANGED, this);
    lv_obj_t* split = lv_obj_create(root_);
    lv_obj_set_size(split, lv_pct(100), lv_pct(100));
    lv_obj_set_style_border_width(split, 0, 0);
    lv_obj_set_style_bg_opa(split, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(split, 0, 0);
    lv_obj_set_flex_flow(split, LV_FLEX_FLOW_ROW);
    editor_ = lv_textarea_create(split);
    lv_obj_set_size(editor_, 120, lv_pct(100));
    lv_obj_set_flex_grow(editor_, 1);
    lv_textarea_set_placeholder_text(editor_, "在此编辑UXC JSON...");
    preview_ = lv_obj_create(split);
    lv_obj_set_size(preview_, 120, lv_pct(100));
    lv_obj_set_flex_grow(preview_, 1);
    lv_obj_set_style_border_width(preview_, 0, 0);
    lv_obj_set_style_radius(preview_, 0, 0);
    lv_obj_set_style_bg_opa(preview_, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(preview_, LV_FLEX_FLOW_COLUMN);
    refresh_list();
}
