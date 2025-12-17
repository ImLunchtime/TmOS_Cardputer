#pragma once
#include <lvgl.h>
#include <M5Cardputer.h>
#include <vector>
#include <string>
#include <map>

#include "core/window_system.h"
#include "storage/SDFileManager.h"

// ESP8266Audio headers
#include <AudioOutput.h>
#include <AudioFileSourceSD.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>

// FreeRTOS (ESP32)
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

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

// Music app: scans SD for MP3s and plays via a dedicated RTOS audio task
class AppMusic : public IApp {
public:
    AppMusic() = default;
    ~AppMusic() override = default;

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
    lv_obj_t* player_view_ = nullptr;
    lv_obj_t* track_name_ = nullptr;
    lv_obj_t* back_btn_ = nullptr;
    lv_obj_t* player_volume_ = nullptr;
    lv_obj_t* content_col_ = nullptr;
    lv_obj_t* control_col_ = nullptr;
    lv_obj_t* lyric_prev_ = nullptr;
    lv_obj_t* lyric_curr_ = nullptr;
    lv_obj_t* lyric_next_ = nullptr;

    // View state
    bool in_player_mode_ = false;
    enum ListLevel { LEVEL_ARTIST, LEVEL_ALBUM, LEVEL_TRACK };
    ListLevel list_level_ = LEVEL_ARTIST;
    std::string current_artist_;
    std::string current_album_;

    // Files and playback state
    std::vector<std::string> paths_;
    std::vector<std::string> names_;
    std::map<std::string, std::map<std::string, std::vector<int>>> category_;
    int current_index_ = -1;
    bool is_playing_ = false;
    struct LyricLine { uint32_t t; std::string s; };
    std::vector<LyricLine> lyrics_;
    int lyric_index_ = -1;
    uint32_t lyric_start_ms_ = 0;
    uint32_t lyric_pause_accum_ms_ = 0;
    uint32_t lyric_pause_start_ms_ = 0;
    bool lyric_initialized_ = false;
    int lyric_track_index_ = -1;

    // SD manager for proper SPI pin setup
    SDFileManager sd_;

    // Audio components (owned by audio task)
    AudioFileSourceSD* file_ = nullptr;
    AudioFileSourceID3* id3_ = nullptr;
    AudioGeneratorMP3* mp3_ = nullptr;
    AudioOutputM5Speaker* out_ = nullptr;

    // RTOS audio task plumbing
    enum AudioCommand {
        AUDIO_CMD_PLAY,
        AUDIO_CMD_PAUSE,
        AUDIO_CMD_STOP,
        AUDIO_CMD_NEXT,
        AUDIO_CMD_PREV,
        AUDIO_CMD_VOLUME,
        AUDIO_CMD_SHUTDOWN
    };

    struct AudioTaskCommand {
        AudioCommand cmd;
        int param;                 // e.g., volume percent
        char filePath[128];        // path to play (optional)
    };

    struct AudioStatus {
        bool isPlaying = false;
        bool isPaused = false;
        int currentFileIndex = -1;
        int currentVolume = 50;
        char currentSongName[64] = {0};
        bool hasError = false;
        char errorMessage[128] = {0};
    };

    TaskHandle_t audioTaskHandle_ = nullptr;
    QueueHandle_t audioCommandQueue_ = nullptr;
    SemaphoreHandle_t audioStatusMutex_ = nullptr;
    AudioStatus audioStatus_;
    bool audio_initialized_ = false;
    bool error_notified_ = false;

    // UI helpers
    void buildUI(lv_obj_t* parent);
    void updateStatus(const char* text);
    void updateNowPlaying(const char* text);
    void updateUIFromAudioStatus();
    void handleNextPrevRequests();
    void switchToPlayerView(const char* trackName);
    void switchToListView();
    void populateArtistList();
    void populateAlbumList(const std::string& artist);
    void populateTrackList(const std::string& artist, const std::string& album);
    void rebuildFocusGroup();
    static bool is_focusable(lv_obj_t* obj);
    static void add_focusables_recursive(lv_obj_t* node, lv_group_t* group);

    // Files
    void scanMusic();
    void addDir(const char* dir);
    static std::string basename(const std::string& path);
    static std::string stripExtension(const std::string& s);
    static bool parseNameParts(const std::string& base, std::string& artist, std::string& album, std::string& title);
    static std::string extractTitle(const std::string& base);
    static std::string replaceExtension(const std::string& path, const char* newExt);
    static uint32_t parse_lrc_timestamp(const char* p, size_t len);
    void loadLyricsForPath(const std::string& mp3_path);
    void clearLyrics();
    void updateLyrics(uint32_t elapsed_ms);

    // Playback
    void playIndex(int idx);
    void playByName(const char* name);
    void playTrackByTitle(const char* title);
    void stopPlayback();
    void playNextSong();
    void playPreviousSong();

    // Events
    static void on_list_item_clicked(lv_event_t* e);
    static void on_volume_event(lv_event_t* e);
    static void on_back_btn_clicked(lv_event_t* e);
    static void on_player_volume_event(lv_event_t* e);
    static void on_artist_item_clicked(lv_event_t* e);
    static void on_album_item_clicked(lv_event_t* e);
    static void on_track_item_clicked(lv_event_t* e);

    // Audio task (runs on its own core)
    void initializeAudioTask();
    static void audioTaskThunk(void* parameter);
    void audioTaskLoop();

    // Audio task internal helpers
    void sendAudioCommand(AudioCommand cmd, int param = 0, const char* filePath = nullptr);
    void playAudioFile(const char* filePath);
    void pauseAudioPlayback();
    void resumeAudioPlayback();
    void stopAudioPlaybackInternal();
    void cleanupAudioTask();
    void setAudioVolume(int volume);
    void updateAudioStatus(bool playing, bool paused, const char* songPath);
    void updateAudioError(const char* errorMsg);

    // Keyboard shortcuts
    void adjustVolumeDelta(int delta);
};
