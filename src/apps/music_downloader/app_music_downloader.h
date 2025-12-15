#pragma once
#include "window_system.h"
#include "wifi_manager.h"
#include <lvgl.h>
#include <vector>

class AppMusicDownloader : public IApp {
public:
    AppMusicDownloader();
    ~AppMusicDownloader() override;

    const char* title() const override { return "Music Downloader"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Light; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override;
    void onClose() override;

    static void on_search(lv_event_t* e);

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* label_status_ = nullptr;
    lv_obj_t* ta_keyword_ = nullptr;
    lv_obj_t* btn_search_ = nullptr;
    lv_obj_t* list_ = nullptr;

    struct SongItem {
        String title;
        String artist;
    };

    std::vector<SongItem> results_;

    void perform_search();
    void update_results_list();
    void rebuildFocusGroup();
    void add_focusables_recursive(lv_obj_t* node, lv_group_t* group);
};

