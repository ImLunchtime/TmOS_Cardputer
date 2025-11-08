#include "app_music.h"
#include "theme.h"
#include <SD.h>

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

void AppMusic::onOpen(lv_obj_t* window_root) {
    root_ = window_root;

    // Speaker config tuned for smoother audio on Cardputer
    auto spk_cfg = M5Cardputer.Speaker.config();
    spk_cfg.sample_rate = 96000;
    spk_cfg.task_pinned_core = APP_CPU_NUM;
    M5Cardputer.Speaker.config(spk_cfg);

    buildUI(root_);

    // Initialize SD with board-specific SPI pins
    if (!sd_.initialize()) {
        updateStatus("SD init failed");
        return;
    }

    // Initial volume 50%
    lv_slider_set_range(volume_, 0, 100);
    lv_slider_set_value(volume_, 50, LV_ANIM_OFF);
    M5Cardputer.Speaker.setVolume((50 * 255) / 100);

    // Scan SD for music files
    scanMusic();
}

void AppMusic::onTick() {
    // Drive MP3 playback
    if (mp3_) {
        if (mp3_->isRunning()) {
            if (!mp3_->loop()) {
                // Finished
                updateStatus("Finished");
                stopPlayback();
            }
        }
    }
}

void AppMusic::onClose() {
    stopPlayback();
    // UI objects are deleted by WindowSystem when container is destroyed
}

void AppMusic::buildUI(lv_obj_t* parent) {
    // Vertical layout
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 6, 0);
    lv_obj_set_style_pad_row(parent, 6, 0);
    ui_theme::apply_small_text_recursive(parent);

    // Now playing label
    now_playing_ = lv_label_create(parent);
    lv_label_set_text(now_playing_, "Now Playing: -");

    // File list
    list_ = lv_list_create(parent);
    lv_obj_set_size(list_, lv_pct(100), lv_pct(70));
    lv_obj_add_flag(list_, LV_OBJ_FLAG_SCROLLABLE);

    // Bottom bar with volume and status
    lv_obj_t* bar = lv_obj_create(parent);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(bar, lv_pct(100), lv_pct(20));
    ui_theme::apply_window(bar);

    // Volume slider
    volume_ = lv_slider_create(bar);
    lv_obj_set_width(volume_, lv_pct(60));
    lv_obj_add_event_cb(volume_, on_volume_event, LV_EVENT_VALUE_CHANGED, this);

    // Status label
    status_ = lv_label_create(bar);
    lv_label_set_text(status_, "Ready");
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
                // Add button to list
                lv_obj_t* btn = lv_list_add_btn(list_, LV_SYMBOL_AUDIO, names_.back().c_str());
                lv_obj_add_event_cb(btn, on_list_item_clicked, LV_EVENT_CLICKED, this);
            }
        }
        file = root.openNextFile();
    }
    root.close();
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

void AppMusic::playIndex(int idx) {
    if (idx < 0 || idx >= (int)paths_.size()) return;

    stopPlayback();

    // Build audio pipeline
    file_ = new AudioFileSourceSD();
    if (!file_->open(paths_[idx].c_str())) {
        updateStatus("Open failed");
        delete file_; file_ = nullptr;
        return;
    }

    id3_ = new AudioFileSourceID3(file_);
    out_ = new AudioOutputM5Speaker(&M5Cardputer.Speaker, 0);
    mp3_ = new AudioGeneratorMP3();

    if (!mp3_->begin(id3_, out_)) {
        updateStatus("Playback init failed");
        delete mp3_; mp3_ = nullptr;
        delete out_; out_ = nullptr;
        delete id3_; id3_ = nullptr;
        file_->close(); delete file_; file_ = nullptr;
        return;
    }

    current_index_ = idx;
    is_playing_ = true;
    updateNowPlaying(names_[idx].c_str());
    updateStatus("Playing");
}

void AppMusic::stopPlayback() {
    if (mp3_) {
        if (mp3_->isRunning()) mp3_->stop();
        delete mp3_; mp3_ = nullptr;
    }
    if (out_) { out_->stop(); delete out_; out_ = nullptr; }
    if (id3_) { delete id3_; id3_ = nullptr; }
    if (file_) { file_->close(); delete file_; file_ = nullptr; }
    if (is_playing_) {
        is_playing_ = false;
        current_index_ = -1;
    }
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

void AppMusic::on_volume_event(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        int v = lv_slider_get_value(app->volume_);
        if (v < 0) v = 0; if (v > 100) v = 100;
        M5Cardputer.Speaker.setVolume((v * 255) / 100);
    }
}