#include <lvgl.h>
#include "ui.h"
#include "vars.h"
#include "screens.h"
#include "actions.h"

#include <WiFi.h>
#include <HTTPClient.h>

#include <Arduino_GFX_Library.h>
#include "TouchDrvCSTXXX.hpp"

// ---------- WiFi + API ----------
const char* WIFI_SSID = "Your wifi SSID";
const char* WIFI_PASS = "Your Wifi Pasword";
const char* STRESS_URL = "Cloud url where you have uploaded the python code given in this project";

// ---------- EEZ variable storage ----------
static String waterStressText = "Press button to fetch";
static String highestStressText = "No data yet";

bool touchEnabled = true;
bool onHighStressScreen = false;

int highestStressValue = 0;
static lv_chart_series_t *waterSeries = NULL;
void refreshResultLabel();
void setHomeButtonText();
void disableArcTouch();
void startArcAnimationAfterScreenLoad();
void makeWaterStressTicker();
// ---------- EEZ variable functions ----------
extern "C" const char *get_var_water_stress_result() {
  return waterStressText.c_str();
}
// Swipe to next screen
extern "C" void action_swipe_lr(lv_event_t *e) {
  if (lv_event_get_code(e) != LV_EVENT_GESTURE) return;

  if (onHighStressScreen) return;

  lv_indev_t *indev = lv_indev_active();
  if (indev == NULL) return;

  lv_dir_t dir = lv_indev_get_gesture_dir(indev);

  if (dir == LV_DIR_LEFT) {
    Serial.println("Swipe LEFT: opening HIGH STRESS screen");

    loadScreen(SCREEN_ID_HIGH_STRESS_SCREEN);

    onHighStressScreen = true;

    refreshResultLabel();
    setHomeButtonText();
    disableArcTouch();
    startArcAnimationAfterScreenLoad();
  }
}

extern "C" void set_var_water_stress_result(const char *value) {
  waterStressText = value;
}

extern "C" const char *get_var_highest_stress_result() {
  return highestStressText.c_str();
}

extern "C" void set_var_highest_stress_result(const char *value) {
  highestStressText = value;
}

