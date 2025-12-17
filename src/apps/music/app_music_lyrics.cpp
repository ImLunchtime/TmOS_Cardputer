#include "apps/music/app_music.h"
#include <SD.h>
#include <algorithm>

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
    if (lyrics_label_) lv_label_set_text(lyrics_label_, "");
    if (lyrics_container_) lv_obj_scroll_to_y(lyrics_container_, 0, LV_ANIM_OFF);
}

void AppMusic::loadLyricsForPath(const std::string& mp3_path) {
    clearLyrics();
    std::string lrc = replaceExtension(mp3_path, ".lrc");
    File f = SD.open(lrc.c_str());
    if (!f) {
        if (lyrics_label_) lv_label_set_text(lyrics_label_, "No lyrics file");
        return;
    }
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
    if (lyrics_.empty()) {
        if (lyrics_label_) lv_label_set_text(lyrics_label_, "No lyrics file");
        return;
    }
    std::string display;
    display.reserve(lyrics_.size() * 24);
    for (size_t i = 0; i < lyrics_.size(); ++i) {
        if (i) display.push_back('\n');
        display += lyrics_[i].s;
    }
    if (lyrics_label_) lv_label_set_text(lyrics_label_, display.c_str());
    if (lyrics_container_) lv_obj_scroll_to_y(lyrics_container_, 0, LV_ANIM_OFF);
}

void AppMusic::updateLyrics(uint32_t elapsed_ms) {
    (void)elapsed_ms;
}
