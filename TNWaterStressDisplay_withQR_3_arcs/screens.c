#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

//
// Screens
//

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 280);
    lv_obj_add_event_cb(obj, action_swipe_lr, LV_EVENT_GESTURE, (void *)0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xfff5f5f5), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // container1
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.container1 = obj;
            lv_obj_set_pos(obj, 8, 8);
            lv_obj_set_size(obj, 223, 263);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_event_cb(obj, action_swipe_lr, LV_EVENT_GESTURE, (void *)0);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0xffd77878), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_stop(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_main_stop(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // btnGet
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_get = obj;
                    lv_obj_set_pos(obj, 7, 208);
                    lv_obj_set_size(obj, 193, 50);
                    lv_obj_add_event_cb(obj, action_get_water_stress, LV_EVENT_CLICKED, (void *)0);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffadd1ed), LV_PART_MAIN | LV_STATE_PRESSED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // bntText
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.bnt_text = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Get Stress");
                        }
                    }
                }
                {
                    // icon
                    lv_obj_t *obj = lv_image_create(parent_obj);
                    objects.icon = obj;
                    lv_obj_set_pos(obj, 0, 170);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_image_set_src(obj, &img_watericon);
                }
                {
                    // txtRes
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.txt_res = obj;
                    lv_obj_set_pos(obj, 17, 20);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff1913e5), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
            }
        }
        {
            // titleTxt
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.title_txt = obj;
            lv_obj_set_pos(obj, 34, 182);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff201bd3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "WATER STRESS");
        }
        {
            // waterChart
            lv_obj_t *obj = lv_chart_create(parent_obj);
            objects.water_chart = obj;
            lv_obj_set_pos(obj, 22, 66);
            lv_obj_set_size(obj, 182, 112);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
    {
        const char *new_val = get_var_water_stress_result();
        const char *cur_val = lv_label_get_text(objects.txt_res);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.txt_res;
            lv_label_set_text(objects.txt_res, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_high_stress_screen() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.high_stress_screen = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 280);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffe78e8e), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0xffdd5858), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // resultContainer
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.result_container = obj;
            lv_obj_set_pos(obj, 5, 11);
            lv_obj_set_size(obj, 231, 259);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_event_cb(obj, action_swipe_lr, LV_EVENT_GESTURE, (void *)0);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // txtHighest
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.txt_highest = obj;
                    lv_obj_set_pos(obj, 3, 1);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff3018df), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
                {
                    // highArc
                    lv_obj_t *obj = lv_arc_create(parent_obj);
                    objects.high_arc = obj;
                    lv_obj_set_pos(obj, 3, 69);
                    lv_obj_set_size(obj, 161, 152);
                    lv_arc_set_value(obj, 25);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
                    lv_obj_set_style_bg_image_src(obj, &img_watericon, LV_PART_KNOB | LV_STATE_DEFAULT);
                }
                {
                    // seeReport
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.see_report = obj;
                    lv_obj_set_pos(obj, 175, 166);
                    lv_obj_set_size(obj, 49, 97);
                    lv_obj_add_event_cb(obj, action_go_to_qr_screen, LV_EVENT_CLICKED, (void *)0);
                    lv_obj_add_event_cb(obj, action_go_to_qr_screen, LV_EVENT_SINGLE_CLICKED, (void *)0);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // lblHome_1
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.lbl_home_1 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "<<");
                        }
                    }
                }
                {
                    // btnHome
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_home = obj;
                    lv_obj_set_pos(obj, 173, 49);
                    lv_obj_set_size(obj, 49, 97);
                    lv_obj_add_event_cb(obj, action_go_home, LV_EVENT_CLICKED, (void *)0);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // lblHome
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.lbl_home = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "<<");
                        }
                    }
                }
            }
        }
    }
    
    tick_screen_high_stress_screen();
}

void tick_screen_high_stress_screen() {
    {
        const char *new_val = get_var_highest_stress_result();
        const char *cur_val = lv_label_get_text(objects.txt_highest);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.txt_highest;
            lv_label_set_text(objects.txt_highest, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_report() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.report = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 280);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffea8686), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // thirdcontainer
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.thirdcontainer = obj;
            lv_obj_set_pos(obj, 7, 5);
            lv_obj_set_size(obj, 226, 270);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_event_cb(obj, action_swipe_lr, LV_EVENT_GESTURE, (void *)0);
            lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0xffe09595), LV_PART_KNOB | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // btnHome_1
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.btn_home_1 = obj;
                    lv_obj_set_pos(obj, 32, 231);
                    lv_obj_set_size(obj, 177, 37);
                    lv_obj_add_event_cb(obj, action_go_home, LV_EVENT_CLICKED, (void *)0);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // lblHome_2
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.lbl_home_2 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "<<");
                        }
                    }
                }
                {
                    // reportQR
                    lv_obj_t *obj = lv_qrcode_create(parent_obj);
                    objects.report_qr = obj;
                    lv_obj_set_pos(obj, 8, 33);
                    lv_obj_set_size(obj, 210, 192);
                    lv_qrcode_set_size(obj, 192);
                    lv_qrcode_set_dark_color(obj, lv_color_hex(0xff20429f));
                    lv_qrcode_set_light_color(obj, lv_color_hex(0xffe2f5fe));
                    lv_qrcode_update(obj, "https://gee-esp-dashboard.onrender.com/report", 45);
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
                }
                {
                    // tamilreport
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.tamilreport = obj;
                    lv_obj_set_pos(obj, 27, 7);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff3018df), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "TAMIL REPORT QR");
                }
            }
        }
    }
    
    tick_screen_report();
}

void tick_screen_report() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_high_stress_screen,
    tick_screen_report,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_main();
    create_screen_high_stress_screen();
    create_screen_report();
}