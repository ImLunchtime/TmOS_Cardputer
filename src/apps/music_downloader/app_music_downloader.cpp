#include "apps/music_downloader/app_music_downloader.h"
#include "ui/theme.h"
#include "ui/ui_notify.h"
#include "drivers/input_kb.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <SD.h>
#include "services/dns_resolver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef LV_SYMBOL_SEARCH
#define LV_SYMBOL_SEARCH LV_SYMBOL_REFRESH
#endif

AppMusicDownloader::AppMusicDownloader() {}
AppMusicDownloader::~AppMusicDownloader() {}

static void on_search_event(lv_event_t* e) {
    AppMusicDownloader* app = (AppMusicDownloader*)lv_event_get_user_data(e);
    if (app) AppMusicDownloader::on_search(e);
}

static void on_back_event(lv_event_t* e) {
    AppMusicDownloader* app = (AppMusicDownloader*)lv_event_get_user_data(e);
    if (app) AppMusicDownloader::on_back(e);
}

void AppMusicDownloader::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 6, 0);
    lv_obj_set_style_pad_row(root_, 4, 0);
    ui_theme::apply_small_text_recursive(root_);

    label_status_ = lv_label_create(root_);
    lv_obj_set_style_text_font(label_status_, ui_theme::get_system_font(), 0);
    lv_label_set_text(label_status_, "Enter keyword and press Search");

    page_search_ = lv_obj_create(root_);
    lv_obj_set_size(page_search_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(page_search_, 1);
    lv_obj_set_style_bg_opa(page_search_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page_search_, 0, 0);
    lv_obj_set_style_pad_all(page_search_, 0, 0);
    lv_obj_set_style_pad_row(page_search_, 4, 0);
    lv_obj_set_flex_flow(page_search_, LV_FLEX_FLOW_COLUMN);

    lv_obj_t* search_row = lv_obj_create(page_search_);
    lv_obj_set_size(search_row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(search_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(search_row, 0, 0);
    lv_obj_set_style_pad_all(search_row, 0, 0);
    lv_obj_set_style_pad_row(search_row, 0, 0);
    lv_obj_set_style_pad_column(search_row, 4, 0);
    lv_obj_set_flex_flow(search_row, LV_FLEX_FLOW_ROW);

    ta_keyword_ = lv_textarea_create(search_row);
    lv_obj_set_flex_grow(ta_keyword_, 1);
    lv_obj_set_height(ta_keyword_, 20);
    lv_textarea_set_one_line(ta_keyword_, true);
    lv_textarea_set_placeholder_text(ta_keyword_, "Keyword");
    lv_obj_set_style_text_font(ta_keyword_, ui_theme::get_system_font(), 0);

    btn_search_ = lv_btn_create(search_row);
    lv_obj_set_size(btn_search_, 20, 20);
    lv_obj_set_style_radius(btn_search_, LV_RADIUS_CIRCLE, 0);
    ui_theme::apply_button(btn_search_);
    {
        lv_obj_t* l = lv_label_create(btn_search_);
        lv_label_set_text(l, LV_SYMBOL_SEARCH);
        lv_obj_center(l);
    }
    lv_obj_add_event_cb(btn_search_, on_search_event, LV_EVENT_CLICKED, this);

    page_results_ = lv_obj_create(root_);
    lv_obj_set_size(page_results_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(page_results_, 1);
    lv_obj_set_style_bg_opa(page_results_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page_results_, 0, 0);
    lv_obj_set_style_pad_all(page_results_, 0, 0);
    lv_obj_set_style_pad_row(page_results_, 4, 0);
    lv_obj_set_flex_flow(page_results_, LV_FLEX_FLOW_COLUMN);

    btn_back_ = lv_btn_create(page_results_);
    lv_obj_set_size(btn_back_, 20, 20);
    lv_obj_set_style_radius(btn_back_, LV_RADIUS_CIRCLE, 0);
    ui_theme::apply_button(btn_back_);
    lv_obj_add_flag(btn_back_, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(btn_back_, LV_ALIGN_TOP_LEFT, 2, 2);
    {
        lv_obj_t* l = lv_label_create(btn_back_);
        lv_label_set_text(l, LV_SYMBOL_LEFT);
        lv_obj_center(l);
    }
    lv_obj_add_event_cb(btn_back_, on_back_event, LV_EVENT_CLICKED, this);

    list_ = lv_list_create(page_results_);
    lv_obj_set_size(list_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(list_, 1);
    ui_theme::apply_list_menu(list_);

    page_download_ = lv_obj_create(root_);
    lv_obj_set_size(page_download_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(page_download_, 1);
    lv_obj_set_style_bg_opa(page_download_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page_download_, 0, 0);
    lv_obj_set_style_pad_all(page_download_, 6, 0);
    lv_obj_set_style_pad_row(page_download_, 4, 0);
    lv_obj_set_flex_flow(page_download_, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(page_download_, LV_OBJ_FLAG_HIDDEN);

    label_progress_ = lv_label_create(page_download_);
    lv_label_set_text(label_progress_, "Downloading: -");
    lv_obj_set_style_text_font(label_progress_, ui_theme::get_system_font(), 0);

    bar_ = lv_bar_create(page_download_);
    lv_obj_set_width(bar_, LV_PCT(100));
    lv_obj_set_height(bar_, 12);
    lv_bar_set_range(bar_, 0, 100);
    lv_bar_set_value(bar_, 0, LV_ANIM_OFF);

    btn_cancel_ = lv_btn_create(page_download_);
    lv_obj_set_size(btn_cancel_, 20, 20);
    lv_obj_set_style_radius(btn_cancel_, LV_RADIUS_CIRCLE, 0);
    ui_theme::apply_button(btn_cancel_);
    {
        lv_obj_t* l = lv_label_create(btn_cancel_);
        lv_label_set_text(l, LV_SYMBOL_CLOSE);
        lv_obj_center(l);
    }
    lv_obj_add_event_cb(btn_cancel_, on_cancel_download, LV_EVENT_CLICKED, this);

    lv_obj_add_flag(page_results_, LV_OBJ_FLAG_HIDDEN);
    in_results_ = false;

    rebuildFocusGroup();
}

void AppMusicDownloader::onTick() {
}

void AppMusicDownloader::onClose() {
    kb_clear_app_keys(this);
    dl_cancel_ = true;
    downloading_ = false;
    if (timer_download_) { lv_timer_del(timer_download_); timer_download_ = nullptr; }
}

static String url_encode(const char* s) {
    if (!s) return String();
    String out;
    const char* hex = "0123456789ABCDEF";
    while (*s) {
        unsigned char c = (unsigned char)*s++;
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            out += char(c);
        } else if (c == ' ') {
            out += "%20";
        } else {
            out += '%';
            out += hex[c >> 4];
            out += hex[c & 0x0F];
        }
    }
    return out;
}

void AppMusicDownloader::perform_search() {
    if (!wifi_manager::is_connected()) {
        lv_label_set_text(label_status_, "WiFi not connected");
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "WiFi not connected", 2000);
        return;
    }
    const char* kw = lv_textarea_get_text(ta_keyword_);
    if (!kw || !*kw) {
        lv_label_set_text(label_status_, "Please enter keyword");
        return;
    }
    lv_label_set_text(label_status_, "Connecting HTTPS...");
    WiFiClientSecure client;
    client.setInsecure();
    const char* host = "apis.netstart.cn";
    const uint16_t port = 443;
    if (!client.connect(host, port)) {
        lv_label_set_text(label_status_, "HTTPS connect failed");
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "HTTPS connect failed", 2000);
        return;
    }
    String encoded = url_encode(kw);
    String path = "/music/search?keywords=";
    path += encoded;
    path += "&type=1&limit=5";
    String req = "GET ";
    req += path;
    req += " HTTP/1.1\r\nHost: apis.netstart.cn\r\nConnection: close\r\nAccept: application/json\r\nUser-Agent: CardputerOS2/1.0\r\n\r\n";
    client.print(req);
    unsigned long start = millis();
    String response;
    while ((client.connected() || client.available()) && millis() - start < 8000) {
        while (client.available()) {
            char c = (char)client.read();
            response += c;
            start = millis();
        }
        delay(10);
    }
    client.stop();
    int header_end = response.indexOf("\r\n\r\n");
    if (header_end < 0) {
        lv_label_set_text(label_status_, "Invalid HTTP response");
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "HTTP error", 2000);
        return;
    }
    String header = response.substring(0, header_end);
    String body = response.substring(header_end + 4);
    int line_end = header.indexOf("\r\n");
    String status_line = line_end > 0 ? header.substring(0, line_end) : header;
    int sp1 = status_line.indexOf(' ');
    int sp2 = status_line.indexOf(' ', sp1 + 1);
    int status_code = 0;
    if (sp1 > 0 && sp2 > sp1) {
        status_code = status_line.substring(sp1 + 1, sp2).toInt();
    }
    if (status_code != 200) {
        String msg = "HTTP ";
        msg += status_code;
        if (status_code >= 300 && status_code < 400) {
            msg += " redirect";
            int locPos = header.indexOf("Location:");
            if (locPos >= 0) {
                int lineEndLoc = header.indexOf("\r\n", locPos);
                String locLine = lineEndLoc > locPos ? header.substring(locPos, lineEndLoc) : header.substring(locPos);
                msg += " ";
                msg += locLine;
            }
        }
        lv_label_set_text(label_status_, msg.c_str());
        ui_notify::showSymbol(LV_SYMBOL_WARNING, msg.c_str(), 3000);
        return;
    }
    int jsonStart = -1;
    for (int i = 0; i < body.length(); ++i) {
        char c = body[i];
        if (c == '{' || c == '[') {
            jsonStart = i;
            break;
        }
    }
    String jsonText = jsonStart >= 0 ? body.substring(jsonStart) : body;

    DynamicJsonDocument doc(8192);
    DeserializationError err = deserializeJson(doc, jsonText);
    if (err) {
        String jmsg = String("JSON failed: ") + err.c_str();
        lv_label_set_text(label_status_, jmsg.c_str());
        ui_notify::showSymbol(LV_SYMBOL_WARNING, jmsg.c_str(), 2500);
        return;
    }
    results_.clear();
    JsonVariant result = doc["result"];
    if (!result.is<JsonObject>()) {
        lv_label_set_text(label_status_, "No result");
        update_results_list();
        return;
    }
    JsonArray songs = result["songs"].as<JsonArray>();
    for (JsonVariant v : songs) {
        JsonObject song = v.as<JsonObject>();
        if (song.isNull()) continue;
        SongItem item;
        const char* name = song["name"] | "";
        item.title = String(name);
        item.id = song["id"] | 0;
        int fee = song["fee"] | 0;
        item.vip = (fee != 0);
        JsonArray artists = song["artists"].as<JsonArray>();
        if (!artists.isNull() && artists.size() > 0) {
            const char* aname = artists[0]["name"] | "";
            item.artist = String(aname);
        } else {
            item.artist = String();
        }
        results_.push_back(item);
    }
    update_results_list();
    char buf[32];
    snprintf(buf, sizeof(buf), "Found %u result(s)", (unsigned)results_.size());
    lv_label_set_text(label_status_, buf);
    switch_to_results_page();
}

void AppMusicDownloader::update_results_list() {
    if (!list_) return;
    lv_obj_clean(list_);
    ui_theme::apply_list_menu(list_);
    if (results_.empty()) {
        lv_list_add_text(list_, "No songs");
    } else {
        for (size_t i = 0; i < results_.size(); ++i) {
            String line;
            if (results_[i].vip) {
                line += "[VIP] ";
            }
            line += results_[i].title;
            if (results_[i].artist.length()) {
                line += " - ";
                line += results_[i].artist;
            }
            lv_obj_t* btn = lv_list_add_btn(list_, LV_SYMBOL_AUDIO, line.c_str());
            ui_theme::apply_list_menu_item(btn);
            lv_obj_add_event_cb(btn, on_result_item_clicked, LV_EVENT_CLICKED, this);
        }
    }
}

void AppMusicDownloader::rebuildFocusGroup() {
    lv_group_t* grp = kb_get_current_group();
    if (!grp) return;
    lv_group_remove_all_objs(grp);
    bool download_visible = page_download_ && !lv_obj_has_flag(page_download_, LV_OBJ_FLAG_HIDDEN);
    bool results_visible = page_results_ && !lv_obj_has_flag(page_results_, LV_OBJ_FLAG_HIDDEN);
    if (!results_visible && !download_visible) {
        if (ta_keyword_) lv_group_add_obj(grp, ta_keyword_);
        if (btn_search_) lv_group_add_obj(grp, btn_search_);
        if (ta_keyword_) lv_group_focus_obj(ta_keyword_);
    } else if (results_visible) {
        if (btn_back_) lv_group_add_obj(grp, btn_back_);
        if (list_) add_focusables_recursive(list_, grp);
        if (btn_back_) lv_group_focus_obj(btn_back_);
    } else if (download_visible) {
        if (btn_cancel_) lv_group_add_obj(grp, btn_cancel_);
        if (btn_cancel_) lv_group_focus_obj(btn_cancel_);
    }
}

void AppMusicDownloader::add_focusables_recursive(lv_obj_t* node, lv_group_t* group) {
    if (!node || !group) return;
    if (lv_obj_has_class(node, &lv_btn_class) ||
        lv_obj_has_class(node, &lv_textarea_class) ||
        lv_obj_has_class(node, &lv_dropdown_class) ||
        lv_obj_has_class(node, &lv_checkbox_class) ||
        lv_obj_has_class(node, &lv_slider_class) ||
        lv_obj_has_class(node, &lv_switch_class) ||
        lv_obj_has_class(node, &lv_spinbox_class) ||
        lv_obj_has_class(node, &lv_roller_class) ||
        (lv_obj_has_class(node, &lv_img_class) && lv_obj_has_flag(node, LV_OBJ_FLAG_USER_1)) ||
        lv_obj_is_editable(node)) {
        lv_group_add_obj(group, node);
    }
    uint32_t child_cnt = lv_obj_get_child_cnt(node);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(node, i);
        add_focusables_recursive(child, group);
    }
}

void AppMusicDownloader::on_search(lv_event_t* e) {
    AppMusicDownloader* app = (AppMusicDownloader*)lv_event_get_user_data(e);
    if (!app) return;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    app->perform_search();
}

void AppMusicDownloader::switch_to_search_page() {
    in_results_ = false;
    if (page_results_) lv_obj_add_flag(page_results_, LV_OBJ_FLAG_HIDDEN);
    if (page_search_) lv_obj_clear_flag(page_search_, LV_OBJ_FLAG_HIDDEN);
    if (label_status_) lv_obj_clear_flag(label_status_, LV_OBJ_FLAG_HIDDEN);
    if (page_download_) lv_obj_add_flag(page_download_, LV_OBJ_FLAG_HIDDEN);
    rebuildFocusGroup();
}

void AppMusicDownloader::switch_to_results_page() {
    in_results_ = true;
    if (page_search_) lv_obj_add_flag(page_search_, LV_OBJ_FLAG_HIDDEN);
    if (page_results_) lv_obj_clear_flag(page_results_, LV_OBJ_FLAG_HIDDEN);
    if (label_status_) lv_obj_add_flag(label_status_, LV_OBJ_FLAG_HIDDEN);
    if (page_download_) lv_obj_add_flag(page_download_, LV_OBJ_FLAG_HIDDEN);
    rebuildFocusGroup();
}

void AppMusicDownloader::switch_to_download_page() {
    in_results_ = true;
    if (page_search_) lv_obj_add_flag(page_search_, LV_OBJ_FLAG_HIDDEN);
    if (page_results_) lv_obj_add_flag(page_results_, LV_OBJ_FLAG_HIDDEN);
    if (page_download_) lv_obj_clear_flag(page_download_, LV_OBJ_FLAG_HIDDEN);
    if (label_status_) lv_obj_add_flag(label_status_, LV_OBJ_FLAG_HIDDEN);
    lv_bar_set_value(bar_, 0, LV_ANIM_OFF);
    downloading_ = true;
    dl_total_ = -1;
    dl_written_ = 0;
    dl_done_ = false;
    dl_ok_ = false;
    dl_cancel_ = false;
    rebuildFocusGroup();
}

void AppMusicDownloader::on_back(lv_event_t* e) {
    AppMusicDownloader* app = (AppMusicDownloader*)lv_event_get_user_data(e);
    if (!app) return;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    app->switch_to_search_page();
}

void AppMusicDownloader::on_result_item_clicked(lv_event_t* e) {
    AppMusicDownloader* app = (AppMusicDownloader*)lv_event_get_user_data(e);
    if (!app) return;
    lv_obj_t* target = lv_event_get_target(e);
    const char* txt = lv_list_get_btn_text(app->list_, target);
    if (!txt) return;
    int idx = -1;
    for (int i = 0; i < (int)app->results_.size(); ++i) {
        String line;
        if (app->results_[i].vip) line += "[VIP] ";
        line += app->results_[i].title;
        if (app->results_[i].artist.length()) { line += " - "; line += app->results_[i].artist; }
        if (line == txt) { idx = i; break; }
    }
    if (idx < 0) return;
    app->start_download_by_index(idx);
}

void AppMusicDownloader::on_cancel_download(lv_event_t* e) {
    AppMusicDownloader* app = (AppMusicDownloader*)lv_event_get_user_data(e);
    if (!app) return;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    app->dl_cancel_ = true;
    app->downloading_ = false;
}

String AppMusicDownloader::sanitize_token(const String& s) {
    String out;
    for (size_t i = 0; i < s.length(); ++i) {
        char c = s[i];
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|' ) c = '_';
        if (c == '\r' || c == '\n' || (unsigned char)c < 32) continue;
        out += c;
    }
    out.trim();
    return out;
}

String AppMusicDownloader::build_filename_for_index(int idx) {
    if (idx < 0 || idx >= (int)results_.size()) return String("/download.mp3");
    String artist = sanitize_token(results_[idx].artist);
    String album = "Single";
    String title = sanitize_token(results_[idx].title);
    if (artist.length() == 0) artist = "Unknown";
    if (title.length() == 0) title = "Unknown";
    String name = "/";
    name += artist;
    name += "-";
    name += album;
    name += "-";
    name += title;
    name += ".mp3";
    return name;
}

void AppMusicDownloader::start_download_by_index(int idx) {
    if (idx < 0 || idx >= (int)results_.size()) return;
    if (!wifi_manager::is_connected()) {
        ui_notify::showSymbol(LV_SYMBOL_WARNING, "WiFi not connected", 2000);
        return;
    }
    dl_save_path_ = build_filename_for_index(idx);
    dl_url_ = "https://api.qijieya.cn/meting/?type=url&id=" + String(results_[idx].id);
    String label = "Downloading: ";
    label += results_[idx].title;
    if (results_[idx].artist.length()) { label += " - "; label += results_[idx].artist; }
    lv_label_set_text(label_progress_, label.c_str());
    switch_to_download_page();
    if (timer_download_) { lv_timer_del(timer_download_); timer_download_ = nullptr; }
    timer_download_ = lv_timer_create(AppMusicDownloader::on_download_timer, 100, this);
    xTaskCreate(AppMusicDownloader::downloadTaskThunk, "music_dl", 8192, this, 1, &downloadTaskHandle_);
}

static bool parse_url(const char* url, String& scheme, String& host, uint16_t& port, String& path) {
    scheme.clear(); host.clear(); path.clear(); port = 0;
    String s(url);
    int p = s.indexOf("://");
    if (p < 0) return false;
    scheme = s.substring(0, p);
    int hstart = p + 3;
    int pslash = s.indexOf('/', hstart);
    String hostport = pslash >= 0 ? s.substring(hstart, pslash) : s.substring(hstart);
    int colon = hostport.indexOf(':');
    if (colon >= 0) {
        host = hostport.substring(0, colon);
        port = hostport.substring(colon + 1).toInt();
    } else {
        host = hostport;
        port = (scheme == "https") ? 443 : 80;
    }
    host.trim();
    while (host.endsWith(".")) host.remove(host.length() - 1);
    path = pslash >= 0 ? s.substring(pslash) : "/";
    return host.length() > 0;
}

bool AppMusicDownloader::download_to_file(const char* url, const char* save_path) {
    String scheme, host, path;
    uint16_t port = 0;
    if (!parse_url(url, scheme, host, port, path)) { dl_error_ = "Invalid URL"; return false; }
    WiFiClientSecure https;
    WiFiClient http;
    bool use_tls = (scheme == "https");
    if (use_tls) https.setInsecure();
    IPAddress ip;
    bool has_ip = dns_resolver::resolve(host.c_str(), ip);
    bool conn;
    if (use_tls) {
        conn = https.connect(host.c_str(), port);
    } else {
        conn = has_ip ? http.connect(ip, port) : http.connect(host.c_str(), port);
    }
    if (!conn) { dl_error_ = String("Connect failed: ") + host; return false; }
    String req = "GET ";
    req += path;
    req += " HTTP/1.1\r\nHost: ";
    req += host;
    req += "\r\nConnection: close\r\nUser-Agent: CardputerOS2/1.0\r\nAccept: */*\r\nRange: bytes=0-\r\n\r\n";
    if (use_tls) https.print(req); else http.print(req);
    String header;
    unsigned long start = millis();
    bool header_done = false;
    while (millis() - start < 10000 && !header_done) {
        int avail = use_tls ? https.available() : http.available();
        if (avail <= 0) {
            if (use_tls ? !https.connected() : !http.connected()) break;
            delay(5);
            continue;
        }
        char c = use_tls ? (char)https.read() : (char)http.read();
        header += c;
        if (header.endsWith("\r\n\r\n")) {
            header_done = true;
            break;
        }
        start = millis();
    }
    if (!header_done) {
        if (use_tls) https.stop(); else http.stop();
        dl_error_ = "Invalid HTTP response";
        return false;
    }
    int lineEnd = header.indexOf("\r\n");
    String status = lineEnd > 0 ? header.substring(0, lineEnd) : header;
    int sp1 = status.indexOf(' ');
    int sp2 = status.indexOf(' ', sp1 + 1);
    int code = 0;
    if (sp1 > 0 && sp2 > sp1) code = status.substring(sp1 + 1, sp2).toInt();
    if (code >= 300 && code < 400) {
        int locPos = header.indexOf("Location:");
        if (locPos < 0) {
            if (use_tls) https.stop(); else http.stop();
            dl_error_ = "Redirect without Location";
            return false;
        }
        int locEnd = header.indexOf("\r\n", locPos);
        String locLine = locEnd > locPos ? header.substring(locPos, locEnd) : header.substring(locPos);
        int colon = locLine.indexOf(':');
        String loc = colon >= 0 ? locLine.substring(colon + 1) : String();
        loc.trim();
        if (use_tls) https.stop(); else http.stop();
        if (loc.startsWith("http")) {
            return download_to_file(loc.c_str(), save_path);
        } else {
            String next = scheme + "://" + host + loc;
            return download_to_file(next.c_str(), save_path);
        }
    } else if (!(code == 200 || code == 206)) {
        dl_error_ = String("HTTP ") + code;
        return false;
    }
    int clPos = header.indexOf("Content-Length:");
    int clEnd = clPos >= 0 ? header.indexOf("\r\n", clPos) : -1;
    long total = -1;
    if (clPos >= 0 && clEnd > clPos) {
        String clLine = header.substring(clPos, clEnd);
        int sep = clLine.indexOf(':');
        if (sep >= 0) total = clLine.substring(sep + 1).toInt();
    }
    int ctPos = header.indexOf("Content-Type:");
    String contentType;
    if (ctPos >= 0) {
        int ctEnd = header.indexOf("\r\n", ctPos);
        String ctLine = ctEnd > ctPos ? header.substring(ctPos, ctEnd) : header.substring(ctPos);
        int sep = ctLine.indexOf(':');
        if (sep >= 0) contentType = ctLine.substring(sep + 1);
        contentType.trim();
    }
    if (host == "api.qijieya.cn" || contentType.startsWith("text/plain")) {
        String bodyUrl;
        unsigned long t0 = millis();
        while (millis() - t0 < 8000) {
            int avail = use_tls ? https.available() : http.available();
            if (avail <= 0) {
                if (use_tls ? !https.connected() : !http.connected()) break;
                delay(5);
                continue;
            }
            char buf[128];
            int n = avail;
            if (n > (int)sizeof(buf)) n = sizeof(buf);
            int r = use_tls ? https.read((uint8_t*)buf, n) : http.read((uint8_t*)buf, n);
            if (r > 0) {
                bodyUrl += String(buf, r);
                t0 = millis();
            } else {
                delay(5);
            }
        }
        if (use_tls) https.stop(); else http.stop();
        bodyUrl.trim();
        if (bodyUrl.startsWith("http")) {
            return download_to_file(bodyUrl.c_str(), save_path);
        } else {
            dl_error_ = "Invalid URL from resolver";
            return false;
        }
    }
    dl_total_ = total;
    File file = SD.open(save_path, FILE_WRITE);
    if (!file) {
        if (use_tls) https.stop(); else http.stop();
        dl_error_ = "SD open failed";
        return false;
    }
    long written = 0;
    uint8_t buf[1024];
    unsigned long last = millis();
    for (;;) {
        if (dl_cancel_) {
            file.close();
            SD.remove(save_path);
            if (use_tls) https.stop(); else http.stop();
            dl_error_ = "Canceled";
            return false;
        }
        int avail = use_tls ? https.available() : http.available();
        if (avail <= 0) {
            if (use_tls ? !https.connected() : !http.connected()) break;
            if (millis() - last > 10000) break;
            delay(5);
            continue;
        }
        int n = avail;
        if (n > (int)sizeof(buf)) n = sizeof(buf);
        int r = use_tls ? https.read(buf, n) : http.read(buf, n);
        if (r <= 0) {
            delay(1);
            continue;
        }
        size_t w = file.write(buf, r);
        if (w != (size_t)r) {
            file.close();
            SD.remove(save_path);
            if (use_tls) https.stop(); else http.stop();
            dl_error_ = "Write failed";
            return false;
        }
        written += r;
        dl_written_ = written;
        last = millis();
        delay(1);
    }
    file.close();
    if (use_tls) https.stop(); else http.stop();
    if (total > 0 && written < total) {
        dl_error_ = "Incomplete bytes";
        return false;
    }
    return written > 0;
}

void AppMusicDownloader::downloadTaskThunk(void* parameter) {
    AppMusicDownloader* app = (AppMusicDownloader*)parameter;
    if (!app) { vTaskDelete(nullptr); return; }
    app->downloadTaskLoop();
    vTaskDelete(nullptr);
}

void AppMusicDownloader::downloadTaskLoop() {
    bool ok = download_to_file(dl_url_.c_str(), dl_save_path_.c_str());
    dl_ok_ = ok;
    dl_done_ = true;
}

void AppMusicDownloader::on_download_timer(lv_timer_t* t) {
    auto* app = (AppMusicDownloader*)t->user_data;
    if (!app) return;
    if (app->dl_total_ > 0) {
        lv_bar_set_range(app->bar_, 0, (int)app->dl_total_);
        lv_bar_set_value(app->bar_, (int)app->dl_written_, LV_ANIM_OFF);
        char msg[64];
        snprintf(msg, sizeof(msg), "Downloading: %ld/%ld bytes", app->dl_written_, app->dl_total_);
        lv_label_set_text(app->label_progress_, msg);
    } else {
        lv_bar_set_range(app->bar_, 0, 100);
        int pct = app->dl_written_ > 0 ? 50 : 0;
        lv_bar_set_value(app->bar_, pct, LV_ANIM_OFF);
        char msg[64];
        snprintf(msg, sizeof(msg), "Downloading: %ld bytes", app->dl_written_);
        lv_label_set_text(app->label_progress_, msg);
    }
    if (app->dl_done_) {
        if (app->timer_download_) { lv_timer_del(app->timer_download_); app->timer_download_ = nullptr; }
        if (app->dl_ok_) ui_notify::showSymbol(LV_SYMBOL_OK, "Download complete", 1500);
        else ui_notify::showSymbol(LV_SYMBOL_WARNING, app->dl_error_.length() ? app->dl_error_.c_str() : "Download failed", 2500);
        app->downloading_ = false;
        app->switch_to_results_page();
    }
}
