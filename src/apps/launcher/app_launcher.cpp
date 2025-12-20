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
#include "apps/music_downloader/app_music_downloader.h"
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
#include "apps/wifi/app_wifi.h"
#include "apps/remote/app_remote.h"
#include "ui/theme.h"
#include "drivers/input_kb.h"
LV_IMG_DECLARE(icon_music_sd);
LV_IMG_DECLARE(icon_music_cloud);
LV_IMG_DECLARE(icon_test);
LV_IMG_DECLARE(icon_devices);
LV_IMG_DECLARE(icon_bluetooth);
LV_IMG_DECLARE(icon_pictures);
LV_IMG_DECLARE(icon_circuitsim2);
LV_IMG_DECLARE(icon_uxedit2);
LV_IMG_DECLARE(icon_uxexec);
LV_IMG_DECLARE(icon_calculator);
LV_IMG_DECLARE(icon_radio);
LV_IMG_DECLARE(icon_station_reporter);
LV_IMG_DECLARE(icon_brightness);
LV_IMG_DECLARE(icon_wifi);
LV_IMG_DECLARE(icon_remote);

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

static void on_music_dl_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchMusicDownloader();
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

static void on_wifi_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchWiFi();
}

static void on_remote_item_event(lv_event_t* e) {
    auto* launcher = static_cast<AppLauncher*>(lv_event_get_user_data(e));
    if (!launcher) return;
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) launcher->launchRemote();
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

    kb_register_app_keys(this, {
        { ';', [this](){ return true; }, [this](){ moveFocus(0, -1); } },
        { ',', [this](){ return true; }, [this](){ moveFocus(-1, 0); } },
        { '.', [this](){ return true; }, [this](){ moveFocus(0, 1); } },
        { '/', [this](){ return true; }, [this](){ moveFocus(1, 0); } },
    });
    struct Spec { const lv_img_dsc_t* icon; lv_event_cb_t cb; };
    const Spec specs[] = {
        { &icon_brightness, on_brightness_item_event },
        { &icon_wifi, on_wifi_item_event },
        { &icon_music_sd, on_music_item_event },
        { &icon_music_cloud, on_music_dl_item_event },
        { &icon_pictures, on_pictures_item_event },
        { &icon_remote, on_remote_item_event },
        { &icon_circuitsim2, on_circuitsim_item_event },
        { &icon_station_reporter, on_station_reporter_item_event },
        { &icon_uxedit2, on_ux_editor_item_event },
        { &icon_uxexec, on_ux_executor_item_event },
        { &icon_devices, on_devices_item_event },
        { &icon_bluetooth, on_bluetooth_item_event },
        { &icon_radio, on_radiosim_item_event },
        { &icon_test, on_test_item_event },
    };
    int index = 0;
    for (const auto& s : specs) {
        lv_obj_t* btn = lv_btn_create(grid_);
        lv_obj_set_size(btn, 36, 36);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_style_radius(btn, 0, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_set_style_outline_width(btn, 0, 0);
        int col = index % cols_;
        int row = index / cols_;
        lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_CENTER, col, 1, LV_GRID_ALIGN_CENTER, row, 1);
        items_.push_back({btn, col, row});
        lv_obj_add_event_cb(btn, s.cb, LV_EVENT_CLICKED, this);
        lv_obj_t* img = lv_img_create(btn);
        lv_img_set_src(img, s.icon);
        lv_obj_center(img);
        index++;
    }
}

void AppLauncher::launchMusic() { wm_.openApp(std::unique_ptr<IApp>(new AppMusic())); }

void AppLauncher::launchMusicDownloader() { wm_.openApp(std::unique_ptr<IApp>(new AppMusicDownloader())); }
 
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
void AppLauncher::launchWiFi() { wm_.openApp(std::unique_ptr<IApp>(new AppWiFi())); }
void AppLauncher::launchRemote() { wm_.openApp(std::unique_ptr<IApp>(new AppRemote())); }
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
