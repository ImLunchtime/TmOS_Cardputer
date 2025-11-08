#include "window_system.h"
#include "input_kb.h"
#include <M5Cardputer.h>

static inline bool obj_is_focusable(lv_obj_t* obj) {
    if (!lv_obj_is_valid(obj)) return false;
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) return false;
    // Whitelist typical interactive controls
    if (lv_obj_has_class(obj, &lv_btn_class)) return true;
    if (lv_obj_has_class(obj, &lv_textarea_class)) return true;
    if (lv_obj_has_class(obj, &lv_dropdown_class)) return true;
    if (lv_obj_has_class(obj, &lv_checkbox_class)) return true;
    if (lv_obj_has_class(obj, &lv_slider_class)) return true;
    if (lv_obj_has_class(obj, &lv_switch_class)) return true;
    if (lv_obj_has_class(obj, &lv_spinbox_class)) return true;
    if (lv_obj_has_class(obj, &lv_roller_class)) return true;
    // Fallback: editable objects can be focused
    if (lv_obj_is_editable(obj)) return true;
    return false;
}

static void add_focusables_recursive(lv_obj_t* root, lv_group_t* group) {
    if (!root || !group) return;
    if (obj_is_focusable(root)) {
        lv_group_add_obj(group, root);
    }
    uint32_t child_cnt = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(root, i);
        add_focusables_recursive(child, group);
    }
}

static lv_obj_t* find_first_focusable(lv_obj_t* root) {
    if (!root) return nullptr;
    if (obj_is_focusable(root)) return root;
    uint32_t child_cnt = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(root, i);
        lv_obj_t* res = find_first_focusable(child);
        if (res) return res;
    }
    return nullptr;
}

WindowSystem::WindowSystem() {}
WindowSystem::~WindowSystem() {
    // Ensure all windows are cleaned up
    while (!stack_.empty()) {
        closeTop();
    }
}

lv_obj_t* WindowSystem::createWindowContainer(const char* title, lv_coord_t* out_w, lv_coord_t* out_h, lv_coord_t* out_x, lv_coord_t* out_y) const {
    // Screen resolution based on lvgl_port setup
    const lv_coord_t scr_w = lv_disp_get_hor_res(NULL);
    const lv_coord_t scr_h = lv_disp_get_ver_res(NULL);

    // Choose a window size smaller than screen
    lv_coord_t w = scr_w - 30; // leave margin
    lv_coord_t h = scr_h - 25;
    if (w < 100) w = scr_w; // fallback
    if (h < 60)  h = scr_h;

    // Position logic: top-left by default; offset from previous by (+30, +20)
    lv_coord_t x = 0;
    lv_coord_t y = 0;
    if (!stack_.empty()) {
        const WindowEntry& prev = stack_.back();
        x = prev.x + 30;
        y = prev.y + 20;
        if (x + w > scr_w) x = scr_w - w;
        if (y + h > scr_h) y = scr_h - h;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
    }

    lv_obj_t* cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, w, h);
    lv_obj_set_pos(cont, x, y);
    lv_obj_set_style_radius(cont, 8, 0);
    lv_obj_set_style_border_width(cont, 2, 0);
    lv_obj_set_style_border_color(cont, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_pad_all(cont, 6, 0);
    lv_obj_set_style_bg_color(cont, lv_color_make(30, 30, 30), 0);

    // Simple title label at top-left
    if (title) {
        lv_obj_t* lbl = lv_label_create(cont);
        lv_label_set_text(lbl, title);
        lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 4, 2);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    }

    // Bring to foreground
    lv_obj_move_foreground(cont);

    if (out_w) *out_w = w;
    if (out_h) *out_h = h;
    if (out_x) *out_x = x;
    if (out_y) *out_y = y;
    return cont;
}

lv_group_t* WindowSystem::buildFocusGroup(lv_obj_t* root) const {
    lv_group_t* group = lv_group_create();
    add_focusables_recursive(root, group);
    return group;
}

void WindowSystem::applyActiveState() {
    // Disable input for all background windows; enable only top-most
    lv_indev_t* indev = kb_get_indev();
    for (size_t i = 0; i < stack_.size(); ++i) {
        auto& win = stack_[i];
        if (!win.root) continue;
        if (i + 1 == stack_.size()) {
            // Active window
            lv_obj_clear_state(win.root, LV_STATE_DISABLED);
            if (indev && win.group) lv_indev_set_group(indev, win.group);
        } else {
            // Background window: disable
            lv_obj_add_state(win.root, LV_STATE_DISABLED);
        }
    }
}

void WindowSystem::openApp(std::unique_ptr<IApp> app) {
    if (!app) return;
    lv_coord_t w, h, x, y;
    lv_obj_t* cont = createWindowContainer(app->title(), &w, &h, &x, &y);
    // Let the app build its UI inside the window container
    app->onOpen(cont);

    WindowEntry entry;
    entry.app = std::move(app);
    entry.root = cont;
    entry.group = buildFocusGroup(cont);
    entry.x = x; entry.y = y; entry.w = w; entry.h = h;
    // Focus the first focusable control in the window
    if (entry.group) {
        lv_obj_t* first = find_first_focusable(cont);
        if (first) lv_group_focus_obj(first);
    }

    stack_.push_back(std::move(entry));

    applyActiveState();
}

void WindowSystem::closeTop() {
    if (stack_.empty()) return;
    auto entry = std::move(stack_.back());
    stack_.pop_back();

    if (entry.app) entry.app->onClose();
    if (entry.group) {
        // Deleting group will auto-detach indev when switching below
        lv_group_del(entry.group);
        entry.group = nullptr;
    }
    if (entry.root) {
        lv_obj_del(entry.root);
        entry.root = nullptr;
    }

    applyActiveState();
}

void WindowSystem::update() {
    // Handle close button for non-launcher
    if (!stack_.empty()) {
        IApp* app = stack_.back().app.get();
        if (app && !app->isLauncher()) {
            if (M5Cardputer.BtnA.wasPressed()) {
                closeTop();
            }
        }
        // Tick only active app
        if (app) app->onTick();
    }
}

IApp* WindowSystem::activeApp() const {
    if (stack_.empty()) return nullptr;
    return stack_.back().app.get();
}