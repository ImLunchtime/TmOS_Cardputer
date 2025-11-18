#include <M5Cardputer.h>
#include <lvgl.h>
#include <vector>

static lv_indev_t *kb_indev = nullptr;
static lv_group_t *kb_group = nullptr;
static std::vector<uint16_t> key_queue;
static uint16_t current_key = 0;
static bool emit_release_next = false;

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

void kb_process_hardware_keys() {
    M5Cardputer.update();

    if (M5Cardputer.Keyboard.isChange()) {
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
        if (M5Cardputer.Keyboard.isPressed()) {
            if (status.enter) key_queue.push_back(LV_KEY_ENTER);
            if (status.del)   key_queue.push_back(LV_KEY_BACKSPACE);

            if (M5Cardputer.Keyboard.isKeyPressed(KEY_TAB)) {
                if (status.shift) key_queue.push_back(LV_KEY_PREV);
                else              key_queue.push_back(LV_KEY_NEXT);
            }

            for (auto c : status.word) {
                // Skip TAB character to avoid duplicate LV_KEY_NEXT events
                if (c == '\t') continue;
                if (status.fn) {
                    if (c == ';')       key_queue.push_back(LV_KEY_PREV);
                    else if (c == '.')  key_queue.push_back(LV_KEY_NEXT);
                    else if (c == ',')  key_queue.push_back(LV_KEY_LEFT);
                    else if (c == '/')  key_queue.push_back(LV_KEY_RIGHT);
                    else                 key_queue.push_back(static_cast<uint8_t>(c));
                } else {
                    key_queue.push_back(static_cast<uint8_t>(c));
                }
            }
        }
    }
}