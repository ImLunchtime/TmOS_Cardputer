#include <Arduino.h>
#include <M5Cardputer.h>
#include <lvgl.h>

static void lv_tick_task(void *arg) {
    while (1) {
        lv_tick_inc(10);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void lvgl_task(void *arg) {
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void lvgl_setup() {
    lv_init();

    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf1[240 * 5];
    static lv_color_t buf2[240 * 5];
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, 240 * 5);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = [](lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
        uint32_t w = area->x2 - area->x1 + 1;
        uint32_t h = area->y2 - area->y1 + 1;
        M5.Display.startWrite();
        M5.Display.setAddrWindow(area->x1, area->y1, w, h);
        M5.Display.pushPixels((uint16_t *)&color_p->full, w * h, true);
        M5.Display.endWrite();
        lv_disp_flush_ready(disp_drv);
    };
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 135;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}

void start_lvgl_tasks() {
    xTaskCreatePinnedToCore(lv_tick_task, "lv_tick_task", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(lvgl_task, "lvgl_task", 8192, NULL, 1, NULL, 1);
}