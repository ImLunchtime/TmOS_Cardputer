#include "apps/station_reporter/app_station_reporter.h"
#include <Arduino.h>

void AppStationReporter::initializeAudioTask() {
    if (audio_initialized_) return;
    audioCommandQueue_ = xQueueCreate(8, sizeof(AudioTaskCommand));
    audio_initialized_ = true;
    xTaskCreatePinnedToCore(audioTaskThunk, "station_audio", 4096, this, 3, &audioTaskHandle_, APP_CPU_NUM);
}

void AppStationReporter::audioTaskThunk(void* parameter) {
    static_cast<AppStationReporter*>(parameter)->audioTaskLoop();
}

void AppStationReporter::audioTaskLoop() {
    bool running = true;
    while (running) {
        AudioTaskCommand cmd;
        while (audioCommandQueue_ && xQueueReceive(audioCommandQueue_, &cmd, 0) == pdTRUE) {
            switch (cmd.cmd) {
                case AUDIO_CMD_PLAY:
                    playAudioFile(cmd.filePath);
                    break;
                case AUDIO_CMD_STOP:
                    stopAudioPlaybackInternal();
                    break;
                case AUDIO_CMD_VOLUME:
                    setAudioVolume(cmd.param);
                    break;
                case AUDIO_CMD_SHUTDOWN:
                    stopAudioPlaybackInternal();
                    running = false;
                    break;
            }
        }
        if (mp3_) {
            bool paused = false;
            if (!paused && mp3_->isRunning()) {
                if (!mp3_->loop()) {
                    stopAudioPlaybackInternal();
                    if (announcement_active_ && announce_index_ + 1 < (int)announce_queue_.size()) {
                        announce_index_++;
                        sendAudioCommand(AUDIO_CMD_PLAY, 0, announce_queue_[announce_index_].c_str());
                    } else {
                        announcement_active_ = false;
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    vTaskDelete(NULL);
}

void AppStationReporter::playAudioFile(const char* filePath) {
    stopAudioPlaybackInternal();
    if (!filePath || strlen(filePath) == 0) return;
    file_ = new AudioFileSourceSD(filePath);
    if (!file_ || !file_->isOpen()) {
        if (file_) { delete file_; file_ = nullptr; }
        return;
    }
    id3_ = new AudioFileSourceID3(file_);
    out_ = new AudioOutputM5SpeakerSR(&M5Cardputer.Speaker);
    mp3_ = new AudioGeneratorMP3();
    mp3_->begin(id3_, out_);
}

void AppStationReporter::stopAudioPlaybackInternal() {
    if (mp3_) { mp3_->stop(); delete mp3_; mp3_ = nullptr; }
    if (out_) { out_->stop(); delete out_; out_ = nullptr; }
    if (id3_) { delete id3_; id3_ = nullptr; }
    if (file_) { delete file_; file_ = nullptr; }
}

void AppStationReporter::cleanupAudioTask() {
    stopAudioPlaybackInternal();
}

void AppStationReporter::setAudioVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 10) volume = 10;
    M5Cardputer.Speaker.setVolume((volume * 255) / 10);
}

void AppStationReporter::sendAudioCommand(AudioCommand cmd, int param, const char* filePath) {
    if (!audioCommandQueue_) return;
    AudioTaskCommand c; c.cmd = cmd; c.param = param; c.filePath[0] = '\0';
    if (filePath) { strncpy(c.filePath, filePath, sizeof(c.filePath) - 1); c.filePath[sizeof(c.filePath) - 1] = '\0'; }
    xQueueSend(audioCommandQueue_, &c, 0);
}

void AppStationReporter::start_depart_announcement() {
    if (stations_.empty()) return;
    int next_idx = current_station_idx_ + 1;
    if (next_idx >= (int)stations_.size()) return;
    String terminal_audio = stations_.back().audio;
    String next_audio = stations_[next_idx].audio;
    announce_queue_.clear();
    for (auto &tok : audio_template_) {
        String f;
        if (tok == "$line" || tok == "$线路") f = line_audio_;
        else if (tok == "$terminal" || tok == "$本线路终点站") f = terminal_audio;
        else if (tok == "$next" || tok == "$下一站") f = next_audio;
        else f = tok;
        if (f.length() > 0) {
            String path = String("/bus_routes/audios/") + f;
            announce_queue_.push_back(path);
        }
    }
    if (!announce_queue_.empty()) {
        announce_index_ = 0;
        announcement_active_ = true;
        sendAudioCommand(AUDIO_CMD_PLAY, 0, announce_queue_[0].c_str());
    }
}
