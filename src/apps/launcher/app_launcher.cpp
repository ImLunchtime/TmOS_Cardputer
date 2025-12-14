/*
 * @Author: ImLunchtime knoxmedia@yeah.net
 * @Date: 2025-11-08 18:13:39
 * @LastEditors: ImLunchtime knoxmedia@yeah.net
 * @LastEditTime: 2025-12-07 10:44:09
 * @FilePath: \CardputerOS2_LVGL\src\app_launcher.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "apps/launcher/app_launcher.h"
#include "apps/music/app_music.h"
#include "apps/pictures/app_pictures.h"
#include "apps/devices/app_devices.h"
#include "apps/bluetooth/app_bluetooth.h"
#include "apps/test/app_test.h"
#include "apps/circuitsim/app_circuitsim.h"
#include "apps/ux_editor/app_ux_editor.h"
#include "apps/ux_executor/app_ux_executor.h"
#include "apps/calculator/app_calculator.h"
#include "apps/radiosim/app_radiosim.h"
#include "apps/station_reporter/app_station_reporter.h"
#include "apps/brightness/app_brightness.h"
#include "theme.h"
#include "input_kb.h"
LV_IMG_DECLARE(icon_music_sd);
LV_IMG_DECLARE(icon_test);
LV_IMG_DECLARE(icon_devices);
LV_IMG_DECLARE(icon_bluetooth);
LV_IMG_DECLARE(icon_pictures);
LV_IMG_DECLARE(icon_circuitsim);
LV_IMG_DECLARE(icon_uxedit2);
LV_IMG_DECLARE(icon_uxexec);
LV_IMG_DECLARE(icon_calculator);
LV_IMG_DECLARE(icon_radio);
LV_IMG_DECLARE(icon_station_reporter);
LV_IMG_DECLARE(icon_brightness);

static void on_brightness_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchBrightness();
}

static void on_music_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchMusic();
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

static void on_calculator_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchCalculator();
}

static void on_radiosim_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchRadioSim();
}


static void on_station_reporter_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchStationReporter();
}

void AppLauncher::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    // Narrow window to fit 4x36px items
    lv_obj_set_width(root_, 37 * 4);

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
    static lv_coord_t col_dsc[] = {36, 36, 36, 36, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {36, 36, 36, 36, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(grid_, col_dsc, row_dsc);

    lv_obj_t* btn_music = lv_btn_create(grid_);
    lv_obj_set_size(btn_music, 36, 36);
    lv_obj_set_style_bg_opa(btn_music, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_music, 0, 0);
    lv_obj_set_style_radius(btn_music, 0, 0);
    lv_obj_set_style_shadow_width(btn_music, 0, 0);
    lv_obj_set_style_outline_width(btn_music, 0, 0);
    lv_obj_set_grid_cell(btn_music, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    items_.push_back({btn_music, 0, 0});
    lv_obj_add_event_cb(btn_music, on_music_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_music);
        lv_img_set_src(img, &icon_music_sd);
        lv_obj_center(img);
    }


    lv_obj_t* btn_test = lv_btn_create(grid_);
    lv_obj_set_size(btn_test, 36, 36);
    lv_obj_set_style_bg_opa(btn_test, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_test, 0, 0);
    lv_obj_set_style_radius(btn_test, 0, 0);
    lv_obj_set_style_shadow_width(btn_test, 0, 0);
    lv_obj_set_style_outline_width(btn_test, 0, 0);
    lv_obj_set_grid_cell(btn_test, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    items_.push_back({btn_test, 1, 0});
    lv_obj_add_event_cb(btn_test, on_test_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_test);
        lv_img_set_src(img, &icon_test);
        lv_obj_center(img);
    }


    lv_obj_t* btn_devices = lv_btn_create(grid_);
    lv_obj_set_size(btn_devices, 36, 36);
    lv_obj_set_style_bg_opa(btn_devices, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_devices, 0, 0);
    lv_obj_set_style_radius(btn_devices, 0, 0);
    lv_obj_set_style_shadow_width(btn_devices, 0, 0);
    lv_obj_set_style_outline_width(btn_devices, 0, 0);
    lv_obj_set_grid_cell(btn_devices, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    items_.push_back({btn_devices, 2, 0});
    lv_obj_add_event_cb(btn_devices, on_devices_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_devices);
        lv_img_set_src(img, &icon_devices);
        lv_obj_center(img);
    }

    lv_obj_t* btn_bluetooth = lv_btn_create(grid_);
    lv_obj_set_size(btn_bluetooth, 36, 36);
    lv_obj_set_style_bg_opa(btn_bluetooth, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_bluetooth, 0, 0);
    lv_obj_set_style_radius(btn_bluetooth, 0, 0);
    lv_obj_set_style_shadow_width(btn_bluetooth, 0, 0);
    lv_obj_set_style_outline_width(btn_bluetooth, 0, 0);
    lv_obj_set_grid_cell(btn_bluetooth, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    items_.push_back({btn_bluetooth, 3, 0});
    lv_obj_add_event_cb(btn_bluetooth, on_bluetooth_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_bluetooth);
        lv_img_set_src(img, &icon_bluetooth);
        lv_obj_center(img);
    }

    lv_obj_t* btn_pictures = lv_btn_create(grid_);
    lv_obj_set_size(btn_pictures, 36, 36);
    lv_obj_set_style_bg_opa(btn_pictures, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_pictures, 0, 0);
    lv_obj_set_style_radius(btn_pictures, 0, 0);
    lv_obj_set_style_shadow_width(btn_pictures, 0, 0);
    lv_obj_set_style_outline_width(btn_pictures, 0, 0);
    lv_obj_set_grid_cell(btn_pictures, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    items_.push_back({btn_pictures, 0, 1});
    lv_obj_add_event_cb(btn_pictures, on_pictures_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_pictures);
        lv_img_set_src(img, &icon_pictures);
        lv_obj_center(img);
    }

    lv_obj_t* btn_circuitsim = lv_btn_create(grid_);
    lv_obj_set_size(btn_circuitsim, 36, 36);
    lv_obj_set_style_bg_opa(btn_circuitsim, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_circuitsim, 0, 0);
    lv_obj_set_style_radius(btn_circuitsim, 0, 0);
    lv_obj_set_style_shadow_width(btn_circuitsim, 0, 0);
    lv_obj_set_style_outline_width(btn_circuitsim, 0, 0);
    lv_obj_set_grid_cell(btn_circuitsim, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    items_.push_back({btn_circuitsim, 1, 1});
    lv_obj_add_event_cb(btn_circuitsim, on_circuitsim_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_circuitsim);
        lv_img_set_src(img, &icon_circuitsim);
        lv_obj_center(img);
    }

    lv_obj_t* btn_ux_editor = lv_btn_create(grid_);
    lv_obj_set_size(btn_ux_editor, 36, 36);
    lv_obj_set_style_bg_opa(btn_ux_editor, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_ux_editor, 0, 0);
    lv_obj_set_style_radius(btn_ux_editor, 0, 0);
    lv_obj_set_style_shadow_width(btn_ux_editor, 0, 0);
    lv_obj_set_style_outline_width(btn_ux_editor, 0, 0);
    lv_obj_set_grid_cell(btn_ux_editor, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    items_.push_back({btn_ux_editor, 2, 1});
    lv_obj_add_event_cb(btn_ux_editor, on_ux_editor_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_ux_editor);
        lv_img_set_src(img, &icon_uxedit2);
        lv_obj_center(img);
    }

    lv_obj_t* btn_ux_executor = lv_btn_create(grid_);
    lv_obj_set_size(btn_ux_executor, 36, 36);
    lv_obj_set_style_bg_opa(btn_ux_executor, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_ux_executor, 0, 0);
    lv_obj_set_style_radius(btn_ux_executor, 0, 0);
    lv_obj_set_style_shadow_width(btn_ux_executor, 0, 0);
    lv_obj_set_style_outline_width(btn_ux_executor, 0, 0);
    lv_obj_set_grid_cell(btn_ux_executor, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    items_.push_back({btn_ux_executor, 3, 1});
    lv_obj_add_event_cb(btn_ux_executor, on_ux_executor_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_ux_executor);
        lv_img_set_src(img, &icon_uxexec);
        lv_obj_center(img);
    }

    lv_obj_t* btn_calculator = lv_btn_create(grid_);
    lv_obj_set_size(btn_calculator, 36, 36);
    lv_obj_set_style_bg_opa(btn_calculator, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_calculator, 0, 0);
    lv_obj_set_style_radius(btn_calculator, 0, 0);
    lv_obj_set_style_shadow_width(btn_calculator, 0, 0);
    lv_obj_set_style_outline_width(btn_calculator, 0, 0);
    lv_obj_set_grid_cell(btn_calculator, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    items_.push_back({btn_calculator, 0, 2});

    kb_register_app_keys(this, {
        { ';', [this](){ return true; }, [this](){ moveFocus(0, -1); } },
        { ',', [this](){ return true; }, [this](){ moveFocus(-1, 0); } },
        { '.', [this](){ return true; }, [this](){ moveFocus(0, 1); } },
        { '/', [this](){ return true; }, [this](){ moveFocus(1, 0); } },
    });
    lv_obj_add_event_cb(btn_calculator, on_calculator_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_calculator);
        lv_img_set_src(img, &icon_calculator);
        lv_obj_center(img);
    }

    lv_obj_t* btn_radiosim = lv_btn_create(grid_);
    lv_obj_set_size(btn_radiosim, 36, 36);
    lv_obj_set_style_bg_opa(btn_radiosim, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_radiosim, 0, 0);
    lv_obj_set_style_radius(btn_radiosim, 0, 0);
    lv_obj_set_style_shadow_width(btn_radiosim, 0, 0);
    lv_obj_set_style_outline_width(btn_radiosim, 0, 0);
    lv_obj_set_grid_cell(btn_radiosim, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    items_.push_back({btn_radiosim, 1, 2});
    lv_obj_add_event_cb(btn_radiosim, on_radiosim_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_radiosim);
        lv_img_set_src(img, &icon_radio);
        lv_obj_center(img);
    }


    lv_obj_t* btn_station = lv_btn_create(grid_);
    lv_obj_set_size(btn_station, 36, 36);
    lv_obj_set_style_bg_opa(btn_station, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_station, 0, 0);
    lv_obj_set_style_radius(btn_station, 0, 0);
    lv_obj_set_style_shadow_width(btn_station, 0, 0);
    lv_obj_set_style_outline_width(btn_station, 0, 0);
    lv_obj_set_grid_cell(btn_station, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    items_.push_back({btn_station, 2, 2});
    lv_obj_add_event_cb(btn_station, on_station_reporter_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_station);
        lv_img_set_src(img, &icon_station_reporter);
        lv_obj_center(img);
    }

    lv_obj_t* btn_brightness = lv_btn_create(grid_);
    lv_obj_set_size(btn_brightness, 36, 36);
    lv_obj_set_style_bg_opa(btn_brightness, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_brightness, 0, 0);
    lv_obj_set_style_radius(btn_brightness, 0, 0);
    lv_obj_set_style_shadow_width(btn_brightness, 0, 0);
    lv_obj_set_style_outline_width(btn_brightness, 0, 0);
    lv_obj_set_grid_cell(btn_brightness, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    items_.push_back({btn_brightness, 3, 2});
    lv_obj_add_event_cb(btn_brightness, on_brightness_item_event, LV_EVENT_CLICKED, this);
    {
        lv_obj_t* img = lv_img_create(btn_brightness);
        lv_img_set_src(img, &icon_brightness);
        lv_obj_center(img);
    }
}

void AppLauncher::launchMusic() { wm_.openApp(std::unique_ptr<IApp>(new AppMusic())); }
 
void AppLauncher::launchTest() { wm_.openApp(std::unique_ptr<IApp>(new AppTest())); }
void AppLauncher::launchDevices() { wm_.openApp(std::unique_ptr<IApp>(new AppDevices())); }
void AppLauncher::launchBluetooth() { wm_.openApp(std::unique_ptr<IApp>(new AppBluetooth())); }
void AppLauncher::launchPictures() { wm_.openApp(std::unique_ptr<IApp>(new AppPictures())); }
void AppLauncher::launchCircuitSim() { wm_.openApp(std::unique_ptr<IApp>(new AppCircuitSim())); }
void AppLauncher::launchUXEditor() { wm_.openApp(std::unique_ptr<IApp>(new AppUXEditor())); }
void AppLauncher::launchUXExecutor() { wm_.openApp(std::unique_ptr<IApp>(new AppUXExecutor(wm_))); }
void AppLauncher::launchCalculator() { wm_.openApp(std::unique_ptr<IApp>(new AppCalculator())); }
void AppLauncher::launchRadioSim() { wm_.openApp(std::unique_ptr<IApp>(new AppRadioSim())); }
void AppLauncher::launchStationReporter() { wm_.openApp(std::unique_ptr<IApp>(new AppStationReporter())); }
void AppLauncher::launchBrightness() { wm_.openApp(std::unique_ptr<IApp>(new AppBrightness())); }
void AppLauncher::moveFocus(int dx, int dy) {
    int curc = -1, curr = -1;
    for (auto &it : items_) { if (lv_obj_has_state(it.obj, LV_STATE_FOCUSED)) { curc = it.col; curr = it.row; break; } }
    if (curc < 0 || curr < 0) return;
    int tc = curc + dx;
    int tr = curr + dy;
    if (tc < 0 || tc >= cols_ || tr < 0 || tr >= rows_) return;
    for (auto &it : items_) { if (it.col == tc && it.row == tr) { lv_group_focus_obj(it.obj); return; } }
}
void AppLauncher::onClose() {
    kb_clear_app_keys(this);
}
