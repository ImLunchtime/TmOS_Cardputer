#include "apps/radiosim/app_radiosim.h"

LV_FONT_DECLARE(simhei_12);

AppRadioSim::AppRadioSim() {}
AppRadioSim::~AppRadioSim() {}

void AppRadioSim::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_ROW); // Split into Left/Right
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_set_style_pad_gap(root_, 0, 0);
    lv_obj_set_style_bg_color(root_, lv_color_hex(0xE0D8B0), 0); // Yellowed white plastic chassis
    lv_obj_set_style_radius(root_, 0, 0);
    lv_obj_set_scroll_dir(root_, LV_DIR_NONE);

    // --- LEFT COLUMN (Display + Playback) ---
    lv_obj_t* left_col = lv_obj_create(root_);
    lv_obj_set_height(left_col, lv_pct(100));
    lv_obj_set_flex_grow(left_col, 2); // Takes ~66% width
    lv_obj_set_style_bg_color(left_col, lv_color_hex(0xE0D8B0), 0);
    lv_obj_set_style_pad_all(left_col, 2, 0);
    lv_obj_set_style_pad_gap(left_col, 4, 0);
    lv_obj_set_style_border_width(left_col, 0, 0);
    lv_obj_set_style_radius(left_col, 0, 0);
    lv_obj_set_flex_flow(left_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(left_col, LV_SCROLLBAR_MODE_OFF);

    // 1. Frequency Display (LCD)
    lv_obj_t* display_panel = lv_obj_create(left_col);
    lv_obj_set_width(display_panel, lv_pct(100));
    lv_obj_set_height(display_panel, 55); // Slightly taller
    lv_obj_set_style_bg_color(display_panel, lv_color_hex(0x9ACD32), 0); // YellowGreen LCD
    lv_obj_set_style_pad_all(display_panel, 0, 0);
    lv_obj_set_style_border_width(display_panel, 2, 0);
    lv_obj_set_style_border_color(display_panel, lv_color_hex(0x606060), 0);
    lv_obj_set_style_radius(display_panel, 4, 0); // Slight rounded corners for LCD
    lv_obj_set_scrollbar_mode(display_panel, LV_SCROLLBAR_MODE_OFF);

    // Frequency Label
    lv_obj_t* freq_label = lv_label_create(display_panel);
    lv_label_set_text(freq_label, "FM 88.6 MHz");
    lv_obj_set_style_text_font(freq_label, &simhei_12, 0);
    lv_obj_set_style_text_color(freq_label, lv_color_hex(0x102010), 0);
    lv_obj_align(freq_label, LV_ALIGN_CENTER, 0, -6);

    // Info Label
    lv_obj_t* info_label = lv_label_create(display_panel);
    lv_label_set_text(info_label, "STEREO SIGNAL:5");
    lv_obj_set_style_text_font(info_label, &simhei_12, 0);
    lv_obj_set_style_text_color(info_label, lv_color_hex(0x102010), 0);
    lv_obj_align(info_label, LV_ALIGN_BOTTOM_MID, 0, -2);

    // 2. Playback Controls (Silver Area)
    lv_obj_t* playback_panel = lv_obj_create(left_col);
    lv_obj_set_width(playback_panel, lv_pct(100));
    lv_obj_set_flex_grow(playback_panel, 1);
    // Silver gradient effect (simple solid color for now, maybe lighter grey)
    lv_obj_set_style_bg_color(playback_panel, lv_color_hex(0xC0C0C0), 0); 
    lv_obj_set_style_bg_grad_color(playback_panel, lv_color_hex(0x808080), 0);
    lv_obj_set_style_bg_grad_dir(playback_panel, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_pad_all(playback_panel, 2, 0);
    lv_obj_set_style_pad_gap(playback_panel, 2, 0);
    lv_obj_set_style_border_width(playback_panel, 1, 0);
    lv_obj_set_style_border_color(playback_panel, lv_color_hex(0x606060), 0);
    lv_obj_set_style_radius(playback_panel, 4, 0);
    lv_obj_set_flex_flow(playback_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(playback_panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(playback_panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(playback_panel, LV_OBJ_FLAG_SCROLLABLE);

    // Row 1: Play, Stop, Rec
    lv_obj_t* row1 = lv_obj_create(playback_panel);
    lv_obj_set_size(row1, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row1, 0, 0);
    lv_obj_set_style_border_width(row1, 0, 0);
    lv_obj_set_flex_flow(row1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row1, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(row1, 0, 0);

    const char* row1_syms[] = {LV_SYMBOL_PLAY, LV_SYMBOL_STOP, LV_SYMBOL_BULLET}; // Bullet for Rec
    uint32_t row1_colors[] = {0x008000, 0xFF0000, 0xFF0000}; // Green, Red, Red

    for(int i=0; i<3; i++) {
        lv_obj_t* btn = lv_btn_create(row1);
        lv_obj_set_size(btn, 26, 22); // Smaller buttons
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xE0E0E0), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x606060), 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_set_style_radius(btn, 10, 0); // Rounder buttons

        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, row1_syms[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(row1_colors[i]), 0);
        lv_obj_center(lbl);
    }

    // Row 2: Prev, Next
    lv_obj_t* row2 = lv_obj_create(playback_panel);
    lv_obj_set_size(row2, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row2, 0, 0);
    lv_obj_set_style_border_width(row2, 0, 0);
    lv_obj_set_flex_flow(row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row2, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(row2, 0, 0);

    const char* row2_syms[] = {LV_SYMBOL_PREV, LV_SYMBOL_NEXT};
    
    for(int i=0; i<2; i++) {
        lv_obj_t* btn = lv_btn_create(row2);
        lv_obj_set_size(btn, 34, 22); // Smaller buttons
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xE0E0E0), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x606060), 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_set_style_radius(btn, 4, 0);

        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, row2_syms[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0x000000), 0);
        lv_obj_center(lbl);
    }


    // --- RIGHT COLUMN (Function Buttons) ---
    lv_obj_t* right_col = lv_obj_create(root_);
    lv_obj_set_height(right_col, lv_pct(100));
    lv_obj_set_flex_grow(right_col, 1); // Takes remaining width
    lv_obj_set_style_bg_color(right_col, lv_color_hex(0xE0D8B0), 0); // Match chassis
    lv_obj_set_style_pad_all(right_col, 2, 0);
    lv_obj_set_style_pad_gap(right_col, 2, 0);
    lv_obj_set_style_border_width(right_col, 0, 0); // No border between cols? Or maybe separate visually
    lv_obj_set_style_radius(right_col, 0, 0);
    lv_obj_set_flex_flow(right_col, LV_FLEX_FLOW_ROW_WRAP); // Wrap buttons
    lv_obj_set_flex_align(right_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scrollbar_mode(right_col, LV_SCROLLBAR_MODE_OFF);

    // Buttons (Existing logic adapted)
    const char* labels[] = {"F-", "F+", "V-", "V+", "RX","PTT", "LCK", "MUT", "DIG"};
    int num_labels = sizeof(labels) / sizeof(labels[0]);

    for(int i=0; i<num_labels; i++) {
        lv_obj_t* btn = lv_btn_create(right_col);
        lv_obj_set_width(btn, lv_pct(48)); // 2 buttons per row approx
        lv_obj_set_height(btn, 28);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xD0C8A0), 0); 
        lv_obj_set_style_radius(btn, 3, 0);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xA09870), 0);
        
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, labels[i]);
        lv_obj_set_style_text_font(lbl, &simhei_12, 0);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0x403020), 0); 
        lv_obj_center(lbl);
    }
}

void AppRadioSim::onClose() {}
