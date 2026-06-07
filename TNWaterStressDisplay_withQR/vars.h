#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_WATER_STRESS_RESULT = 0,
    FLOW_GLOBAL_VARIABLE_HIGHEST_STRESS_RESULT = 1
};

// Native global variables

extern const char *get_var_water_stress_result();
extern void set_var_water_stress_result(const char *value);
extern const char *get_var_highest_stress_result();
extern void set_var_highest_stress_result(const char *value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/