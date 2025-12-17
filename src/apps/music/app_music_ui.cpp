#include "apps/music/app_music.h"
#include "ui/theme.h"
#include "drivers/input_kb.h"
#include <lvgl.h>
#include "ui/ui_notify.h"

LV_IMG_DECLARE(music_disc);

static const lv_color_t kMusicAccent = lv_color_hex(0xED6B6B);

static void apply_music_list_item_style(lv_obj_t* btn) {
    if (!btn) return;
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_set_style_bg_color(btn, lv_color_white(), LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_STATE_FOCUSED);
    lv_obj_set_style_text_color(btn, kMusicAccent, LV_STATE_FOCUSED);
    lv_obj_set_style_bg_color(btn, lv_color_white(), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_set_style_text_color(btn, kMusicAccent, LV_STATE_PRESSED);

    uint32_t child_cnt = lv_obj_get_child_cnt(btn);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* c = lv_obj_get_child(btn, i);
        if (!c) continue;
        if (lv_obj_has_class(c, &lv_label_class)) {
            lv_obj_set_style_text_color(c, lv_color_white(), 0);
            lv_obj_set_style_text_color(c, kMusicAccent, LV_STATE_FOCUSED);
            lv_obj_set_style_text_color(c, kMusicAccent, LV_STATE_PRESSED);
        }
    }
}

