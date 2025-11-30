/*
 * @Author: ImLunchtime knoxmedia@yeah.net
 * @Date: 2025-11-08 18:13:39
 * @LastEditors: ImLunchtime knoxmedia@yeah.net
 * @LastEditTime: 2025-11-23 17:14:27
 * @FilePath: \CardputerOS2_LVGL\src\app_launcher.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "app_launcher.h"
#include "app_music.h"
#include "app_theme_center.h"
#include "app_pictures.h"
#include "app_devices.h"
#include "app_bluetooth.h"
#include "app_settings.h"
#include "app_test.h"
#include "app_circuitsim.h"
#include "app_ux_editor.h"
#include "app_ux_executor.h"
#include "theme.h"
LV_IMG_DECLARE(icon_music_sd);
LV_IMG_DECLARE(icon_theme);
LV_IMG_DECLARE(icon_test);
LV_IMG_DECLARE(icon_devices);
LV_IMG_DECLARE(icon_bluetooth);
LV_IMG_DECLARE(icon_pictures);
LV_IMG_DECLARE(icon_circuitsim);
LV_IMG_DECLARE(icon_uxedit2);
LV_IMG_DECLARE(icon_uxexec);

 

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

static void on_devices_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchDevices();
}

static void on_bluetooth_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchBluetooth();
}

static void on_pictures_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchPictures();
}

static void on_circuitsim_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchCircuitSim();
}

static void on_ux_editor_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchUXEditor();
}

static void on_ux_executor_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchUXExecutor();
}

void AppLauncher::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_set_style_pad_row(root_, 0, 0);
    lv_obj_set_scroll_dir(root_, LV_DIR_NONE);

    grid_ = lv_obj_create(root_);
    lv_obj_set_size(grid_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(grid_, 1);
    lv_obj_set_style_pad_all(grid_, 0, 0);
    lv_obj_set_style_pad_row(grid_, 0, 0);
    lv_obj_set_style_pad_column(grid_, 0, 0);
    lv_obj_set_style_border_width(grid_, 0, 0);
    lv_obj_set_style_radius(grid_, 0, 0);
    lv_obj_set_style_bg_opa(grid_, LV_OPA_TRANSP, 0);
    static lv_coord_t col_dsc[] = {48, 48, 48, 48, 48, 48, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {48, 48, 48, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(grid_, col_dsc, row_dsc);

    lv_obj_t* btn_music = lv_btn_create(grid_);
    lv_obj_set_size(btn_music, 48, 48);
    lv_obj_set_style_bg_opa(btn_music, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_music, 0, 0);
    lv_obj_set_style_radius(btn_music, 0, 0);
    lv_obj_set_style_shadow_width(btn_music, 0, 0);
    lv_obj_set_style_outline_width(btn_music, 0, 0);
    lv_obj_set_grid_cell(btn_music, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(btn_music, on_music_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_music);
        lv_img_set_src(img, &icon_music_sd);
        lv_obj_center(img);
    }

    lv_obj_t* btn_theme = lv_btn_create(grid_);
    lv_obj_set_size(btn_theme, 48, 48);
    lv_obj_set_style_bg_opa(btn_theme, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_theme, 0, 0);
    lv_obj_set_style_radius(btn_theme, 0, 0);
    lv_obj_set_style_shadow_width(btn_theme, 0, 0);
    lv_obj_set_style_outline_width(btn_theme, 0, 0);
    lv_obj_set_grid_cell(btn_theme, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(btn_theme, on_theme_center_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_theme);
        lv_img_set_src(img, &icon_theme);
        lv_obj_center(img);
    }

    lv_obj_t* btn_test = lv_btn_create(grid_);
    lv_obj_set_size(btn_test, 48, 48);
    lv_obj_set_style_bg_opa(btn_test, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_test, 0, 0);
    lv_obj_set_style_radius(btn_test, 0, 0);
    lv_obj_set_style_shadow_width(btn_test, 0, 0);
    lv_obj_set_style_outline_width(btn_test, 0, 0);
    lv_obj_set_grid_cell(btn_test, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(btn_test, on_test_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_test);
        lv_img_set_src(img, &icon_test);
        lv_obj_center(img);
    }

    lv_obj_t* btn_settings = lv_btn_create(grid_);
    lv_obj_set_size(btn_settings, 48, 48);
    lv_obj_set_style_bg_opa(btn_settings, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_settings, 0, 0);
    lv_obj_set_style_radius(btn_settings, 0, 0);
    lv_obj_set_style_shadow_width(btn_settings, 0, 0);
    lv_obj_set_style_outline_width(btn_settings, 0, 0);
    lv_obj_set_grid_cell(btn_settings, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(btn_settings, on_settings_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* label = lv_label_create(btn_settings);
        lv_label_set_text(label, LV_SYMBOL_SETTINGS);
        lv_obj_center(label);
    }

    lv_obj_t* btn_devices = lv_btn_create(grid_);
    lv_obj_set_size(btn_devices, 48, 48);
    lv_obj_set_style_bg_opa(btn_devices, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_devices, 0, 0);
    lv_obj_set_style_radius(btn_devices, 0, 0);
    lv_obj_set_style_shadow_width(btn_devices, 0, 0);
    lv_obj_set_style_outline_width(btn_devices, 0, 0);
    lv_obj_set_grid_cell(btn_devices, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_add_event_cb(btn_devices, on_devices_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_devices);
        lv_img_set_src(img, &icon_devices);
        lv_obj_center(img);
    }

    lv_obj_t* btn_bluetooth = lv_btn_create(grid_);
    lv_obj_set_size(btn_bluetooth, 48, 48);
    lv_obj_set_style_bg_opa(btn_bluetooth, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_bluetooth, 0, 0);
    lv_obj_set_style_radius(btn_bluetooth, 0, 0);
    lv_obj_set_style_shadow_width(btn_bluetooth, 0, 0);
    lv_obj_set_style_outline_width(btn_bluetooth, 0, 0);
    lv_obj_set_grid_cell(btn_bluetooth, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_add_event_cb(btn_bluetooth, on_bluetooth_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_bluetooth);
        lv_img_set_src(img, &icon_bluetooth);
        lv_obj_center(img);
    }

    lv_obj_t* btn_pictures = lv_btn_create(grid_);
    lv_obj_set_size(btn_pictures, 48, 48);
    lv_obj_set_style_bg_opa(btn_pictures, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_pictures, 0, 0);
    lv_obj_set_style_radius(btn_pictures, 0, 0);
    lv_obj_set_style_shadow_width(btn_pictures, 0, 0);
    lv_obj_set_style_outline_width(btn_pictures, 0, 0);
    lv_obj_set_grid_cell(btn_pictures, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_add_event_cb(btn_pictures, on_pictures_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_pictures);
        lv_img_set_src(img, &icon_pictures);
        lv_obj_center(img);
    }

    lv_obj_t* btn_circuitsim = lv_btn_create(grid_);
    lv_obj_set_size(btn_circuitsim, 48, 48);
    lv_obj_set_style_bg_opa(btn_circuitsim, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_circuitsim, 0, 0);
    lv_obj_set_style_radius(btn_circuitsim, 0, 0);
    lv_obj_set_style_shadow_width(btn_circuitsim, 0, 0);
    lv_obj_set_style_outline_width(btn_circuitsim, 0, 0);
    lv_obj_set_grid_cell(btn_circuitsim, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_add_event_cb(btn_circuitsim, on_circuitsim_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_circuitsim);
        lv_img_set_src(img, &icon_circuitsim);
        lv_obj_center(img);
    }

    lv_obj_t* btn_ux_editor = lv_btn_create(grid_);
    lv_obj_set_size(btn_ux_editor, 48, 48);
    lv_obj_set_style_bg_opa(btn_ux_editor, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_ux_editor, 0, 0);
    lv_obj_set_style_radius(btn_ux_editor, 0, 0);
    lv_obj_set_style_shadow_width(btn_ux_editor, 0, 0);
    lv_obj_set_style_outline_width(btn_ux_editor, 0, 0);
    lv_obj_set_grid_cell(btn_ux_editor, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_add_event_cb(btn_ux_editor, on_ux_editor_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_ux_editor);
        lv_img_set_src(img, &icon_uxedit2);
        lv_obj_center(img);
    }

    lv_obj_t* btn_ux_executor = lv_btn_create(grid_);
    lv_obj_set_size(btn_ux_executor, 48, 48);
    lv_obj_set_style_bg_opa(btn_ux_executor, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_ux_executor, 0, 0);
    lv_obj_set_style_radius(btn_ux_executor, 0, 0);
    lv_obj_set_style_shadow_width(btn_ux_executor, 0, 0);
    lv_obj_set_style_outline_width(btn_ux_executor, 0, 0);
    lv_obj_set_grid_cell(btn_ux_executor, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_add_event_cb(btn_ux_executor, on_ux_executor_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_ux_executor);
        lv_img_set_src(img, &icon_uxexec);
        lv_obj_center(img);
    }
}

void AppLauncher::launchMusic() { wm_.openApp(std::unique_ptr<IApp>(new AppMusic())); }
void AppLauncher::launchThemeCenter() { wm_.openApp(std::unique_ptr<IApp>(new AppThemeCenter())); }
void AppLauncher::launchSettings() { wm_.openApp(std::unique_ptr<IApp>(new AppSettings())); }
void AppLauncher::launchTest() { wm_.openApp(std::unique_ptr<IApp>(new AppTest())); }
void AppLauncher::launchDevices() { wm_.openApp(std::unique_ptr<IApp>(new AppDevices())); }
void AppLauncher::launchBluetooth() { wm_.openApp(std::unique_ptr<IApp>(new AppBluetooth())); }
void AppLauncher::launchPictures() { wm_.openApp(std::unique_ptr<IApp>(new AppPictures())); }
void AppLauncher::launchCircuitSim() { wm_.openApp(std::unique_ptr<IApp>(new AppCircuitSim())); }
void AppLauncher::launchUXEditor() { wm_.openApp(std::unique_ptr<IApp>(new AppUXEditor())); }
void AppLauncher::launchUXExecutor() { wm_.openApp(std::unique_ptr<IApp>(new AppUXExecutor(wm_))); }
