/*
 * @Author: ImLunchtime knoxmedia@yeah.net
 * @Date: 2025-11-07 17:33:17
 * @LastEditors: ImLunchtime knoxmedia@yeah.net
 * @LastEditTime: 2025-12-05 10:34:02
 * @FilePath: \CardputerOS2_LVGL\src\input_kb.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <M5Cardputer.h>
#include <lvgl.h>
#include <vector>
#include <unordered_map>
#include "input_kb.h"

static lv_indev_t *kb_indev = nullptr;
static lv_group_t *kb_group = nullptr;
static std::vector<uint16_t> key_queue;
static uint16_t current_key = 0;
static bool emit_release_next = false;
static volatile bool g_exit_requested = false;

static IApp* g_active_app = nullptr;
struct RegKey { uint16_t key; std::function<bool()> is_active; std::function<void()> on_press; };
static std::unordered_map<IApp*, std::vector<RegKey>> g_app_keys;

static inline bool try_handle_custom(uint16_t key) {
    if (!g_active_app) return false;
    auto it = g_app_keys.find(g_active_app);
    if (it == g_app_keys.end()) return false;
    const auto& ks = it->second;
    for (const auto& k : ks) {
        if (k.key == key) {
            if (!k.is_active || k.is_active()) {
                if (k.on_press) k.on_press();
                return true;
            }
        }
    }
    return false;
}

static void keyboard_read(lv_indev_drv_t * drv, lv_indev_data_t * data) {
    (void)drv;

    if (emit_release_next) {
        data->key = current_key;
        data->state = LV_INDEV_STATE_RELEASED;
        emit_release_next = false;
        return;
    }

    if (!key_queue.empty()) {
        current_key = key_queue.front();
        key_queue.erase(key_queue.begin());
        data->key = current_key;
        data->state = LV_INDEV_STATE_PRESSED;
        emit_release_next = true;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void kb_init() {
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = keyboard_read;
    kb_indev = lv_indev_drv_register(&indev_drv);

    kb_group = lv_group_create();
    lv_indev_set_group(kb_indev, kb_group);
}

lv_indev_t* kb_get_indev() { return kb_indev; }
lv_group_t* kb_get_group() { return kb_group; }

void kb_set_active_app(IApp* app) { g_active_app = app; }
void kb_register_app_keys(IApp* owner, const std::vector<AppCustomKey>& keys) {
    std::vector<RegKey> v; v.reserve(keys.size());
    for (const auto& k : keys) v.push_back({k.key, k.is_active, k.on_press});
    g_app_keys[owner] = std::move(v);
}
void kb_clear_app_keys(IApp* owner) {
    g_app_keys.erase(owner);
}

void kb_process_hardware_keys() {
    M5Cardputer.update();

    if (M5Cardputer.Keyboard.isChange()) {
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
        if (M5Cardputer.Keyboard.isPressed()) {
            if (status.enter) {
                if (!try_handle_custom(LV_KEY_ENTER)) key_queue.push_back(LV_KEY_ENTER);
            }
            if (status.del)   {
                if (!try_handle_custom(LV_KEY_BACKSPACE)) key_queue.push_back(LV_KEY_BACKSPACE);
            }

            if (M5Cardputer.Keyboard.isKeyPressed(KEY_TAB)) {
                uint16_t k = status.shift ? LV_KEY_PREV : LV_KEY_NEXT;
                if (!try_handle_custom('\t')) {
                    key_queue.push_back(k);
                }
            }

            for (auto c : status.word) {
                // Skip TAB character to avoid duplicate LV_KEY_NEXT events
                if (c == '\t') continue;
                // Map punctuation to navigation keys regardless of FN state
                if (c == ';')       { if (!try_handle_custom(';')) key_queue.push_back(LV_KEY_PREV);  continue; }
                if (c == ',')       { if (!try_handle_custom(',')) key_queue.push_back(LV_KEY_LEFT);  continue; }
                if (c == '.')       { if (!try_handle_custom('.')) key_queue.push_back(LV_KEY_NEXT);  continue; }
                if (c == '/')       { if (!try_handle_custom('/')) key_queue.push_back(LV_KEY_RIGHT); continue; }
                if (c == '`')       { if (!try_handle_custom('`')) g_exit_requested = true; continue; }
                // Default: enqueue ASCII character for text input
                if (!try_handle_custom(static_cast<uint8_t>(c))) {
                    key_queue.push_back(static_cast<uint8_t>(c));
                }
            }
        }
    }
}

bool kb_consume_exit_requested() {
    bool r = g_exit_requested;
    g_exit_requested = false;
    return r;
}