void AppMusic::buildUI(lv_obj_t* parent) {
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 0, 0);
    lv_obj_set_style_pad_row(parent, 0, 0);
    ui_theme::apply_small_text_recursive(parent);
    lv_obj_set_style_bg_color(parent, kMusicAccent, 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);

    now_playing_ = nullptr;
    status_ = nullptr;
    volume_ = nullptr;

    list_ = lv_list_create(parent);
    lv_obj_set_size(list_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(list_, 1);
    lv_obj_add_flag(list_, LV_OBJ_FLAG_SCROLLABLE);
    ui_theme::apply_list_menu(list_);
    lv_obj_set_style_bg_opa(list_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list_, 0, 0);
    lv_obj_set_style_pad_all(list_, 0, 0);
    lv_obj_set_style_bg_opa(list_, LV_OPA_TRANSP, LV_PART_SCROLLBAR);

    player_view_ = lv_obj_create(parent);
    lv_obj_set_size(player_view_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_grow(player_view_, 1);
    lv_obj_set_flex_flow(player_view_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(player_view_, 2, 0);
    lv_obj_set_style_pad_row(player_view_, 2, 0);
    lv_obj_set_style_bg_opa(player_view_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(player_view_, 0, 0);
    lv_obj_add_flag(player_view_, LV_OBJ_FLAG_HIDDEN);

    content_col_ = lv_obj_create(player_view_);
    lv_obj_set_flex_grow(content_col_, 1);
    lv_obj_set_style_pad_all(content_col_, 2, 0);
    lv_obj_set_style_pad_row(content_col_, 2, 0);
    lv_obj_set_style_pad_column(content_col_, 2, 0);
    lv_obj_set_flex_flow(content_col_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_height(content_col_, lv_pct(100));
    lv_obj_set_style_border_width(content_col_, 0, 0);
    lv_obj_set_style_border_opa(content_col_, LV_OPA_TRANSP, 0);
    lv_obj_set_width(content_col_, lv_pct(100));
    lv_obj_set_style_bg_opa(content_col_, LV_OPA_TRANSP, 0);

    track_name_ = lv_label_create(content_col_);
    lv_label_set_text(track_name_, "Track: -");
    lv_label_set_long_mode(track_name_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_line_space(track_name_, 0, 0);
    lv_obj_set_style_min_height(track_name_, 8, 0);
    lv_obj_set_style_text_color(track_name_, lv_color_white(), 0);

    lv_obj_t* content_row = lv_obj_create(content_col_);
    lv_obj_set_width(content_row, lv_pct(100));
    lv_obj_set_flex_grow(content_row, 1);
    lv_obj_set_flex_flow(content_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_border_width(content_row, 0, 0);
    lv_obj_set_style_bg_opa(content_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(content_row, 0, 0);
    lv_obj_set_style_pad_column(content_row, 4, 0);

    lv_obj_t* disc_box = lv_obj_create(content_row);
    lv_obj_set_size(disc_box, 64, lv_pct(100));
    lv_obj_set_style_bg_opa(disc_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(disc_box, 0, 0);
    lv_obj_set_style_pad_all(disc_box, 0, 0);

    disc_img_ = lv_img_create(disc_box);
    lv_img_set_src(disc_img_, &music_disc);
    lv_obj_set_style_bg_opa(disc_img_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(disc_img_, 0, 0);
    lv_obj_center(disc_img_);

    lyrics_container_ = lv_obj_create(content_row);
    lv_obj_set_flex_grow(lyrics_container_, 1);
    lv_obj_set_height(lyrics_container_, lv_pct(100));
    lv_obj_set_style_bg_opa(lyrics_container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(lyrics_container_, 0, 0);
    lv_obj_set_style_pad_all(lyrics_container_, 0, 0);
    lv_obj_set_scroll_dir(lyrics_container_, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(lyrics_container_, LV_SCROLLBAR_MODE_OFF);

    lyrics_label_ = lv_label_create(lyrics_container_);
    lv_label_set_text(lyrics_label_, "");
    lv_label_set_long_mode(lyrics_label_, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lyrics_label_, lv_pct(100));
    lv_obj_set_style_text_color(lyrics_label_, lv_color_white(), 0);
    ui_theme::apply_small_text_recursive(lyrics_label_);
    lv_obj_set_style_text_line_space(lyrics_label_, 0, 0);

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
    lv_obj_set_style_bg_opa(control_col_, LV_OPA_TRANSP, 0);
    lv_obj_move_to_index(control_col_, 0);

    back_btn_ = lv_btn_create(control_col_);
    lv_obj_set_size(back_btn_, 16, 14);
    lv_obj_set_style_bg_opa(back_btn_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(back_btn_, 0, 0);
    lv_obj_set_style_shadow_opa(back_btn_, LV_OPA_TRANSP, 0);
    lv_obj_t* back_label = lv_label_create(back_btn_);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(back_label, lv_color_white(), 0);
    lv_obj_center(back_label);
    lv_obj_add_flag(back_btn_, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_event_cb(back_btn_, on_back_btn_clicked, LV_EVENT_CLICKED, this);

    player_volume_ = lv_slider_create(control_col_);
    lv_obj_set_flex_grow(player_volume_, 1);
    lv_obj_set_height(player_volume_, 8);
    lv_slider_set_range(player_volume_, 0, 10);
    lv_obj_set_style_bg_color(player_volume_, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(player_volume_, LV_OPA_40, 0);
    lv_obj_set_style_bg_color(player_volume_, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(player_volume_, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(player_volume_, lv_color_white(), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(player_volume_, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_border_width(player_volume_, 0, 0);
    lv_obj_add_event_cb(player_volume_, on_player_volume_event, LV_EVENT_VALUE_CHANGED, this);
}

void AppMusic::updateStatus(const char* text) {
    if (status_) lv_label_set_text(status_, text);
}

void AppMusic::updateNowPlaying(const char* text) {
    if (now_playing_) {
        std::string s = std::string("Now Playing: ") + (text ? text : "-");
        lv_label_set_text(now_playing_, s.c_str());
    }
    if (track_name_) {
        std::string s = std::string("Track: ") + (text ? text : "-");
        lv_label_set_text(track_name_, s.c_str());
    }
}

void AppMusic::switchToPlayerView(const char* trackName) {
    if (list_) lv_obj_add_flag(list_, LV_OBJ_FLAG_HIDDEN);
    if (player_view_) lv_obj_clear_flag(player_view_, LV_OBJ_FLAG_HIDDEN);
    in_player_mode_ = true;
    if (track_name_) lv_label_set_text(track_name_, trackName ? trackName : "-");
    rebuildFocusGroup();
}

void AppMusic::switchToListView() {
    if (player_view_) lv_obj_add_flag(player_view_, LV_OBJ_FLAG_HIDDEN);
    if (list_) lv_obj_clear_flag(list_, LV_OBJ_FLAG_HIDDEN);
    in_player_mode_ = false;
    rebuildFocusGroup();
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
        apply_music_list_item_style(btn);
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
    lv_obj_t* back = lv_list_add_btn(list_, LV_SYMBOL_LEFT, "Back");
    ui_theme::apply_list_menu_item(back);
    apply_music_list_item_style(back);
    lv_obj_add_event_cb(back, on_album_item_clicked, LV_EVENT_CLICKED, this);
    auto it = category_.find(artist);
    if (it != category_.end()) {
        for (const auto& kv : it->second) {
            lv_obj_t* btn = lv_list_add_btn(list_, LV_SYMBOL_DIRECTORY, kv.first.c_str());
            ui_theme::apply_list_menu_item(btn);
            apply_music_list_item_style(btn);
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
    lv_obj_t* back = lv_list_add_btn(list_, LV_SYMBOL_LEFT, "Back");
    ui_theme::apply_list_menu_item(back);
    apply_music_list_item_style(back);
    lv_obj_add_event_cb(back, on_track_item_clicked, LV_EVENT_CLICKED, this);
    auto it = category_.find(artist);
    if (it != category_.end()) {
        auto it2 = it->second.find(album);
        if (it2 != it->second.end()) {
            for (int idx : it2->second) {
                lv_obj_t* btn = lv_list_add_btn(list_, LV_SYMBOL_AUDIO, names_[idx].c_str());
                ui_theme::apply_list_menu_item(btn);
                apply_music_list_item_style(btn);
                lv_obj_add_event_cb(btn, on_track_item_clicked, LV_EVENT_CLICKED, this);
            }
        }
    }
    rebuildFocusGroup();
}

static lv_obj_t* find_first_focusable_local(lv_obj_t* root) {
    if (!root) return nullptr;
    if (lv_obj_is_valid(root) && !lv_obj_has_flag(root, LV_OBJ_FLAG_HIDDEN)) {
        if (lv_obj_has_class(root, &lv_btn_class)) return root;
        if (lv_obj_has_class(root, &lv_textarea_class)) return root;
        if (lv_obj_has_class(root, &lv_dropdown_class)) return root;
        if (lv_obj_has_class(root, &lv_checkbox_class)) return root;
        if (lv_obj_has_class(root, &lv_slider_class)) return root;
        if (lv_obj_has_class(root, &lv_switch_class)) return root;
        if (lv_obj_has_class(root, &lv_spinbox_class)) return root;
        if (lv_obj_has_class(root, &lv_roller_class)) return root;
        if (lv_obj_has_class(root, &lv_img_class) && lv_obj_has_flag(root, LV_OBJ_FLAG_USER_1)) return root;
        if (lv_obj_is_editable(root)) return root;
    }
    uint32_t child_cnt = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(root, i);
        lv_obj_t* res = find_first_focusable_local(child);
        if (res) return res;
    }
    return nullptr;
}

void AppMusic::rebuildFocusGroup() {
    lv_group_t* grp = kb_get_current_group();
    if (!grp) return;
    lv_group_remove_all_objs(grp);
    lv_obj_t* root = in_player_mode_ ? player_view_ : list_;
    if (root) add_focusables_recursive(root, grp);
    lv_obj_t* focus = nullptr;
    if (!in_player_mode_) {
        if (list_) {
            uint32_t cnt = lv_obj_get_child_cnt(list_);
            for (uint32_t i = 0; i < cnt; ++i) {
                lv_obj_t* c = lv_obj_get_child(list_, i);
                if (is_focusable(c)) { focus = c; break; }
            }
        }
    } else {
        if (back_btn_ && is_focusable(back_btn_)) focus = back_btn_;
        if (!focus && player_volume_ && is_focusable(player_volume_)) focus = player_volume_;
    }
    if (!focus && root) focus = find_first_focusable_local(root);
    if (focus) lv_group_focus_obj(focus);
}

bool AppMusic::is_focusable(lv_obj_t* obj) {
    if (!lv_obj_is_valid(obj)) return false;
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) return false;
    if (lv_obj_has_class(obj, &lv_btn_class)) return true;
    if (lv_obj_has_class(obj, &lv_textarea_class)) return true;
    if (lv_obj_has_class(obj, &lv_dropdown_class)) return true;
    if (lv_obj_has_class(obj, &lv_checkbox_class)) return true;
    if (lv_obj_has_class(obj, &lv_slider_class)) return true;
    if (lv_obj_has_class(obj, &lv_switch_class)) return true;
    if (lv_obj_has_class(obj, &lv_spinbox_class)) return true;
    if (lv_obj_has_class(obj, &lv_roller_class)) return true;
    if (lv_obj_has_class(obj, &lv_img_class) && lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_1)) return true;
    if (lv_obj_is_editable(obj)) return true;
    return false;
}

void AppMusic::add_focusables_recursive(lv_obj_t* node, lv_group_t* group) {
    if (!node || !group) return;
    if (is_focusable(node)) lv_group_add_obj(group, node);
    uint32_t child_cnt = lv_obj_get_child_cnt(node);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(node, i);
        add_focusables_recursive(child, group);
    }
}

void AppMusic::on_back_btn_clicked(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    if (!app) return;
    app->switchToListView();
    if (app->list_level_ == LEVEL_TRACK) app->populateAlbumList(app->current_artist_);
    else app->populateArtistList();
}

void AppMusic::on_player_volume_event(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    if (!app) return;
    int v = lv_slider_get_value(app->player_volume_);
    app->sendAudioCommand(AUDIO_CMD_VOLUME, v);
}

void AppMusic::on_artist_item_clicked(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    if (!app) return;
    lv_obj_t* target = lv_event_get_target(e);
    const char* txt = lv_list_get_btn_text(app->list_, target);
    if (!txt) return;
    app->populateAlbumList(txt);
}

void AppMusic::on_album_item_clicked(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    if (!app) return;
    lv_obj_t* target = lv_event_get_target(e);
    const char* txt = lv_list_get_btn_text(app->list_, target);
    if (!txt) return;
    if (strcmp(txt, "Back") == 0) app->populateArtistList();
    else app->populateTrackList(app->current_artist_, txt);
}

void AppMusic::on_track_item_clicked(lv_event_t* e) {
    auto* app = static_cast<AppMusic*>(lv_event_get_user_data(e));
    if (!app) return;
    lv_obj_t* target = lv_event_get_target(e);
    const char* txt = lv_list_get_btn_text(app->list_, target);
    if (!txt) return;
    if (strcmp(txt, "Back") == 0) {
        app->populateAlbumList(app->current_artist_);
        return;
    }
    int idx = -1;
    for (int i = 0; i < (int)app->names_.size(); ++i) {
        if (app->names_[i] == txt) { idx = i; break; }
    }
    if (idx < 0) return;
    app->current_index_ = idx;
    app->updateNowPlaying(app->names_[idx].c_str());
    app->loadLyricsForPath(app->paths_[idx]);
    app->lyric_index_ = -1;
    app->lyric_pause_accum_ms_ = 0;
    app->lyric_pause_start_ms_ = 0;
    app->lyric_initialized_ = false;
    app->lyric_track_index_ = idx;
    app->updateLyrics(0);
    app->switchToPlayerView(app->names_[idx].c_str());
    app->sendAudioCommand(AUDIO_CMD_PLAY, 0, app->paths_[idx].c_str());
}

void AppMusic::updateUIFromAudioStatus() {
    if (!audioStatusMutex_) return;
    if (xSemaphoreTake(audioStatusMutex_, pdMS_TO_TICKS(1)) == pdTRUE) {
        if (audioStatus_.isPlaying && !audioStatus_.isPaused) {
            updateStatus("Playing");
            updateNowPlaying(audioStatus_.currentSongName);
            if (!lyric_initialized_) {
                lyric_start_ms_ = millis();
                lyric_pause_accum_ms_ = 0;
                lyric_pause_start_ms_ = 0;
                lyric_initialized_ = true;
            }
            uint32_t elapsed = lyric_start_ms_ ? millis() - lyric_start_ms_ - lyric_pause_accum_ms_ : 0;
            updateLyrics(elapsed);
        } else if (audioStatus_.isPaused) {
            updateNowPlaying("Paused");
            updateStatus("Paused");
            if (lyric_pause_start_ms_ == 0) lyric_pause_start_ms_ = millis();
        } else {
            updateNowPlaying("-");
        }
        if (audioStatus_.hasError && strlen(audioStatus_.errorMessage) > 0) {
            updateStatus(audioStatus_.errorMessage);
            if (!error_notified_) {
                ui_notify::showSymbol(LV_SYMBOL_WARNING, audioStatus_.errorMessage, 2500);
                error_notified_ = true;
            }
        } else {
            error_notified_ = false;
        }
        xSemaphoreGive(audioStatusMutex_);
    }
}
