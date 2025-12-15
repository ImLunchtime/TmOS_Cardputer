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
    static void on_back(lv_event_t* e);
    static void on_result_item_clicked(lv_event_t* e);
    static void on_cancel_download(lv_event_t* e);
    static void on_download_timer(lv_timer_t* t);
    static void downloadTaskThunk(void* parameter);

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* label_status_ = nullptr;
    lv_obj_t* page_search_ = nullptr;
    lv_obj_t* page_results_ = nullptr;
    lv_obj_t* page_download_ = nullptr;
    lv_obj_t* ta_keyword_ = nullptr;
    lv_obj_t* btn_search_ = nullptr;
    lv_obj_t* list_ = nullptr;
    lv_obj_t* btn_back_ = nullptr;
    lv_obj_t* bar_ = nullptr;
    lv_obj_t* label_progress_ = nullptr;
    lv_obj_t* btn_cancel_ = nullptr;
    bool in_results_ = false;
    bool downloading_ = false;
    lv_timer_t* timer_download_ = nullptr;
    TaskHandle_t downloadTaskHandle_ = nullptr;
    volatile long dl_total_ = -1;
    volatile long dl_written_ = 0;
    volatile bool dl_done_ = false;
    volatile bool dl_ok_ = false;
    volatile bool dl_cancel_ = false;
    String dl_error_;
    String dl_url_;
    String dl_save_path_;

    struct SongItem {
        String title;
        String artist;
        uint32_t id = 0;
        bool vip = false;
    };

    std::vector<SongItem> results_;

    void perform_search();
    void update_results_list();
    void switch_to_search_page();
    void switch_to_results_page();
    void switch_to_download_page();
    void start_download_by_index(int idx);
    String build_filename_for_index(int idx);
    static String sanitize_token(const String& s);
    bool download_to_file(const char* url, const char* save_path);
    void downloadTaskLoop();
    void rebuildFocusGroup();
    void add_focusables_recursive(lv_obj_t* node, lv_group_t* group);
};
