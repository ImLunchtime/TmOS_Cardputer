#include "app_music.h"
#include "theme.h"
#include <algorithm>
#include "input_kb.h"
#include <SD.h>
#include <cstring>

static bool ends_with_ci(const std::string& s, const char* suffix) {
    size_t n = s.size();
    size_t m = strlen(suffix);
    if (m > n) return false;
    for (size_t i = 0; i < m; ++i) {
        char a = s[n - m + i];
        char b = suffix[i];
        if (a >= 'A' && a <= 'Z') a = a - 'A' + 'a';
        if (b >= 'A' && b <= 'Z') b = b - 'A' + 'a';
        if (a != b) return false;
    }
    return true;
}

std::string AppMusic::basename(const std::string& path) {
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

std::string AppMusic::stripExtension(const std::string& s) {
    size_t p = s.find_last_of('.');
    if (p == std::string::npos) return s;
    return s.substr(0, p);
}

bool AppMusic::parseNameParts(const std::string& base, std::string& artist, std::string& album, std::string& title) {
    std::string stem = stripExtension(base);
    size_t p1 = stem.find('-');
    if (p1 == std::string::npos) return false;
    size_t p2 = stem.find('-', p1 + 1);
    if (p2 == std::string::npos) return false;
    if (stem.find('-', p2 + 1) != std::string::npos) return false;
    artist = stem.substr(0, p1);
    album = stem.substr(p1 + 1, p2 - p1 - 1);
    title = stem.substr(p2 + 1);
    if (artist.empty() || album.empty() || title.empty()) return false;
    return true;
}

std::string AppMusic::extractTitle(const std::string& base) {
    std::string a, b, t;
    if (parseNameParts(base, a, b, t)) return t;
    return stripExtension(base);
}

void AppMusic::onOpen(lv_obj_t* window_root) {
    root_ = window_root;

    // Speaker config tuned for smoother audio on Cardputer
    auto spk_cfg = M5Cardputer.Speaker.config();
    spk_cfg.sample_rate = 128000; // match reference for smoother output
    spk_cfg.task_pinned_core = APP_CPU_NUM;
    M5Cardputer.Speaker.config(spk_cfg);

    buildUI(root_);

    // Initialize SD with board-specific SPI pins
    if (!sd_.initialize()) {
        updateStatus("SD init failed");
        return;
    }

    M5Cardputer.Speaker.setVolume((1 * 255) / 10);
    if (player_volume_) {
        lv_slider_set_range(player_volume_, 0, 10);
        lv_slider_set_value(player_volume_, 1, LV_ANIM_OFF);
    }

    // Initialize dedicated audio task and command queue
    initializeAudioTask();
    sendAudioCommand(AUDIO_CMD_VOLUME, 1);

    // Scan SD for music files
    scanMusic();
    populateArtistList();
}

void AppMusic::onTick() {
    // Drive UI updates from audio status and handle next/prev requests
    handleNextPrevRequests();
    updateUIFromAudioStatus();
}

void AppMusic::onClose() {
    // Request audio task shutdown; audio task will perform resource cleanup
    Serial.println("[Music] onClose: requesting audio shutdown");
    sendAudioCommand(AUDIO_CMD_SHUTDOWN);
    if (audioTaskHandle_) {
        // Allow task to exit gracefully
        Serial.printf("[Music] onClose: waiting 50ms, task=%p\n", (void*)audioTaskHandle_);
        vTaskDelay(pdMS_TO_TICKS(50));
        audioTaskHandle_ = nullptr;
    }
    // UI objects are deleted by WindowSystem when container is destroyed
}

void AppMusic::buildUI(lv_obj_t* parent) {
    // Compact layout: remove extra padding and make views fill window
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 0, 0);
    lv_obj_set_style_pad_row(parent, 0, 0);
    ui_theme::apply_small_text_recursive(parent);

    // List view fills the whole window
    now_playing_ = nullptr; // no header label in compact mode
    status_ = nullptr;      // no status label in list view
    volume_ = nullptr;      // no volume slider in list view

    list_ = lv_list_create(parent);
    lv_obj_set_size(list_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(list_, 1);
    lv_obj_add_flag(list_, LV_OBJ_FLAG_SCROLLABLE);
    // Apply compact list menu style (16px item height)
    ui_theme::apply_list_menu(list_);

    // Player view container (initially hidden), also fills window
    player_view_ = lv_obj_create(parent);
    lv_obj_set_size(player_view_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(player_view_, 1);
    lv_obj_set_flex_flow(player_view_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(player_view_, 2, 0);
    lv_obj_set_style_pad_row(player_view_, 2, 0);
    lv_obj_add_flag(player_view_, LV_OBJ_FLAG_HIDDEN);

    content_col_ = lv_obj_create(player_view_);
    lv_obj_set_flex_grow(content_col_, 1);
    lv_obj_set_style_pad_all(content_col_, 2, 0);
    lv_obj_set_style_pad_row(content_col_, 0, 0);
    lv_obj_set_flex_flow(content_col_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_height(content_col_, LV_SIZE_CONTENT);
    lv_obj_set_style_border_width(content_col_, 0, 0);
    lv_obj_set_style_border_opa(content_col_, LV_OPA_TRANSP, 0);

    control_col_ = lv_obj_create(player_view_);
    lv_obj_set_width(control_col_, lv_pct(100));
    lv_obj_set_style_pad_all(control_col_, 2, 0);
    lv_obj_set_style_pad_row(control_col_, 4, 0);
    lv_obj_set_style_pad_top(control_col_, 0, 0);
    lv_obj_set_style_pad_bottom(control_col_, 0, 0);
    lv_obj_set_flex_flow(control_col_, LV_FLEX_FLOW_ROW);
    lv_obj_set_height(control_col_, 18);
    lv_obj_set_style_border_width(control_col_, 0, 0);
    lv_obj_set_style_border_opa(control_col_, LV_OPA_TRANSP, 0);
    lv_obj_move_to_index(control_col_, 0);

    track_name_ = lv_label_create(content_col_);
    lv_label_set_text(track_name_, "Track: -");
    lv_label_set_long_mode(track_name_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_line_space(track_name_, 0, 0);
    lv_obj_set_style_min_height(track_name_, 8, 0);
    lyric_prev_ = lv_label_create(content_col_);
    lv_label_set_text(lyric_prev_, "");
    lv_obj_set_style_text_align(lyric_prev_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_ver(lyric_prev_, 0, 0);
    lv_obj_set_style_text_line_space(lyric_prev_, 0, 0);
    lv_obj_set_style_min_height(lyric_prev_, 8, 0);
    lv_label_set_long_mode(lyric_prev_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(lyric_prev_, lv_pct(100));
    lv_obj_set_style_text_color(lyric_prev_, lv_color_hex(0x000000), 0);
    ui_theme::apply_small_text_recursive(lyric_prev_);
    lyric_curr_ = lv_label_create(content_col_);
    lv_label_set_text(lyric_curr_, "");
    lv_obj_set_style_text_align(lyric_curr_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_ver(lyric_curr_, 0, 0);
    lv_obj_set_style_text_line_space(lyric_curr_, 0, 0);
    lv_obj_set_style_min_height(lyric_curr_, 8, 0);
    lv_label_set_long_mode(lyric_curr_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(lyric_curr_, lv_pct(100));
    ui_theme::apply_small_text_recursive(lyric_curr_);
    lv_obj_set_style_text_color(lyric_curr_, lv_color_hex(0x000000), 0);
    lyric_next_ = lv_label_create(content_col_);
    lv_label_set_text(lyric_next_, "");
    lv_obj_set_style_text_align(lyric_next_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_ver(lyric_next_, 0, 0);
    lv_obj_set_style_text_line_space(lyric_next_, 0, 0);
    lv_obj_set_style_min_height(lyric_next_, 8, 0);
    lv_label_set_long_mode(lyric_next_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(lyric_next_, lv_pct(100));
    lv_obj_set_style_text_color(lyric_next_, lv_color_hex(0x000000), 0);
    ui_theme::apply_small_text_recursive(lyric_next_);

    // Back button first (left), then slider (right)
    back_btn_ = lv_btn_create(control_col_);
    lv_obj_set_size(back_btn_, 16, 14);
    lv_obj_set_style_bg_opa(back_btn_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(back_btn_, 0, 0);
    lv_obj_set_style_shadow_opa(back_btn_, LV_OPA_TRANSP, 0);
    lv_obj_t* back_label = lv_label_create(back_btn_);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(back_label, lv_color_hex(0x000000), 0);
    lv_obj_center(back_label);
    lv_obj_add_flag(back_btn_, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_event_cb(back_btn_, on_back_btn_clicked, LV_EVENT_CLICKED, this);

    player_volume_ = lv_slider_create(control_col_);
    lv_obj_set_flex_grow(player_volume_, 1);
    lv_obj_set_height(player_volume_, 8);
    lv_slider_set_range(player_volume_, 0, 10);
    lv_obj_add_event_cb(player_volume_, on_player_volume_event, LV_EVENT_VALUE_CHANGED, this);

    // Removed old back button in content_col_
}

void AppMusic::updateStatus(const char* text) {
    if (status_) lv_label_set_text(status_, text);
}

void AppMusic::updateNowPlaying(const char* text) {
    if (now_playing_) {
        std::string s = std::string("Now Playing: ") + (text ? text : "-");
        lv_label_set_text(now_playing_, s.c_str());
    }
}

void AppMusic::scanMusic() {
    paths_.clear();
    names_.clear();
    if (list_) lv_obj_clean(list_);

    addDir("/");
    addDir("/music");

    if (paths_.empty()) {
        updateStatus("No MP3 files found");
    } else {
        updateStatus("Found MP3 files");
    }
}

void AppMusic::addDir(const char* dir) {
    File root = SD.open(dir);
    if (!root) return;
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            std::string name = file.name();
            // Only accept .mp3 files
            if (ends_with_ci(name, ".mp3")) {
                std::string path;
                if (std::string(dir) == "/") path = std::string("/") + name;
                else path = std::string(dir) + "/" + name;
                paths_.push_back(path);
                names_.push_back(basename(path));
                {
                    std::string base = names_.back();
                    std::string artist, album, title;
                    if (!parseNameParts(base, artist, album, title)) {
                        artist = "未分类";
                        album = "未分类";
                    }
                    category_[artist][album].push_back((int)paths_.size() - 1);
                }
                // Add button to list
                lv_obj_t* btn = lv_list_add_btn(list_, LV_SYMBOL_AUDIO, names_.back().c_str());
                // Ensure compact 16px item style applies to each list button
                ui_theme::apply_list_menu_item(btn);
                lv_obj_add_event_cb(btn, on_list_item_clicked, LV_EVENT_CLICKED, this);
            }
        }
        file = root.openNextFile();
    }
    root.close();
}

void AppMusic::populateArtistList() {
    if (!list_) return;
    lv_obj_clean(list_);
    list_level_ = LEVEL_ARTIST;
    current_artist_.clear();
    current_album_.clear();
    for (const auto& kv : category_) {
        lv_obj_t* btn = lv_list_add_btn(list_, LV_SYMBOL_DIRECTORY, kv.first.c_str());
        ui_theme::apply_list_menu_item(btn);
        lv_obj_add_event_cb(btn, on_artist_item_clicked, LV_EVENT_CLICKED, this);
    }
    rebuildFocusGroup();
}

void AppMusic::populateAlbumList(const std::string& artist) {
    if (!list_) return;
    lv_obj_clean(list_);
    list_level_ = LEVEL_ALBUM;
    current_artist_ = artist;
    current_album_.clear();
    lv_obj_t* back = lv_list_add_btn(list_, LV_SYMBOL_LEFT, "返回");
    ui_theme::apply_list_menu_item(back);
    lv_obj_add_event_cb(back, on_album_item_clicked, LV_EVENT_CLICKED, this);
    auto it = category_.find(artist);
    if (it != category_.end()) {
        for (const auto& kv : it->second) {
            lv_obj_t* btn = lv_list_add_btn(list_, LV_SYMBOL_DIRECTORY, kv.first.c_str());
            ui_theme::apply_list_menu_item(btn);
            lv_obj_add_event_cb(btn, on_album_item_clicked, LV_EVENT_CLICKED, this);
        }
    }
    rebuildFocusGroup();
}

void AppMusic::populateTrackList(const std::string& artist, const std::string& album) {
    if (!list_) return;
    lv_obj_clean(list_);
    list_level_ = LEVEL_TRACK;
    current_artist_ = artist;
    current_album_ = album;
    lv_obj_t* back = lv_list_add_btn(list_, LV_SYMBOL_LEFT, "返回");
    ui_theme::apply_list_menu_item(back);
    lv_obj_add_event_cb(back, on_track_item_clicked, LV_EVENT_CLICKED, this);
    auto ita = category_.find(artist);
    if (ita == category_.end()) return;
    auto ita2 = ita->second.find(album);
    if (ita2 == ita->second.end()) return;
    for (int idx : ita2->second) {
        std::string t = extractTitle(names_[idx]);
        lv_obj_t* btn = lv_list_add_btn(list_, LV_SYMBOL_AUDIO, t.c_str());
        ui_theme::apply_list_menu_item(btn);
        lv_obj_add_event_cb(btn, on_track_item_clicked, LV_EVENT_CLICKED, this);
    }
    rebuildFocusGroup();
}

bool AppMusic::is_focusable(lv_obj_t* obj) {
    if (!lv_obj_is_valid(obj)) return false;
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) return false;
    if (lv_obj_has_class(obj, &lv_btn_class)) return true;
    if (lv_obj_has_class(obj, &lv_slider_class)) return true;
    if (lv_obj_has_class(obj, &lv_textarea_class)) return true;
    if (lv_obj_has_class(obj, &lv_dropdown_class)) return true;
    if (lv_obj_is_editable(obj)) return true;
    return false;
}

void AppMusic::add_focusables_recursive(lv_obj_t* node, lv_group_t* group) {
    if (!node || !group) return;
    if (is_focusable(node)) lv_group_add_obj(group, node);
    uint32_t n = lv_obj_get_child_cnt(node);
    for (uint32_t i = 0; i < n; ++i) {
        lv_obj_t* child = lv_obj_get_child(node, i);
        add_focusables_recursive(child, group);
    }
}

void AppMusic::rebuildFocusGroup() {
    lv_indev_t* indev = kb_get_indev();
    if (!indev || !root_) return;
    lv_group_t* group = kb_get_group();
    if (!group) return;
    lv_group_remove_all_objs(group);
    add_focusables_recursive(root_, group);
    lv_indev_set_group(indev, group);
    if (list_ && lv_obj_get_child_cnt(list_) > 0) {
        lv_obj_t* first = lv_obj_get_child(list_, 0);
        if (first) lv_group_focus_obj(first);
    }
}

void AppMusic::playTrackByTitle(const char* title) {
    if (!title) return;
    auto ita = category_.find(current_artist_);
    if (ita == category_.end()) return;
    auto ita2 = ita->second.find(current_album_);
    if (ita2 == ita->second.end()) return;
    for (int idx : ita2->second) {
        std::string a, b, t;
        if (!parseNameParts(names_[idx], a, b, t)) t = stripExtension(names_[idx]);
        if (t == title) { playIndex(idx); return; }
    }
}

void AppMusic::playByName(const char* name) {
    if (!name) return;
    for (size_t i = 0; i < names_.size(); ++i) {
        if (names_[i] == name) {
            playIndex(static_cast<int>(i));
            return;
        }
    }
}

void AppMusic::switchToPlayerView(const char* trackName) {
    if (in_player_mode_) return;
    in_player_mode_ = true;

    // Hide list view elements
    if (now_playing_) lv_obj_add_flag(now_playing_, LV_OBJ_FLAG_HIDDEN);
    if (list_) lv_obj_add_flag(list_, LV_OBJ_FLAG_HIDDEN);
    if (volume_) { lv_obj_t* bar = lv_obj_get_parent(volume_); if (bar) lv_obj_add_flag(bar, LV_OBJ_FLAG_HIDDEN); }
    if (status_) { lv_obj_t* bar = lv_obj_get_parent(status_); if (bar) lv_obj_add_flag(bar, LV_OBJ_FLAG_HIDDEN); }

    // Show player view
    lv_obj_clear_flag(player_view_, LV_OBJ_FLAG_HIDDEN);

    // Set track name
    if (track_name_) {
        std::string s = std::string("Track: ") + (trackName ? trackName : "-");
        lv_label_set_text(track_name_, s.c_str());
    }

    // Force redraw
    lv_obj_invalidate(root_);
}

void AppMusic::switchToListView() {
    if (!in_player_mode_) return;
    in_player_mode_ = false;

    // Show list view elements
    if (now_playing_) lv_obj_clear_flag(now_playing_, LV_OBJ_FLAG_HIDDEN);
    if (list_) lv_obj_clear_flag(list_, LV_OBJ_FLAG_HIDDEN);
    if (volume_) { lv_obj_t* bar = lv_obj_get_parent(volume_); if (bar) lv_obj_clear_flag(bar, LV_OBJ_FLAG_HIDDEN); }
    if (status_) { lv_obj_t* bar = lv_obj_get_parent(status_); if (bar) lv_obj_clear_flag(bar, LV_OBJ_FLAG_HIDDEN); }

    // Hide player view
    lv_obj_add_flag(player_view_, LV_OBJ_FLAG_HIDDEN);

    // Force redraw
    lv_obj_invalidate(root_);
}

void AppMusic::on_back_btn_clicked(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    if (!app || lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    app->stopPlayback();
    app->switchToListView();
}

void AppMusic::on_player_volume_event(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        int v = lv_slider_get_value(app->player_volume_);
        if (v < 0) v = 0; if (v > 10) v = 10;
        app->sendAudioCommand(AUDIO_CMD_VOLUME, v);
    }
}

void AppMusic::playIndex(int idx) {
    if (idx < 0 || idx >= (int)paths_.size()) return;
    current_index_ = idx;
    {
        std::string t = extractTitle(names_[idx]);
        switchToPlayerView(t.c_str());
    }
    loadLyricsForPath(paths_[idx]);
    lyric_index_ = -1;
    lyric_pause_accum_ms_ = 0;
    lyric_pause_start_ms_ = 0;
    lyric_initialized_ = false;
    lyric_track_index_ = idx;
    updateLyrics(0);
    sendAudioCommand(AUDIO_CMD_PLAY, 0, paths_[idx].c_str());
}

void AppMusic::stopPlayback() {
    // Delegate stopping to audio task
    sendAudioCommand(AUDIO_CMD_STOP);
}

void AppMusic::on_list_item_clicked(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    lv_event_code_t code = lv_event_get_code(e);
    if (!app || code != LV_EVENT_CLICKED) return;
    lv_obj_t* btn = lv_event_get_target(e);
    if (!btn) return;
    const char* text = lv_list_get_btn_text(app->list_, btn);
    if (text) {
        app->playByName(text);
    }
}

void AppMusic::on_artist_item_clicked(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    if (!app || lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    lv_obj_t* btn = lv_event_get_target(e);
    if (!btn) return;
    const char* text = lv_list_get_btn_text(app->list_, btn);
    if (!text) return;
    std::string s(text);
    app->populateAlbumList(s);
}

void AppMusic::on_album_item_clicked(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    if (!app || lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    lv_obj_t* btn = lv_event_get_target(e);
    if (!btn) return;
    const char* text = lv_list_get_btn_text(app->list_, btn);
    if (!text) return;
    std::string s(text);
    if (s == "返回") { app->populateArtistList(); return; }
    app->populateTrackList(app->current_artist_, s);
}

void AppMusic::on_track_item_clicked(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    if (!app || lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    lv_obj_t* btn = lv_event_get_target(e);
    if (!btn) return;
    const char* text = lv_list_get_btn_text(app->list_, btn);
    if (!text) return;
    std::string s(text);
    if (s == "返回") { app->populateAlbumList(app->current_artist_); return; }
    app->playTrackByTitle(s.c_str());
    if (app->track_name_) {
        std::string label = std::string("Track: ") + s;
        lv_label_set_text(app->track_name_, label.c_str());
    }
}

void AppMusic::on_volume_event(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        int v = lv_slider_get_value(app->volume_);
        if (v < 0) v = 0; if (v > 10) v = 10;
        app->sendAudioCommand(AUDIO_CMD_VOLUME, v);
    }
}

// ===== RTOS audio task implementation =====

void AppMusic::initializeAudioTask() {
    // Clean previous state
    cleanupAudioTask();

    // Create command queue and status mutex
    audioCommandQueue_ = xQueueCreate(10, sizeof(AudioTaskCommand));
    audioStatusMutex_ = xSemaphoreCreateMutex();
    if (!audioCommandQueue_ || !audioStatusMutex_) {
        updateStatus("Audio queue/mutex fail");
        return;
    }

    // Start audio task on core 0
    BaseType_t ok = xTaskCreatePinnedToCore(
        audioTaskThunk,
        "AudioTask",
        8192,
        this,
        1,
        &audioTaskHandle_,
        0
    );
    if (ok != pdPASS) {
        updateStatus("Audio task create fail");
        vQueueDelete(audioCommandQueue_); audioCommandQueue_ = nullptr;
        vSemaphoreDelete(audioStatusMutex_); audioStatusMutex_ = nullptr;
        return;
    }
    audio_initialized_ = true;
    Serial.printf("[Music] audio task created: handle=%p, queue=%p, mutex=%p\n", (void*)audioTaskHandle_, (void*)audioCommandQueue_, (void*)audioStatusMutex_);
}

void AppMusic::audioTaskThunk(void* parameter) {
    AppMusic* app = static_cast<AppMusic*>(parameter);
    Serial.println("[Music] audioTaskThunk: start");
    app->audioTaskLoop();
}

void AppMusic::audioTaskLoop() {
    // Build audio components inside task
    Serial.println("[Music] audioTaskLoop: start");
    file_ = new AudioFileSourceSD();
    out_ = new AudioOutputM5Speaker(&M5Cardputer.Speaker, 0);
    mp3_ = new AudioGeneratorMP3();
    id3_ = nullptr;

    if (!out_ || !mp3_ || !file_) {
        updateAudioError("Audio components alloc fail");
        cleanupAudioTask();
        vTaskDelete(nullptr);
        return;
    }

    if (!out_->begin()) {
        updateAudioError("Audio out begin fail");
        cleanupAudioTask();
        vTaskDelete(nullptr);
        return;
    }

    // Set output format (MP3 defaults)
    out_->SetRate(44100);
    out_->SetBitsPerSample(16);
    out_->SetChannels(2);

    AudioTaskCommand command{};
    for (;;) {
        // If playing, drive decoder
        if (mp3_ && mp3_->isRunning()) {
            if (!mp3_->loop()) {
                // Finished current track
                stopAudioPlaybackInternal();
                updateAudioError("Song finished");
            }
        }

        // Handle commands
        if (xQueueReceive(audioCommandQueue_, &command, pdMS_TO_TICKS(1)) == pdTRUE) {
            Serial.printf("[Music] audioTaskLoop: received cmd=%d param=%d file='%s'\n", (int)command.cmd, (int)command.param, command.filePath);
            switch (command.cmd) {
                case AUDIO_CMD_PLAY:
                    if (strlen(command.filePath) > 0) {
                        playAudioFile(command.filePath);
                    } else {
                        resumeAudioPlayback();
                    }
                    break;
                case AUDIO_CMD_PAUSE:
                    pauseAudioPlayback();
                    break;
                case AUDIO_CMD_STOP:
                    stopAudioPlaybackInternal();
                    break;
                case AUDIO_CMD_NEXT:
                    updateAudioError("Next requested");
                    break;
                case AUDIO_CMD_PREV:
                    updateAudioError("Previous requested");
                    break;
                case AUDIO_CMD_VOLUME:
                    setAudioVolume(command.param);
                    break;
                case AUDIO_CMD_SHUTDOWN:
                    stopAudioPlaybackInternal();
                    cleanupAudioTask();
                    // Delete RTOS primitives from within the audio task to avoid races
                    if (audioCommandQueue_) { vQueueDelete(audioCommandQueue_); audioCommandQueue_ = nullptr; }
                    if (audioStatusMutex_) { vSemaphoreDelete(audioStatusMutex_); audioStatusMutex_ = nullptr; }
                    Serial.println("[Music] audioTaskLoop: shutdown cleanup complete; deleting task");
                    vTaskDelete(nullptr);
                    return;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void AppMusic::sendAudioCommand(AudioCommand cmd, int param, const char* filePath) {
    if (!audioCommandQueue_) return;
    AudioTaskCommand c{};
    c.cmd = cmd;
    c.param = param;
    if (filePath) {
        strncpy(c.filePath, filePath, sizeof(c.filePath) - 1);
        c.filePath[sizeof(c.filePath) - 1] = '\0';
    } else {
        c.filePath[0] = '\0';
    }
    xQueueSend(audioCommandQueue_, &c, 0);
}

void AppMusic::playAudioFile(const char* filePath) {
    // Stop current first
    stopAudioPlaybackInternal();
    vTaskDelay(pdMS_TO_TICKS(50));

    if (!filePath || strlen(filePath) == 0) {
        updateAudioError("Invalid file path");
        return;
    }

    if (!file_->open(filePath)) {
        updateAudioError("Open failed");
        return;
    }

    id3_ = new AudioFileSourceID3(file_);
    if (!id3_) {
        updateAudioError("ID3 alloc fail");
        file_->close();
        return;
    }

    if (mp3_->begin(id3_, out_)) {
        updateAudioStatus(true, false, filePath);
    } else {
        updateAudioError("Playback init fail");
        delete id3_; id3_ = nullptr;
        file_->close();
    }
}

void AppMusic::pauseAudioPlayback() {
    if (mp3_ && mp3_->isRunning()) {
        mp3_->stop();
        updateAudioStatus(false, true, nullptr);
    }
}

void AppMusic::resumeAudioPlayback() {
    // No-op: rely on PLAY with filePath to resume
}

void AppMusic::stopAudioPlaybackInternal() {
    if (mp3_) {
        if (mp3_->isRunning()) mp3_->stop();
    }
    if (out_) { out_->flush(); out_->stop(); }
    if (id3_) { delete id3_; id3_ = nullptr; }
    if (file_) { if (file_->isOpen()) file_->close(); }
    updateAudioStatus(false, false, nullptr);
}

void AppMusic::cleanupAudioTask() {
    if (mp3_) { delete mp3_; mp3_ = nullptr; }
    if (out_) { delete out_; out_ = nullptr; }
    if (id3_) { delete id3_; id3_ = nullptr; }
    if (file_) { delete file_; file_ = nullptr; }
}

void AppMusic::setAudioVolume(int volume) {
    if (volume < 0) volume = 0; if (volume > 10) volume = 10;
    M5Cardputer.Speaker.setVolume((volume * 255) / 10);
    if (audioStatusMutex_ && xSemaphoreTake(audioStatusMutex_, pdMS_TO_TICKS(5)) == pdTRUE) {
        audioStatus_.currentVolume = volume;
        xSemaphoreGive(audioStatusMutex_);
    }
}

void AppMusic::updateAudioStatus(bool playing, bool paused, const char* songPath) {
    if (!audioStatusMutex_) return;
    if (xSemaphoreTake(audioStatusMutex_, pdMS_TO_TICKS(10)) == pdTRUE) {
        audioStatus_.isPlaying = playing;
        audioStatus_.isPaused = paused;
        audioStatus_.hasError = false;
        audioStatus_.errorMessage[0] = '\0';
        if (songPath && strlen(songPath) > 0) {
            const char* base = strrchr(songPath, '/');
            base = base ? base + 1 : songPath;
            strncpy(audioStatus_.currentSongName, base, sizeof(audioStatus_.currentSongName) - 1);
            audioStatus_.currentSongName[sizeof(audioStatus_.currentSongName) - 1] = '\0';
            // Find index
            for (size_t i = 0; i < paths_.size(); ++i) {
                if (paths_[i] == songPath) { audioStatus_.currentFileIndex = (int)i; break; }
            }
        } else if (!playing) {
            audioStatus_.currentSongName[0] = '\0';
            audioStatus_.currentFileIndex = -1;
        }
        xSemaphoreGive(audioStatusMutex_);
    }
}

void AppMusic::updateAudioError(const char* errorMsg) {
    if (!audioStatusMutex_) return;
    if (xSemaphoreTake(audioStatusMutex_, pdMS_TO_TICKS(10)) == pdTRUE) {
        audioStatus_.hasError = true;
        strncpy(audioStatus_.errorMessage, errorMsg ? errorMsg : "", sizeof(audioStatus_.errorMessage) - 1);
        audioStatus_.errorMessage[sizeof(audioStatus_.errorMessage) - 1] = '\0';
        xSemaphoreGive(audioStatusMutex_);
    }
}

void AppMusic::updateUIFromAudioStatus() {
    if (!audioStatusMutex_) return;
    if (xSemaphoreTake(audioStatusMutex_, pdMS_TO_TICKS(5)) == pdTRUE) {
        // Now playing label
        if (audioStatus_.isPlaying && strlen(audioStatus_.currentSongName) > 0) {
            updateNowPlaying(audioStatus_.currentSongName);
            updateStatus("Playing");
            if (!lyric_initialized_ && audioStatus_.currentFileIndex == lyric_track_index_) {
                lyric_start_ms_ = millis();
                lyric_initialized_ = true;
                lyric_pause_accum_ms_ = 0;
                lyric_pause_start_ms_ = 0;
            }
            if (lyric_pause_start_ms_ != 0) {
                lyric_pause_accum_ms_ += millis() - lyric_pause_start_ms_;
                lyric_pause_start_ms_ = 0;
            }
            uint32_t elapsed = 0;
            if (lyric_initialized_) {
                uint32_t now = millis();
                if (now >= lyric_start_ms_) elapsed = now - lyric_start_ms_;
                if (elapsed >= lyric_pause_accum_ms_) elapsed -= lyric_pause_accum_ms_;
                else elapsed = 0;
            }
            updateLyrics(elapsed);
        } else if (audioStatus_.isPaused) {
            updateNowPlaying("Paused");
            updateStatus("Paused");
            if (lyric_pause_start_ms_ == 0) {
                lyric_pause_start_ms_ = millis();
            }
        } else {
            updateNowPlaying("-");
        }
        // Error status
        if (audioStatus_.hasError && strlen(audioStatus_.errorMessage) > 0) {
            updateStatus(audioStatus_.errorMessage);
        }
        xSemaphoreGive(audioStatusMutex_);
    }
}

std::string AppMusic::replaceExtension(const std::string& path, const char* newExt) {
    size_t p = path.find_last_of('.');
    if (p == std::string::npos) return path + newExt;
    return path.substr(0, p) + newExt;
}

uint32_t AppMusic::parse_lrc_timestamp(const char* p, size_t len) {
    int mm = 0, ss = 0, xx = 0;
    const char* end = p + len;
    const char* c = p;
    while (c < end && *c >= '0' && *c <= '9') { mm = mm * 10 + (*c - '0'); c++; }
    if (c < end && (*c == ':' || *c == '.')) c++;
    while (c < end && *c >= '0' && *c <= '9') { ss = ss * 10 + (*c - '0'); c++; }
    if (c < end && (*c == '.' || *c == ':')) c++;
    while (c < end && *c >= '0' && *c <= '9') { xx = xx * 10 + (*c - '0'); c++; }
    return (uint32_t)mm * 60000u + (uint32_t)ss * 1000u + (uint32_t)xx;
}

void AppMusic::clearLyrics() {
    lyrics_.clear();
    lyric_index_ = -1;
    if (lyric_prev_) lv_label_set_text(lyric_prev_, "");
    if (lyric_curr_) lv_label_set_text(lyric_curr_, "");
    if (lyric_next_) lv_label_set_text(lyric_next_, "");
}

void AppMusic::loadLyricsForPath(const std::string& mp3_path) {
    clearLyrics();
    std::string lrc = replaceExtension(mp3_path, ".lrc");
    File f = SD.open(lrc.c_str());
    if (!f) return;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;
        std::string s = std::string(line.c_str());
        size_t pos = 0;
        std::vector<uint32_t> times;
        while (true) {
            size_t lb = s.find('[', pos);
            if (lb == std::string::npos) break;
            size_t rb = s.find(']', lb + 1);
            if (rb == std::string::npos) break;
            uint32_t t = parse_lrc_timestamp(s.c_str() + lb + 1, rb - lb - 1);
            times.push_back(t);
            pos = rb + 1;
        }
        size_t last_rb = s.rfind(']');
        std::string text = last_rb != std::string::npos ? s.substr(last_rb + 1) : s;
        if (text.size() == 0) continue;
        for (auto t : times) {
            lyrics_.push_back({t, text});
        }
    }
    f.close();
    if (!lyrics_.empty()) {
        std::sort(lyrics_.begin(), lyrics_.end(), [](const LyricLine& a, const LyricLine& b){ return a.t < b.t; });
    }
}

void AppMusic::updateLyrics(uint32_t elapsed_ms) {
    if (lyrics_.empty()) return;
    int idx = lyric_index_;
    if (idx < 0 || (size_t)idx >= lyrics_.size() || elapsed_ms < lyrics_[idx].t || (idx + 1 < (int)lyrics_.size() && elapsed_ms >= lyrics_[idx + 1].t)) {
        int lo = 0, hi = (int)lyrics_.size() - 1, ans = 0;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            if (lyrics_[mid].t <= elapsed_ms) { ans = mid; lo = mid + 1; }
            else { hi = mid - 1; }
        }
        idx = ans;
        lyric_index_ = idx;
    }
    const char* prev = idx > 0 ? lyrics_[idx - 1].s.c_str() : "";
    const char* curr = lyrics_[idx].s.c_str();
    const char* next = (idx + 1 < (int)lyrics_.size()) ? lyrics_[idx + 1].s.c_str() : "";
    if (lyric_prev_) lv_label_set_text(lyric_prev_, prev);
    if (lyric_curr_) lv_label_set_text(lyric_curr_, curr);
    if (lyric_next_) lv_label_set_text(lyric_next_, next);
}

void AppMusic::handleNextPrevRequests() {
    if (!audioStatusMutex_) return;
    if (xSemaphoreTake(audioStatusMutex_, pdMS_TO_TICKS(1)) == pdTRUE) {
        if (audioStatus_.hasError) {
            if (strstr(audioStatus_.errorMessage, "Song finished") || strstr(audioStatus_.errorMessage, "Next requested")) {
                audioStatus_.hasError = false;
                audioStatus_.errorMessage[0] = '\0';
                xSemaphoreGive(audioStatusMutex_);
                playNextSong();
                return;
            } else if (strstr(audioStatus_.errorMessage, "Previous requested")) {
                audioStatus_.hasError = false;
                audioStatus_.errorMessage[0] = '\0';
                xSemaphoreGive(audioStatusMutex_);
                playPreviousSong();
                return;
            }
        }
        xSemaphoreGive(audioStatusMutex_);
    }
}

void AppMusic::playNextSong() {
    if (paths_.empty()) return;
    int next = current_index_ >= 0 ? (current_index_ + 1) % (int)paths_.size() : 0;
    current_index_ = next;
    updateNowPlaying(names_[next].c_str());
    loadLyricsForPath(paths_[next]);
    lyric_index_ = -1;
    lyric_pause_accum_ms_ = 0;
    lyric_pause_start_ms_ = 0;
    lyric_initialized_ = false;
    lyric_track_index_ = next;
    updateLyrics(0);
    sendAudioCommand(AUDIO_CMD_PLAY, 0, paths_[next].c_str());
}

void AppMusic::playPreviousSong() {
    if (paths_.empty()) return;
    int prev = current_index_ >= 0 ? (current_index_ - 1 + (int)paths_.size()) % (int)paths_.size() : 0;
    current_index_ = prev;
    updateNowPlaying(names_[prev].c_str());
    loadLyricsForPath(paths_[prev]);
    lyric_index_ = -1;
    lyric_pause_accum_ms_ = 0;
    lyric_pause_start_ms_ = 0;
    lyric_initialized_ = false;
    lyric_track_index_ = prev;
    updateLyrics(0);
    sendAudioCommand(AUDIO_CMD_PLAY, 0, paths_[prev].c_str());
}