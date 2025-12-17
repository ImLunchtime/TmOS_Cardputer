#pragma once
#include "core/window_system.h"
#include <vector>
#include <ArduinoJson.h>
#include <lvgl.h>
#include <M5Cardputer.h>

#include <AudioOutput.h>
#include <AudioFileSourceSD.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

struct Station {
    String name;
    String id;
    String audio;
};

struct RouteInfo {
    String id;
    String name;
};

class AppStationReporter : public IApp {
public:
    AppStationReporter();
    ~AppStationReporter() override;

    const char* title() const override { return "Station Report"; }
    ui_theme::ThemeId theme() const override { return ui_theme::ThemeId::Light; }

    void onOpen(lv_obj_t* window_root) override;
    void onTick() override;
    void onClose() override;

private:
    void loadRoute(const String& filename);
    void updateUI();
    void buildTab1(lv_obj_t* parent);
    void buildTab2(lv_obj_t* parent);
    void buildTab3(lv_obj_t* parent);
    void buildTab4(lv_obj_t* parent);
    void refreshStationList();
    void loadRouteList();
    void refreshTipsButtons();
    void start_depart_announcement();

    static void event_handler(lv_event_t* e);

    lv_obj_t* root_ = nullptr;
    lv_obj_t* tabview_ = nullptr;
    
    // Tab 1 Controls
    lv_obj_t* label_route_name_ = nullptr;
    lv_obj_t* label_curr_station_ = nullptr;
    lv_obj_t* label_next_station_ = nullptr;
    lv_obj_t* label_status_ = nullptr;
    
    lv_obj_t* btn_action_ = nullptr; // Main action button (Depart/Arrive)
    lv_obj_t* label_btn_action_ = nullptr;
    
    lv_obj_t* btn_prev_ = nullptr;
    lv_obj_t* btn_emergency_ = nullptr;
    lv_obj_t* btn_announce_ = nullptr;

    // Tab 2 Controls
    lv_obj_t* list_stations_ = nullptr;

    // Tab 3 Controls
    lv_obj_t* list_routes_ = nullptr;
    lv_obj_t* sr_volume_ = nullptr;
    // Tab 4 Controls
    lv_obj_t* tips_list_ = nullptr;

    // Logic
    std::vector<Station> stations_;
    std::vector<RouteInfo> available_routes_;
    struct Tip { String label; String audio; };
    std::vector<Tip> tips_;
    int current_station_idx_ = 0;
    String route_name_ = "Unknown Route";
    String line_audio_;
    std::vector<String> audio_template_;
    
    enum State {
        NOT_STARTED,
        MOVING,     // Moving to next station
        IN_STATION, // Stopped at station
        EMERGENCY   // Emergency stop
    };
    State state_ = NOT_STARTED;
    bool blink_toggle_ = false;
    uint32_t last_blink_time_ = 0;

    std::vector<String> announce_queue_;
    int announce_index_ = -1;
    bool announcement_active_ = false;

    class AudioOutputM5SpeakerSR : public AudioOutput {
    public:
        explicit AudioOutputM5SpeakerSR(m5::Speaker_Class* speaker, uint8_t virtual_channel = 0)
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

    AudioFileSourceSD* file_ = nullptr;
    AudioFileSourceID3* id3_ = nullptr;
    AudioGeneratorMP3* mp3_ = nullptr;
    AudioOutputM5SpeakerSR* out_ = nullptr;

    enum AudioCommand {
        AUDIO_CMD_PLAY,
        AUDIO_CMD_STOP,
        AUDIO_CMD_VOLUME,
        AUDIO_CMD_SHUTDOWN
    };
    struct AudioTaskCommand {
        AudioCommand cmd;
        int param;
        char filePath[128];
    };
    TaskHandle_t audioTaskHandle_ = nullptr;
    QueueHandle_t audioCommandQueue_ = nullptr;
    bool audio_initialized_ = false;

    void initializeAudioTask();
    void cleanupAudioTask();
    static void audioTaskThunk(void* parameter);
    void audioTaskLoop();
    void playAudioFile(const char* filePath);
    void stopAudioPlaybackInternal();
    void setAudioVolume(int volume);
    void sendAudioCommand(AudioCommand cmd, int param = 0, const char* filePath = nullptr);
};
