#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_HIGH_STRESS_SCREEN = 2,
    _SCREEN_ID_LAST = 2
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *high_stress_screen;
    lv_obj_t *container1;
    lv_obj_t *btn_get;
    lv_obj_t *bnt_text;
    lv_obj_t *icon;
    lv_obj_t *txt_res;
    lv_obj_t *title_txt;
    lv_obj_t *water_chart;
    lv_obj_t *result_container;
    lv_obj_t *txt_highest;
    lv_obj_t *high_arc;
    lv_obj_t *btn_home;
    lv_obj_t *lbl_home;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_high_stress_screen();
void tick_screen_high_stress_screen();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/