void makeWaterStressTicker() {
  if (objects.txt_res == NULL) {
    Serial.println("txt_res is NULL");
    return;
  }

  lv_label_set_long_mode(objects.txt_res, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(objects.txt_res, 220);
  lv_obj_set_height(objects.txt_res, 30);
  lv_obj_remove_flag(objects.txt_res, LV_OBJ_FLAG_SCROLLABLE);
}

// ---------- Refresh EEZ labels ----------
void refreshResultLabel() {
  ui_tick();
  lv_timer_handler();
}


// ---------- Set Home button text ----------
void setHomeButtonText() {
  if (objects.btn_home == NULL) {
    Serial.println("btn_home is NULL");
    return;
  }

  lv_obj_t *child = lv_obj_get_child(objects.btn_home, 0);

  if (child != NULL) {
    lv_label_set_text(child, LV_SYMBOL_HOME);
    Serial.println("Home button text set");
  }
}

// ---------- Chart init ----------
void initWaterChart() {
  if (objects.water_chart == NULL) {
    Serial.println("water_chart is NULL");
    return;
  }

  lv_chart_set_type(objects.water_chart, LV_CHART_TYPE_BAR);
  lv_chart_set_range(objects.water_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
  lv_chart_set_point_count(objects.water_chart, 8);

  lv_obj_remove_flag(objects.water_chart, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_remove_flag(objects.water_chart, LV_OBJ_FLAG_SCROLLABLE);

  Serial.println("Water chart initialized");
}

// ---------- Update chart ----------
void updateWaterChart(String payload) {
  if (objects.water_chart == NULL) {
    Serial.println("water_chart is NULL");
    return;
  }

  String temp = payload;

  temp.replace("[", "");
  temp.replace("]", "");
  temp.replace("{", "");
  temp.replace("}", "");
  temp.replace("\"", "");

  int count = 0;
  int index = 0;

  while ((index = temp.indexOf("h:", index)) != -1) {
    count++;
    index += 2;
  }

  if (count <= 0) {
    Serial.println("No chart values found");
    return;
  }

  lv_chart_set_type(objects.water_chart, LV_CHART_TYPE_BAR);
  lv_chart_set_range(objects.water_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
  lv_chart_set_point_count(objects.water_chart, count);

  if (waterSeries == NULL) {
    waterSeries = lv_chart_add_series(
      objects.water_chart,
      lv_palette_main(LV_PALETTE_BLUE),
      LV_CHART_AXIS_PRIMARY_Y
    );
  }

  int startPos = 0;
  int pointIndex = 0;

  while (true) {
    int hPos = temp.indexOf("h:", startPos);

    if (hPos < 0) break;

    int commaPos = temp.indexOf(",", hPos);

    String value;

    if (commaPos > 0) {
      value = temp.substring(hPos + 2, commaPos);
      startPos = commaPos + 1;
    } else {
      value = temp.substring(hPos + 2);
      startPos = temp.length();
    }

    value.trim();

    int stress = constrain((int)value.toFloat(), 0, 100);

    lv_chart_set_value_by_id(
      objects.water_chart,
      waterSeries,
      pointIndex,
      stress
    );

    pointIndex++;
  }

  lv_chart_refresh(objects.water_chart);

  Serial.println("Water chart updated");
}

// ---------- Arc display-only ----------
void disableArcTouch() {
  if (objects.high_arc != NULL) {
    lv_obj_remove_flag(objects.high_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(objects.high_arc, LV_OBJ_FLAG_SCROLLABLE);
  }
}

// ---------- Arc animation ----------
void animateHighStressArc(int targetValue) {
  if (objects.high_arc == NULL) {
    Serial.println("high_arc is NULL");
    return;
  }

  targetValue = constrain(targetValue, 0, 100);

  lv_arc_set_range(objects.high_arc, 0, 100);
  lv_arc_set_value(objects.high_arc, 0);

  lv_color_t c;

  if (targetValue < 25) {
    c = lv_palette_main(LV_PALETTE_GREEN);
  } else if (targetValue < 50) {
    c = lv_palette_main(LV_PALETTE_YELLOW);
  } else if (targetValue < 75) {
    c = lv_palette_main(LV_PALETTE_ORANGE);
  } else {
    c = lv_palette_main(LV_PALETTE_RED);
  }

  lv_obj_set_style_arc_color(objects.high_arc, c, LV_PART_INDICATOR);

  if (objects.txt_highest != NULL) {
    lv_obj_set_style_text_color(objects.txt_highest, lv_color_black(), LV_PART_MAIN);
  }

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, objects.high_arc);
  lv_anim_set_values(&a, 0, targetValue);
  lv_anim_set_time(&a, 1500);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_out);

  lv_anim_set_exec_cb(&a, [](void *obj, int32_t v) {
    lv_arc_set_value((lv_obj_t *)obj, v);
  });

  lv_anim_start(&a);

  Serial.print("Arc animation started: ");
  Serial.println(targetValue);
}

// ---------- Delayed arc animation ----------
void startArcAnimationAfterScreenLoad() {
  lv_timer_create([](lv_timer_t *t) {
    lv_timer_delete(t);

    Serial.println("Delayed arc animation running");
    animateHighStressArc(highestStressValue);

  }, 600, NULL);
}

// ---------- Extract highest stress ----------
void updateHighestStress(String payload) {
  String temp = payload;

  temp.replace("[", "");
  temp.replace("{", "");
  temp.replace("\"", "");

  int dPos = temp.indexOf("d:");
  int hPos = temp.indexOf("h:");

  if (dPos < 0 || hPos < 0) {
    set_var_highest_stress_result("No valid data");
    highestStressValue = 0;
    return;
  }

  int commaAfterDistrict = temp.indexOf(",", dPos);
  int commaAfterValue = temp.indexOf(",", hPos);

  if (commaAfterDistrict < 0) return;

  String district = temp.substring(dPos + 2, commaAfterDistrict);
  String value;

  if (commaAfterValue > 0) {
    value = temp.substring(hPos + 2, commaAfterValue);
  } else {
    value = temp.substring(hPos + 2);
  }

  district.trim();
  value.replace("}", "");
  value.trim();

  highestStressValue = constrain((int)value.toFloat(), 0, 100);

  String finalText = "The highest water stress was found in:\n" +
                     district + "\n" +
                     value + "%";

  set_var_highest_stress_result(finalText.c_str());

  Serial.print("Highest stress value saved: ");
  Serial.println(highestStressValue);
}

// ---------- Fetch button click ----------
extern "C" void action_get_water_stress(lv_event_t *e) {
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

  Serial.println("FETCH BUTTON CLICKED");

  touchEnabled = false;

  set_var_water_stress_result("Fetching...");
  set_var_highest_stress_result("Fetching...");
  refreshResultLabel();

  if (WiFi.status() != WL_CONNECTED) {
    set_var_water_stress_result("WiFi not connected");
    set_var_highest_stress_result("WiFi not connected");
    refreshResultLabel();

    touchEnabled = true;
    return;
  }

  HTTPClient http;
  http.begin(STRESS_URL);
  http.setTimeout(45000);

  int httpCode = http.GET();

  Serial.print("HTTP code: ");
  Serial.println(httpCode);

  if (httpCode == 200) {
    String payload = http.getString();

    Serial.println(payload);

    updateHighestStress(payload);
    updateWaterChart(payload);

    String labelPayload = payload;

    labelPayload.replace("[", "");
    labelPayload.replace("]", "");
    labelPayload.replace("{", "");
    labelPayload.replace("}", "");
    labelPayload.replace("\"", "");
    labelPayload.replace("d:", "");
    labelPayload.replace("h:", " ");

    // ticker separator
    labelPayload.replace(",", "   |   ");
    set_var_water_stress_result(labelPayload.c_str());
  } else {
    String err = "HTTP error: " + String(httpCode);

    set_var_water_stress_result(err.c_str());
    set_var_highest_stress_result(err.c_str());
  }

  http.end();

  refreshResultLabel();
  touchEnabled = true;
}

// ---------- Open high-stress screen button ----------
extern "C" void action_open_high_stress(lv_event_t *e) {
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

  Serial.println("OPEN HIGH STRESS BUTTON CLICKED");

  loadScreen(SCREEN_ID_HIGH_STRESS_SCREEN);

  onHighStressScreen = true;

  refreshResultLabel();
  setHomeButtonText();
  disableArcTouch();
  startArcAnimationAfterScreenLoad();
}

// ---------- Home button click ----------
extern "C" void action_go_home(lv_event_t *e) {
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

  Serial.println("HOME BUTTON CLICKED");

  loadScreen(SCREEN_ID_MAIN);

  onHighStressScreen = false;

  refreshResultLabel();
  makeWaterStressTicker();
  initWaterChart();
}

// ---------- Display Pins ----------
#define LCD_SCK 1
#define LCD_DIN 2
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6

#define I2C_SDA 8
#define I2C_SCL 7

#define GFX_BL LCD_BL

Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_DIN);

Arduino_GFX *gfx = new Arduino_ST7789(
  bus, LCD_RST, 0, true,
  240, 280,
  0, 20,
  0, 20
);

TouchDrvCSTXXX touch;

uint32_t screenWidth;
uint32_t screenHeight;
uint32_t bufSize;

lv_display_t *disp;
lv_color_t *disp_draw_buf;

uint32_t millis_cb(void) {
  return millis();
}

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

  gfx->draw16bitRGBBitmap(
    area->x1,
    area->y1,
    (uint16_t *)px_map,
    w,
    h
  );

  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (!touchEnabled) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  int16_t x[1], y[1];
  uint8_t touched = touch.getPoint(x, y, 1);

  if (touched > 0) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x[0];
    data->point.y = y[0];

    Serial.print("Touch X: ");
    Serial.print(x[0]);
    Serial.print("  Y: ");
    Serial.println(y[0]);
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void setup() {
  Serial.begin(115200);

  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }

  gfx->fillScreen(RGB565_BLACK);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  bool touchResult = touch.begin(Wire, CST816_SLAVE_ADDRESS, I2C_SDA, I2C_SCL);

  if (!touchResult) {
    Serial.println("Touch init failed!");
  } else {
    Serial.println("Touch init OK");
  }

  lv_init();
  lv_tick_set_cb(millis_cb);

  screenWidth = gfx->width();
  screenHeight = gfx->height();
  bufSize = screenWidth * 120;

  disp_draw_buf = (lv_color_t *)heap_caps_malloc(
    bufSize * 2,
    MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
  );

  if (!disp_draw_buf) {
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(
      bufSize * 2,
      MALLOC_CAP_8BIT
    );
  }

  if (!disp_draw_buf) {
    Serial.println("LVGL buffer allocation failed!");
    return;
  }

  disp = lv_display_create(screenWidth, screenHeight);
  lv_display_set_flush_cb(disp, my_disp_flush);

  lv_display_set_buffers(
    disp,
    disp_draw_buf,
    NULL,
    bufSize * 2,
    LV_DISPLAY_RENDER_MODE_PARTIAL
  );

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  ui_init();
  makeWaterStressTicker();
  onHighStressScreen = false;
  initWaterChart();

  
  if (objects.high_arc != NULL) {
    lv_arc_set_range(objects.high_arc, 0, 100);
    lv_arc_set_value(objects.high_arc, 0);
  }

  set_var_water_stress_result("Connecting WiFi...");
  set_var_highest_stress_result("Connecting WiFi...");
  refreshResultLabel();

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    delay(500);
    Serial.print(".");
    lv_timer_handler();
    ui_tick();
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    set_var_water_stress_result("Press button to fetch");
    set_var_highest_stress_result("No data yet");
  } else {
    Serial.println("\nWiFi failed");
    set_var_water_stress_result("WiFi failed");
    set_var_highest_stress_result("WiFi failed");
  }

  refreshResultLabel();

  Serial.println("Setup done");
}

void loop() {
  lv_timer_handler();
  ui_tick();
  delay(5);
}
