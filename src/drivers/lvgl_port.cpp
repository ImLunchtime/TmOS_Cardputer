#include <Arduino.h>
#include <M5Cardputer.h>
#include <lvgl.h>
#include <SD.h>
#include <SPI.h>

static void* sd_open(lv_fs_drv_t* drv, const char* path, lv_fs_mode_t mode) {
    (void)drv;
    String p = path ? String(path) : String("/");
    if (p.length() > 1 && p.charAt(1) == ':') p = p.substring(2);
    if (!p.startsWith("/")) p = String("/") + p;
    File f = SD.open(p.c_str(), (mode == LV_FS_MODE_WR) ? FILE_WRITE : FILE_READ);
    if (!f) {
        File f2 = SD.open((String("/sdcard") + p).c_str(), (mode == LV_FS_MODE_WR) ? FILE_WRITE : FILE_READ);
        if (!f2) {
            File f3 = SD.open((String("/sd") + p).c_str(), (mode == LV_FS_MODE_WR) ? FILE_WRITE : FILE_READ);
            if (!f3) return nullptr;
            File* fp3 = new File(f3);
            return fp3;
        }
        File* fp2 = new File(f2);
        return fp2;
    }
    if (!f) return nullptr;
    File* fp = new File(f);
    return fp;
}

static lv_fs_res_t sd_close(lv_fs_drv_t* drv, void* file_p) {
    (void)drv;
    File* fp = (File*)file_p;
    if (!fp) return LV_FS_RES_FS_ERR;
    fp->close();
    delete fp;
    return LV_FS_RES_OK;
}

static lv_fs_res_t sd_read(lv_fs_drv_t* drv, void* file_p, void* buf, uint32_t btr, uint32_t* br) {
    (void)drv;
    File* fp = (File*)file_p;
    if (!fp) return LV_FS_RES_FS_ERR;
    int r = fp->read((uint8_t*)buf, btr);
    if (r < 0) return LV_FS_RES_FS_ERR;
    if (br) *br = (uint32_t)r;
    return LV_FS_RES_OK;
}

static lv_fs_res_t sd_seek(lv_fs_drv_t* drv, void* file_p, uint32_t pos, lv_fs_whence_t whence) {
    (void)drv;
    File* fp = (File*)file_p;
    if (!fp) return LV_FS_RES_FS_ERR;
    uint32_t target = pos;
    if (whence == LV_FS_SEEK_CUR) target = fp->position() + pos;
    else if (whence == LV_FS_SEEK_END) target = fp->size() + pos;
    if (!fp->seek(target)) return LV_FS_RES_FS_ERR;
    return LV_FS_RES_OK;
}

static lv_fs_res_t sd_tell(lv_fs_drv_t* drv, void* file_p, uint32_t* pos_p) {
    (void)drv;
    File* fp = (File*)file_p;
    if (!fp || !pos_p) return LV_FS_RES_FS_ERR;
    *pos_p = fp->position();
    return LV_FS_RES_OK;
}

static void* sd_dir_open(lv_fs_drv_t* drv, const char* path) {
    (void)drv;
    String p = path ? String(path) : String("/");
    if (p.length() > 1 && p.charAt(1) == ':') p = p.substring(2);
    if (!p.startsWith("/")) p = String("/") + p;
    File dir = SD.open(p.c_str());
    if (!dir || !dir.isDirectory()) {
        File dir2 = SD.open((String("/sdcard") + p).c_str());
        if (!dir2 || !dir2.isDirectory()) {
            File dir3 = SD.open((String("/sd") + p).c_str());
            if (!dir3 || !dir3.isDirectory()) return nullptr;
            File* dp3 = new File(dir3);
            return dp3;
        }
        File* dp2 = new File(dir2);
        return dp2;
    }
    File* dp = new File(dir);
    return dp;
}

static lv_fs_res_t sd_dir_read(lv_fs_drv_t* drv, void* rddir_p, char* fn) {
    (void)drv;
    if (!rddir_p || !fn) return LV_FS_RES_FS_ERR;
    File* dp = (File*)rddir_p;
    if (!dp) return LV_FS_RES_FS_ERR;
    File entry = dp->openNextFile();
    if (!entry) { fn[0] = '\0'; return LV_FS_RES_OK; }
    String name = entry.name();
    int last = name.lastIndexOf('/');
    if (last >= 0) name = name.substring(last + 1);
    size_t maxn = LV_FS_MAX_FN_LENGTH - 1;
    strncpy(fn, name.c_str(), maxn);
    fn[maxn] = '\0';
    entry.close();
    return LV_FS_RES_OK;
}

static lv_fs_res_t sd_dir_close(lv_fs_drv_t* drv, void* rddir_p) {
    (void)drv;
    File* dp = (File*)rddir_p;
    if (!dp) return LV_FS_RES_FS_ERR;
    dp->close();
    delete dp;
    return LV_FS_RES_OK;
}

static void register_sd_fs() {
    lv_fs_drv_t drv;
    lv_fs_drv_init(&drv);
    drv.letter = 'S';
    drv.open_cb = sd_open;
    drv.close_cb = sd_close;
    drv.read_cb = sd_read;
    drv.seek_cb = sd_seek;
    drv.tell_cb = sd_tell;
    drv.dir_open_cb = sd_dir_open;
    drv.dir_read_cb = sd_dir_read;
    drv.dir_close_cb = sd_dir_close;
    lv_fs_drv_register(&drv);
}

void lvgl_setup() {
    lv_init();

    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf1[240 * 30];
    static lv_color_t buf2[240 * 30];
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, 240 * 30);

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
    register_sd_fs();
}
