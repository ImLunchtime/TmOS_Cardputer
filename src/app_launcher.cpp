#include "app_launcher.h"
#include "app_music.h"
#include "theme.h"

static void on_music_btn_event(lv_event_t* e) {
    // Retrieve launcher instance from user_data
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        // Open the Music app on top
        launcher->onClose(); // no-op but kept for symmetry
        // The launcher stays open in background; open new app window
        launcher->launchMusic();
    }
}

void AppLauncher::onOpen(lv_obj_t* window_root) {
    root_ = window_root;

    // Layout: vertical column
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(root_, 8, 0);

    // Music app button
    btn_music_ = lv_btn_create(root_);
    lv_obj_t* lbm = lv_label_create(btn_music_);
    lv_label_set_text(lbm, "Music");
    lv_obj_center(lbm);
    // Apply themed button style (pill + small)
    ui_theme::apply_button(btn_music_);
    lv_obj_add_event_cb(btn_music_, on_music_btn_event, LV_EVENT_CLICKED, this);
}

void AppLauncher::launchMusic() {
    wm_.openApp(std::unique_ptr<IApp>(new AppMusic()));
}