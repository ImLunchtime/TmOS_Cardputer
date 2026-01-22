#include "apps/music/app_music.h"
#include "ui/theme.h"
#include "drivers/input_kb.h"
#include <SD.h>
#include "ui/ui_notify.h"

void AppMusic::onOpen(lv_obj_t* window_root) {
    root_ = window_root;

    // Speaker config tuned for smoother audio on Cardputer
    auto spk_cfg = M5Cardputer.Speaker.config();
    spk_cfg.sample_rate = 128000; // match reference for smoother output
    spk_cfg.task_pinned_core = APP_CPU_NUM;
    M5Cardputer.Speaker.config(spk_cfg);

    buildUI(root_);

    if (!sd_.initialize()) {
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "Unable to read SD card", 2500);
        updateStatus("Unable to read SD card");
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

    scanMusic();

    // Register keyboard shortcuts for volume control
    kb_register_app_keys(this, {
        { 'Z', [this](){ return in_player_mode_; }, [this](){ adjustVolumeDelta(+1); } },
        { 'z', [this](){ return in_player_mode_; }, [this](){ adjustVolumeDelta(+1); } },
        { 'X', [this](){ return in_player_mode_; }, [this](){ adjustVolumeDelta(-1); } },
        { 'x', [this](){ return in_player_mode_; }, [this](){ adjustVolumeDelta(-1); } },
    });
}

void AppMusic::onTick() {
    // Drive UI updates from audio status and handle next/prev requests
    handleNextPrevRequests();
    updateUIFromAudioStatus();
    handleScanStep();
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
    kb_clear_app_keys(this);
}

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

// ===== RTOS audio task implementation =====

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 

 
