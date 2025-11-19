/*
 * @Author: ImLunchtime knoxmedia@yeah.net
 * @Date: 2025-11-08 18:13:39
 * @LastEditors: ImLunchtime knoxmedia@yeah.net
 * @LastEditTime: 2025-11-19 14:13:34
 * @FilePath: \CardputerOS2_LVGL\src\app_launcher.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "app_launcher.h"
#include "app_music.h"
#include "app_theme_center.h"
#include "app_settings.h"
#include "app_test_iapp.h"
#include "theme.h"
LV_IMG_DECLARE(icon_music_sd);
LV_IMG_DECLARE(icon_theme);
LV_IMG_DECLARE(icon_test);

static lv_obj_t* find_img_child(lv_obj_t* parent) {
    if (!parent) return nullptr;
    uint32_t n = lv_obj_get_child_cnt(parent);
    for (uint32_t i = 0; i < n; ++i) {
        lv_obj_t* c = lv_obj_get_child(parent, i);
        if (lv_obj_has_class(c, &lv_img_class)) return c;
    }
    return nullptr;
}

static void on_music_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchMusic();
}

static void on_theme_center_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchThemeCenter();
}

static void on_settings_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchSettings();
}

static void on_test_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchTest();
}

void AppLauncher::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_set_style_pad_row(root_, 0, 0);
    lv_obj_set_scroll_dir(root_, LV_DIR_NONE);

    list_ = lv_list_create(root_);
    lv_obj_set_size(list_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(list_, 1);
    lv_obj_set_pos(list_, 0, 0);
    lv_obj_set_style_pad_all(list_, 0, 0);
    lv_obj_set_style_radius(list_, 0, 0);
    lv_obj_set_style_border_width(list_, 0, 0);
    lv_obj_set_style_border_opa(list_, LV_OPA_TRANSP, 0);
    ui_theme::apply_list_menu(list_);

    lv_obj_t* item_music = lv_list_add_btn(list_, &icon_music_sd, "Music");
    ui_theme::apply_list_menu_item(item_music);
    lv_obj_set_style_min_height(item_music, 20, 0);
    lv_obj_add_event_cb(item_music, on_music_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = find_img_child(item_music);
        //if (img) lv_img_set_zoom(img, 128);
    }

    lv_obj_t* item_theme = lv_list_add_btn(list_, &icon_theme, "Theme Center");
    ui_theme::apply_list_menu_item(item_theme);
    lv_obj_set_style_min_height(item_theme, 20, 0);
    lv_obj_add_event_cb(item_theme, on_theme_center_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = find_img_child(item_theme);
        //if (img) lv_img_set_zoom(img, 128);
    }

    lv_obj_t* item_test = lv_list_add_btn(list_, &icon_test, "Test");
    ui_theme::apply_list_menu_item(item_test);
    lv_obj_set_style_min_height(item_test, 20, 0);
    lv_obj_add_event_cb(item_test, on_test_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = find_img_child(item_test);
        //if (img) lv_img_set_zoom(img, 128);
    }

    lv_obj_t* item_settings = lv_list_add_btn(list_, LV_SYMBOL_SETTINGS, "Settings");
    ui_theme::apply_list_menu_item(item_settings);
    lv_obj_set_style_min_height(item_settings, 20, 0);
    lv_obj_add_event_cb(item_settings, on_settings_item_event, LV_EVENT_CLICKED, this);

}

void AppLauncher::launchMusic() { wm_.openApp(std::unique_ptr<IApp>(new AppMusic())); }
void AppLauncher::launchThemeCenter() { wm_.openApp(std::unique_ptr<IApp>(new AppThemeCenter())); }
void AppLauncher::launchSettings() { wm_.openApp(std::unique_ptr<IApp>(new AppSettings())); }
void AppLauncher::launchTest() { wm_.openApp(std::unique_ptr<IApp>(new AppTest())); }