#include "apps/music/app_music.h"
#include <SD.h>
#include <cstring>
#include "ui/ui_notify.h"

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

std::string AppMusic::stripExtension(const std::string& s) {
    size_t p = s.find_last_of('.');
    if (p == std::string::npos) return s;
    return s.substr(0, p);
}

bool AppMusic::parseNameParts(const std::string& base, std::string& artist, std::string& album, std::string& title) {
    std::string stem = stripExtension(base);
    size_t p1 = stem.find('-');
    if (p1 == std::string::npos) return false;
    size_t p2 = stem.find('-', p1 + 1);
    if (p2 == std::string::npos) return false;
    if (stem.find('-', p2 + 1) != std::string::npos) return false;
    artist = stem.substr(0, p1);
    album = stem.substr(p1 + 1, p2 - p1 - 1);
    title = stem.substr(p2 + 1);
    if (artist.empty() || album.empty() || title.empty()) return false;
    return true;
}

std::string AppMusic::extractTitle(const std::string& base) {
    std::string a, b, t;
    if (parseNameParts(base, a, b, t)) return t;
    return stripExtension(base);
}

void AppMusic::scanMusic() {
    paths_.clear();
    names_.clear();
    category_.clear();
    current_index_ = -1;
    list_level_ = LEVEL_ARTIST;
    if (list_) lv_obj_clean(list_);
    sd_scan_done_ = false;
    sd_scan_active_ = sd_.beginScan("/", ".mp3");
    if (sd_scan_active_) {
        updateStatus("Scanning Files");
        ui_notify::showSymbol(LV_SYMBOL_REFRESH, "Scanning Files", 1000);
    } else {
        updateStatus("Failed to scan files");
    }
}

void AppMusic::addDir(const char* dir) {
    File root = SD.open(dir);
    if (!root) return;
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            std::string name = file.name();
            if (ends_with_ci(name, ".mp3")) {
                std::string path;
                if (std::string(dir) == "/") path = std::string("/") + name;
                else path = std::string(dir) + "/" + name;
                paths_.push_back(path);
                names_.push_back(basename(path));
                std::string base = names_.back();
                std::string artist, album, title;
                if (!parseNameParts(base, artist, album, title)) {
                    artist = "Uncategorized";
                    album = "Uncategorized";
                }
                category_[artist][album].push_back((int)paths_.size() - 1);
            }
        }
        file = root.openNextFile();
    }
    root.close();
}

void AppMusic::handleScanStep() {
    if (!sd_scan_active_) return;
    const int kMax = 8;
    FileInfo tmp[kMax];
    int cnt = 0;
    bool cont = sd_.stepScan(tmp, cnt, kMax);
    for (int i = 0; i < cnt; ++i) {
        const FileInfo& f = tmp[i];
        std::string path = f.path.c_str();
        paths_.push_back(path);
        names_.push_back(basename(path));
        std::string base = names_.back();
        std::string artist;
        std::string album;
        std::string title;
        if (!parseNameParts(base, artist, album, title)) {
            artist = "Uncategorized";
            album = "Uncategorized";
        }
        category_[artist][album].push_back((int)paths_.size() - 1);
    }
    if (!cont) {
        sd_scan_active_ = false;
        sd_scan_done_ = true;
        if (paths_.empty()) {
            updateStatus("No MP3 files found");
        } else {
            updateStatus("Found MP3 files");
            populateArtistList();
        }
    }
}
