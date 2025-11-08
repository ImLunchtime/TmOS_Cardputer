#pragma once
#include <lvgl.h>
#include <M5Cardputer.h>
#include <vector>
#include <string>

#include "window_system.h"
#include "SDFileManager.h"

// ESP8266Audio headers
#include <AudioOutput.h>
#include <AudioFileSourceSD.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>

// Lightweight audio output that feeds samples to M5Cardputer speaker
class AudioOutputM5Speaker : public AudioOutput {
public:
    explicit AudioOutputM5Speaker(m5::Speaker_Class* speaker, uint8_t virtual_channel = 0)
        : speaker_(speaker), vch_(virtual_channel) {}

    bool begin() override { return true; }

    bool ConsumeSample(int16_t sample[2]) override {
        if (buf_index_ < kBufSize) {
            buf_[tri_index_][buf_index_] = sample[0];
            buf_[tri_index_][buf_index_ + 1] = sample[1];
            buf_index_ += 2;
            return true;
        }
        flush();
        return false;
    }

    void flush() override {
        if (buf_index_) {
            speaker_->playRaw(buf_[tri_index_], buf_index_, hertz, true, 1, vch_);
            tri_index_ = (tri_index_ < 2) ? tri_index_ + 1 : 0;
            buf_index_ = 0;
        }
    }

    bool stop() override {
        flush();
        speaker_->stop(vch_);
        return true;
    }

private:
    m5::Speaker_Class* speaker_;
    uint8_t vch_;
    static constexpr size_t kBufSize = 640;
    int16_t buf_[3][kBufSize];
    size_t buf_index_ = 0;
    size_t tri_index_ = 0;
};

// Simple Music app: scan SD for MP3s and play selected file
class AppMusic : public IApp {
public:
    AppMusic() = default;
    ~AppMusic() override { onClose(); }

    const char* title() const override { return "Music"; }
    void onOpen(lv_obj_t* window_root) override;
    void onTick() override;
    void onClose() override;

private:
    // UI
    lv_obj_t* root_ = nullptr;
    lv_obj_t* list_ = nullptr;
    lv_obj_t* status_ = nullptr;
    lv_obj_t* now_playing_ = nullptr;
    lv_obj_t* volume_ = nullptr;

    // Files and playback state
    std::vector<std::string> paths_;
    std::vector<std::string> names_;
    int current_index_ = -1;
    bool is_playing_ = false;

    // SD manager for proper SPI pin setup
    SDFileManager sd_;

    // Audio components
    AudioFileSourceSD* file_ = nullptr;
    AudioFileSourceID3* id3_ = nullptr;
    AudioGeneratorMP3* mp3_ = nullptr;
    AudioOutputM5Speaker* out_ = nullptr;

    // UI helpers
    void buildUI(lv_obj_t* parent);
    void updateStatus(const char* text);
    void updateNowPlaying(const char* text);

    // Files
    void scanMusic();
    void addDir(const char* dir);
    static std::string basename(const std::string& path);

    // Playback
    void playIndex(int idx);
    void playByName(const char* name);
    void stopPlayback();

    // Events
    static void on_list_item_clicked(lv_event_t* e);
    static void on_volume_event(lv_event_t* e);
};