#include "app_music.h"
#include "theme.h"
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

    // Initial volume 50%
    M5Cardputer.Speaker.setVolume((50 * 255) / 100);
    if (player_volume_) {
        lv_slider_set_range(player_volume_, 0, 100);
        lv_slider_set_value(player_volume_, 50, LV_ANIM_OFF);
    }

    // Initialize dedicated audio task and command queue
    initializeAudioTask();

    // Scan SD for music files
    scanMusic();
}

void AppMusic::onTick() {
    // Drive UI updates from audio status and handle next/prev requests
    handleNextPrevRequests();
    updateUIFromAudioStatus();
}

void AppMusic::onClose() {
    // Request audio task shutdown and clean up RTOS resources
    sendAudioCommand(AUDIO_CMD_SHUTDOWN);
    if (audioTaskHandle_) {
        // Allow task to exit gracefully
        vTaskDelay(pdMS_TO_TICKS(50));
        audioTaskHandle_ = nullptr;
    }
    if (audioCommandQueue_) {
        vQueueDelete(audioCommandQueue_);
        audioCommandQueue_ = nullptr;
    }
    if (audioStatusMutex_) {
        vSemaphoreDelete(audioStatusMutex_);
        audioStatusMutex_ = nullptr;
    }
    cleanupAudioTask();
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
    lv_obj_set_style_pad_all(player_view_, 6, 0);
    lv_obj_set_style_pad_row(player_view_, 6, 0);
    lv_obj_add_flag(player_view_, LV_OBJ_FLAG_HIDDEN);

    // Track name in player view
    track_name_ = lv_label_create(player_view_);
    lv_label_set_text(track_name_, "Track: -");
    lv_label_set_long_mode(track_name_, LV_LABEL_LONG_SCROLL_CIRCULAR);

    // Volume slider in player view (full width)
    player_volume_ = lv_slider_create(player_view_);
    lv_obj_set_width(player_volume_, lv_pct(100));
    lv_slider_set_range(player_volume_, 0, 100);
    lv_slider_set_value(player_volume_, 50, LV_ANIM_OFF);
    lv_obj_add_event_cb(player_volume_, on_player_volume_event, LV_EVENT_VALUE_CHANGED, this);

    // Back button in player view (full width for easy tap)
    back_btn_ = lv_btn_create(player_view_);
    lv_obj_set_width(back_btn_, lv_pct(100));
    lv_obj_t* back_label = lv_label_create(back_btn_);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);
    ui_theme::apply_button(back_btn_);
    lv_obj_add_event_cb(back_btn_, on_back_btn_clicked, LV_EVENT_CLICKED, this);
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
                // Ensure compact 16px item style applies to each list button
                ui_theme::apply_list_menu_item(btn);
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
        if (v < 0) v = 0; if (v > 100) v = 100;
        app->sendAudioCommand(AUDIO_CMD_VOLUME, v);
    }
}

void AppMusic::playIndex(int idx) {
    if (idx < 0 || idx >= (int)paths_.size()) return;
    current_index_ = idx;
    switchToPlayerView(names_[idx].c_str()); // Switch to player view
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

void AppMusic::on_volume_event(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    if (!app) return;
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        int v = lv_slider_get_value(app->volume_);
        if (v < 0) v = 0; if (v > 100) v = 100;
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
}

void AppMusic::audioTaskThunk(void* parameter) {
    AppMusic* app = static_cast<AppMusic*>(parameter);
    app->audioTaskLoop();
}

void AppMusic::audioTaskLoop() {
    // Build audio components inside task
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
    if (volume < 0) volume = 0; if (volume > 100) volume = 100;
    M5Cardputer.Speaker.setVolume((volume * 255) / 100);
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
        } else if (audioStatus_.isPaused) {
            updateNowPlaying("Paused");
            updateStatus("Paused");
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
    sendAudioCommand(AUDIO_CMD_PLAY, 0, paths_[next].c_str());
}

void AppMusic::playPreviousSong() {
    if (paths_.empty()) return;
    int prev = current_index_ >= 0 ? (current_index_ - 1 + (int)paths_.size()) % (int)paths_.size() : 0;
    current_index_ = prev;
    updateNowPlaying(names_[prev].c_str());
    sendAudioCommand(AUDIO_CMD_PLAY, 0, paths_[prev].c_str());
}