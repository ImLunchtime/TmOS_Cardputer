#include "apps/music_downloader/app_music_downloader.h"
#include "theme.h"
#include "ui_notify.h"
#include "input_kb.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

AppMusicDownloader::AppMusicDownloader() {}
AppMusicDownloader::~AppMusicDownloader() {}

static void on_search_event(lv_event_t* e) {
    AppMusicDownloader* app = (AppMusicDownloader*)lv_event_get_user_data(e);
    if (app) AppMusicDownloader::on_search(e);
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

    ta_keyword_ = lv_textarea_create(root_);
    lv_obj_set_width(ta_keyword_, LV_PCT(100));
    lv_textarea_set_one_line(ta_keyword_, true);
    lv_textarea_set_placeholder_text(ta_keyword_, "Keyword");
    lv_obj_set_style_text_font(ta_keyword_, ui_theme::get_system_font(), 0);

    list_ = lv_list_create(root_);
    lv_obj_set_size(list_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(list_, 1);
    ui_theme::apply_list_menu(list_);

    btn_search_ = lv_btn_create(root_);
    lv_obj_add_flag(btn_search_, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(btn_search_, LV_ALIGN_BOTTOM_RIGHT, -4, -4);
    lv_obj_set_size(btn_search_, 32, 24);
    lv_obj_set_style_radius(btn_search_, LV_RADIUS_CIRCLE, 0);
    ui_theme::apply_button(btn_search_);
    {
        lv_obj_t* l = lv_label_create(btn_search_);
        lv_label_set_text(l, "Search");
        lv_obj_center(l);
    }
    lv_obj_add_event_cb(btn_search_, on_search_event, LV_EVENT_CLICKED, this);

    rebuildFocusGroup();
}

void AppMusicDownloader::onTick() {
}

void AppMusicDownloader::onClose() {
    kb_clear_app_keys(this);
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
}

void AppMusicDownloader::update_results_list() {
    if (!list_) return;
    lv_obj_clean(list_);
    ui_theme::apply_list_menu(list_);
    if (results_.empty()) {
        lv_list_add_text(list_, "No songs");
    } else {
        for (size_t i = 0; i < results_.size(); ++i) {
            String line = results_[i].title;
            if (results_[i].artist.length()) {
                line += " - ";
                line += results_[i].artist;
            }
            lv_obj_t* btn = lv_list_add_btn(list_, LV_SYMBOL_AUDIO, line.c_str());
            ui_theme::apply_list_menu_item(btn);
        }
    }
    rebuildFocusGroup();
}

void AppMusicDownloader::rebuildFocusGroup() {
    lv_group_t* grp = kb_get_current_group();
    if (!grp) return;
    lv_group_remove_all_objs(grp);
    if (ta_keyword_) lv_group_add_obj(grp, ta_keyword_);
    if (btn_search_) lv_group_add_obj(grp, btn_search_);
    if (list_) add_focusables_recursive(list_, grp);
    if (ta_keyword_) lv_group_focus_obj(ta_keyword_);
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
