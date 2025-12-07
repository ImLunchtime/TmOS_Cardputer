#include "apps/music/app_music.h"
#include <Arduino.h>

void AppMusic::initializeAudioTask() {
    if (audio_initialized_) return;
    audioCommandQueue_ = xQueueCreate(8, sizeof(AudioTaskCommand));
    audioStatusMutex_ = xSemaphoreCreateMutex();
    audio_initialized_ = true;
    xTaskCreatePinnedToCore(audioTaskThunk, "music_audio", 4096, this, 3, &audioTaskHandle_, APP_CPU_NUM);
}

void AppMusic::audioTaskThunk(void* parameter) {
    static_cast<AppMusic*>(parameter)->audioTaskLoop();
}

void AppMusic::audioTaskLoop() {
    bool running = true;
    while (running) {
        AudioTaskCommand cmd;
        while (audioCommandQueue_ && xQueueReceive(audioCommandQueue_, &cmd, 0) == pdTRUE) {
            switch (cmd.cmd) {
                case AUDIO_CMD_PLAY:
                    playAudioFile(cmd.filePath);
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
                    setAudioVolume(cmd.param);
                    break;
                case AUDIO_CMD_SHUTDOWN:
                    running = false;
                    break;
            }
        }
        if (mp3_) {
            bool paused = false;
            if (audioStatusMutex_ && xSemaphoreTake(audioStatusMutex_, pdMS_TO_TICKS(1)) == pdTRUE) {
                paused = audioStatus_.isPaused;
                xSemaphoreGive(audioStatusMutex_);
            }
            if (!paused && mp3_->isRunning()) {
                if (!mp3_->loop()) {
                    updateAudioError("Song finished");
                    stopAudioPlaybackInternal();
                }
            } else {
                vTaskDelay(1);
            }
        } else {
            vTaskDelay(1);
        }
    }
    cleanupAudioTask();
    vTaskDelete(nullptr);
}

void AppMusic::sendAudioCommand(AudioCommand cmd, int param, const char* filePath) {
    if (!audioCommandQueue_) return;
    AudioTaskCommand c;
    c.cmd = cmd;
    c.param = param;
    c.filePath[0] = '\0';
    if (filePath) {
        strncpy(c.filePath, filePath, sizeof(c.filePath) - 1);
        c.filePath[sizeof(c.filePath) - 1] = '\0';
    }
    xQueueSend(audioCommandQueue_, &c, 0);
}

void AppMusic::playAudioFile(const char* filePath) {
    stopAudioPlaybackInternal();
    if (!filePath || strlen(filePath) == 0) return;
    file_ = new AudioFileSourceSD(filePath);
    if (!file_ || !file_->isOpen()) {
        updateAudioError("Open failed");
        if (file_) { delete file_; file_ = nullptr; }
        return;
    }
    id3_ = new AudioFileSourceID3(file_);
    out_ = new AudioOutputM5Speaker(&M5Cardputer.Speaker);
    mp3_ = new AudioGeneratorMP3();
    mp3_->begin(id3_, out_);
    updateAudioStatus(true, false, filePath);
}

void AppMusic::pauseAudioPlayback() {
    updateAudioStatus(true, true, nullptr);
}

void AppMusic::resumeAudioPlayback() {
    updateAudioStatus(true, false, nullptr);
}

void AppMusic::stopAudioPlaybackInternal() {
    if (mp3_) { mp3_->stop(); delete mp3_; mp3_ = nullptr; }
    if (out_) { out_->stop(); delete out_; out_ = nullptr; }
    if (id3_) { delete id3_; id3_ = nullptr; }
    if (file_) { delete file_; file_ = nullptr; }
    updateAudioStatus(false, false, nullptr);
}

void AppMusic::cleanupAudioTask() {
    stopAudioPlaybackInternal();
}

void AppMusic::setAudioVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 10) volume = 10;
    M5Cardputer.Speaker.setVolume((volume * 255) / 10);
    if (audioStatusMutex_) {
        if (xSemaphoreTake(audioStatusMutex_, pdMS_TO_TICKS(1)) == pdTRUE) {
            audioStatus_.currentVolume = volume;
            xSemaphoreGive(audioStatusMutex_);
        }
    }
}

void AppMusic::updateAudioStatus(bool playing, bool paused, const char* songPath) {
    if (!audioStatusMutex_) return;
    if (xSemaphoreTake(audioStatusMutex_, pdMS_TO_TICKS(1)) == pdTRUE) {
        audioStatus_.isPlaying = playing;
        audioStatus_.isPaused = paused;
        audioStatus_.hasError = false;
        audioStatus_.errorMessage[0] = '\0';
        if (songPath) {
            std::string base = basename(songPath);
            std::string title = extractTitle(base);
            strncpy(audioStatus_.currentSongName, title.c_str(), sizeof(audioStatus_.currentSongName) - 1);
            audioStatus_.currentSongName[sizeof(audioStatus_.currentSongName) - 1] = '\0';
        }
        xSemaphoreGive(audioStatusMutex_);
    }
}

void AppMusic::updateAudioError(const char* errorMsg) {
    if (!audioStatusMutex_) return;
    if (xSemaphoreTake(audioStatusMutex_, pdMS_TO_TICKS(1)) == pdTRUE) {
        audioStatus_.hasError = true;
        if (errorMsg) {
            strncpy(audioStatus_.errorMessage, errorMsg, sizeof(audioStatus_.errorMessage) - 1);
            audioStatus_.errorMessage[sizeof(audioStatus_.errorMessage) - 1] = '\0';
        } else {
            audioStatus_.errorMessage[0] = '\0';
        }
        xSemaphoreGive(audioStatusMutex_);
    }
}

void AppMusic::adjustVolumeDelta(int delta) {
    if (!player_volume_) return;
    int v = lv_slider_get_value(player_volume_);
    v += delta;
    if (v < 0) v = 0;
    if (v > 10) v = 10;
    lv_slider_set_value(player_volume_, v, LV_ANIM_OFF);
    sendAudioCommand(AUDIO_CMD_VOLUME, v);
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

void AppMusic::playIndex(int idx) {
    if (idx < 0 || idx >= (int)paths_.size()) return;
    current_index_ = idx;
    updateNowPlaying(names_[idx].c_str());
    loadLyricsForPath(paths_[idx]);
    lyric_index_ = -1;
    lyric_pause_accum_ms_ = 0;
    lyric_pause_start_ms_ = 0;
    lyric_initialized_ = false;
    lyric_track_index_ = idx;
    updateLyrics(0);
    switchToPlayerView(names_[idx].c_str());
    sendAudioCommand(AUDIO_CMD_PLAY, 0, paths_[idx].c_str());
}

void AppMusic::playByName(const char* name) {
    if (!name) return;
    for (int i = 0; i < (int)names_.size(); ++i) {
        if (names_[i] == name) { playIndex(i); return; }
    }
}

void AppMusic::playTrackByTitle(const char* title) {
    if (!title) return;
    for (int i = 0; i < (int)names_.size(); ++i) {
        std::string t = extractTitle(names_[i]);
        if (t == title) { playIndex(i); return; }
    }
}

void AppMusic::stopPlayback() {
    sendAudioCommand(AUDIO_CMD_STOP);
}
