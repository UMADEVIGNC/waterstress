#include <lvgl.h>
#include "ui.h"
#include "vars.h"
#include "screens.h"
#include "actions.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Arduino_GFX_Library.h>
#include "TouchDrvCSTXXX.hpp"
#include "index.h"

// ---------- Configuration ----------
WebServer server(80);
Preferences preferences;

// WiFi Configuration
String wifi_ssid = "";
String wifi_password = "";
bool wifi_configured = false;
unsigned long wifi_attempt_start = 0;
bool wifi_connecting = false;

// API Configuration
const char* STRESS_URL = "https://gee-esp-dashboard.onrender.com/stress-compact";

// AP Mode Configuration
const char* AP_SSID = "WaterStress_Config";
const char* AP_PASS = "12345678";

// ---------- EEZ variable storage ----------
static String waterStressText = "Press button to fetch";
static String highestStressText = "No data yet";

bool touchEnabled = true;
enum ScreenState {
  SCREEN_MAIN_STATE,
  SCREEN_HIGH_STRESS_STATE,
  SCREEN_REPORT_STATE
};

ScreenState currentScreen = SCREEN_MAIN_STATE;

int highestStressValue = 0;
static lv_chart_series_t *waterSeries = NULL;

// Store top 3 districts data
struct DistrictData {
  String name;
  int value;
};
DistrictData topDistricts[3];
int currentDisplayIndex = 0;
lv_timer_t *cycleTimer = NULL;

// Forward declarations
void refreshResultLabel();
void setHomeButtonText();
void disableArcTouch();
void startArcAnimationAfterScreenLoad();
void makeWaterStressTicker();
void initWaterChart();
void animateCurrentDistrict();
void updateTopStressData(String payload);
void updateWaterChart(String payload);
void cycleThroughDistricts();
void setupWiFi();
void startAPMode();
void checkWiFiConnection();
void setupWebServer();
void handleRoot();
void handleSaveWiFi();
void handleStatus();
void handleRestart();

// ---------- EEZ variable functions ----------
extern "C" const char *get_var_water_stress_result() {
  return waterStressText.c_str();
}

// Swipe to next screen
extern "C" void action_swipe_lr(lv_event_t *e)
{
  if (lv_event_get_code(e) != LV_EVENT_GESTURE)
    return;

  if (currentScreen != SCREEN_MAIN_STATE)
    return;

  lv_indev_t *indev = lv_indev_active();

  if (!indev)
    return;

  lv_dir_t dir = lv_indev_get_gesture_dir(indev);
  Serial.println("The swipe direction is" + String(dir));
  if (dir == LV_DIR_LEFT)
  {
    Serial.println("MAIN -> HIGH STRESS");

    loadScreen(SCREEN_ID_HIGH_STRESS_SCREEN);

    currentScreen = SCREEN_HIGH_STRESS_STATE;

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
void setHomeButtonText()
{
  if (objects.btn_home != NULL)
  {
    lv_obj_t *child = lv_obj_get_child(objects.btn_home, 0);
    if (child != NULL)
    {
      lv_label_set_text(child, LV_SYMBOL_HOME);
    }
  }

  if (objects.btn_home_1 != NULL)
  {
    lv_obj_t *child = lv_obj_get_child(objects.btn_home_1, 0);
    if (child != NULL)
    {
      lv_label_set_text(child, LV_SYMBOL_HOME);
    }
  }
  
  if (objects.see_report != NULL)
  {
    lv_obj_t *child = lv_obj_get_child(objects.see_report, 0);
    if (child != NULL)
    {
      lv_label_set_text(child, LV_SYMBOL_FILE);
    }
  }
}

// ---------- Chart init ----------
void initWaterChart() {
  if (objects.water_chart == NULL) {
    Serial.println("water_chart is NULL");
    return;
  }
  waterSeries = NULL;

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

// ---------- Animate current district (SMOOTH VERSION) ----------
void animateCurrentDistrict() {
  if (objects.high_arc == NULL) {
    Serial.println("ERROR: high_arc is NULL");
    return;
  }
  
  int targetValue = constrain(topDistricts[currentDisplayIndex].value, 0, 100);
  String districtName = topDistricts[currentDisplayIndex].name;
  
  Serial.printf("Animating district %d: %s - %d%%\n", 
                currentDisplayIndex + 1, districtName.c_str(), targetValue);
  
  // Set color based on stress level
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
  
  // Update text label with district and rank
  if (objects.txt_highest != NULL) {
    char labelText[64];
    snprintf(labelText, sizeof(labelText), "#%d: %s\n%d%%", 
             currentDisplayIndex + 1, districtName.c_str(), targetValue);
    lv_label_set_text(objects.txt_highest, labelText);
  }
  
  // Animate from current value to target value (smoother)
  int currentValue = lv_arc_get_value(objects.high_arc);
  
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, objects.high_arc);
  lv_anim_set_values(&a, currentValue, targetValue);
  lv_anim_set_time(&a, 800);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
  
  lv_anim_set_exec_cb(&a, [](void *obj, int32_t v) {
    lv_arc_set_value((lv_obj_t *)obj, v);
  });
  
  lv_anim_start(&a);
}

// ---------- Cycle through top 3 districts ----------
void cycleThroughDistricts() {
  if (currentScreen != SCREEN_HIGH_STRESS_STATE) {
    if (cycleTimer != NULL) {
      lv_timer_del(cycleTimer);
      cycleTimer = NULL;
    }
    return;
  }

  // Move to next district
  int nextIndex = (currentDisplayIndex + 1) % 3;
  
  // Only change if the next district has data
  if (topDistricts[nextIndex].value > 0 || topDistricts[nextIndex].name != "No data") {
    currentDisplayIndex = nextIndex;
    animateCurrentDistrict();
  } else if (topDistricts[0].value > 0) {
    currentDisplayIndex = 0;
    animateCurrentDistrict();
  }
}

// ---------- Delayed arc animation ----------
void startArcAnimationAfterScreenLoad() {
  // Reset to first district
  currentDisplayIndex = 0;
  
  // Check if we have any data
  bool hasData = (topDistricts[0].value > 0 || topDistricts[1].value > 0 || topDistricts[2].value > 0);
  
  if (hasData) {
    // Set initial arc value without animation
    if (objects.high_arc != NULL) {
      lv_arc_set_value(objects.high_arc, 0);
    }
    
    // Animate first district
    animateCurrentDistrict();
    
    // Start timer to cycle every 5 seconds
    if (cycleTimer != NULL) {
      lv_timer_del(cycleTimer);
    }
    cycleTimer = lv_timer_create([](lv_timer_t *t) {
      cycleThroughDistricts();
    }, 5000, NULL);
  } else {
    // No data available
    if (objects.txt_highest != NULL) {
      lv_label_set_text(objects.txt_highest, "No data\nFetch from main screen");
    }
    if (objects.high_arc != NULL) {
      lv_arc_set_value(objects.high_arc, 0);
    }
  }
}

// ---------- Extract top 3 stress data (FIXED PARSER) ----------
void updateTopStressData(String payload) {
  Serial.println("Parsing stress data...");
  Serial.println("Raw payload: " + payload);
  
  // Simple array-based parsing
  struct TempStress {
    char district[32];
    int value;
  };
  
  TempStress tempList[10];
  int itemCount = 0;
  
  String temp = payload;
  
  // Parse JSON-like format: [{"d":"Name","h":value},...]
  int index = 0;
  while (itemCount < 10 && index < temp.length()) {
    // Find "d":" pattern
    int dPos = temp.indexOf("\"d\":\"", index);
    if (dPos < 0) break;
    
    int startName = dPos + 5; // After "d":"
    int endName = temp.indexOf("\"", startName);
    if (endName < 0) break;
    
    String district = temp.substring(startName, endName);
    
    // Find "h": pattern
    int hPos = temp.indexOf("\"h\":", endName);
    if (hPos < 0) break;
    
    int startValue = hPos + 4; // After "h":
    int endValue = startValue;
    while (endValue < temp.length() && 
           (isDigit(temp[endValue]) || temp[endValue] == '.')) {
      endValue++;
    }
    
    String valueStr = temp.substring(startValue, endValue);
    float valueFloat = valueStr.toFloat();
    int value = constrain((int)(valueFloat + 0.5), 0, 100);
    
    // Check for duplicate district (case-insensitive)
    bool duplicate = false;
    for (int i = 0; i < itemCount; i++) {
      if (String(tempList[i].district).equalsIgnoreCase(district)) {
        duplicate = true;
        break;
      }
    }
    
    if (!duplicate) {
      strncpy(tempList[itemCount].district, district.c_str(), 31);
      tempList[itemCount].district[31] = '\0';
      tempList[itemCount].value = value;
      itemCount++;
      
      Serial.printf("  Found: %s = %d%%\n", district.c_str(), value);
    }
    
    index = endValue;
  }
  
  // Sort by value (descending)
  for (int i = 0; i < itemCount - 1; i++) {
    for (int j = i + 1; j < itemCount; j++) {
      if (tempList[i].value < tempList[j].value) {
        TempStress t = tempList[i];
        tempList[i] = tempList[j];
        tempList[j] = t;
      }
    }
  }
  
  // Store top 3
  int topCount = min(3, itemCount);
  
  for (int i = 0; i < topCount; i++) {
    topDistricts[i].name = String(tempList[i].district);
    topDistricts[i].value = tempList[i].value;
    
    if (i == 0) {
      String resultText = String(tempList[i].district) + ": " + String(tempList[i].value) + "%";
      set_var_highest_stress_result(resultText.c_str());
      highestStressValue = tempList[i].value;
    }
  }
  
  // Fill empty slots
  for (int i = topCount; i < 3; i++) {
    topDistricts[i].name = "No data";
    topDistricts[i].value = 0;
  }
  
  Serial.println("Top 3 districts (sorted):");
  for (int i = 0; i < 3; i++) {
    Serial.printf("  %d: %s - %d%%\n", i+1, topDistricts[i].name.c_str(), topDistricts[i].value);
  }
}

// ---------- WiFi Configuration ----------
void setupWiFi() {
  preferences.begin("wifi", false);
  wifi_ssid = preferences.getString("ssid", "");
  wifi_password = preferences.getString("pass", "");
  preferences.end();
  
  if (wifi_ssid.length() > 0) {
    Serial.printf("Connecting to WiFi: %s\n", wifi_ssid.c_str());
    wifi_connecting = true;
    wifi_attempt_start = millis();
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  } else {
    Serial.println("No WiFi credentials found, starting AP mode");
    startAPMode();
  }
}

void startAPMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.printf("AP Mode started. SSID: %s, IP: %s\n", AP_SSID, WiFi.softAPIP().toString().c_str());
  set_var_water_stress_result("Configure WiFi");
  set_var_highest_stress_result("Connect to AP");
}

void checkWiFiConnection() {
  if (!wifi_connecting) return;
  
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connecting = false;
    wifi_configured = true;
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    set_var_water_stress_result("Press button to fetch");
    set_var_highest_stress_result("Ready");
  } else if (millis() - wifi_attempt_start > 20000) {
    wifi_connecting = false;
    Serial.println("WiFi connection timeout, starting AP mode");
    startAPMode();
  }
}

// ---------- Web Server ----------
void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSaveWiFi);
  server.on("/status", handleStatus);
  server.on("/restart", handleRestart);
  server.begin();
  Serial.println("HTTP server started");
}

void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleSaveWiFi() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String new_ssid = server.arg("ssid");
    String new_password = server.arg("password");
    
    preferences.begin("wifi", false);
    preferences.putString("ssid", new_ssid);
    preferences.putString("pass", new_password);
    preferences.end();
    
    server.send(200, "text/html", "<html><body><h2>Credentials Saved!</h2><p>Device will restart in 5 seconds...</p><script>setTimeout(function(){ window.location.href='/restart'; },5000);</script></body></html>");
  } else {
    server.send(400, "text/plain", "Missing SSID or password");
  }
}

void handleStatus() {
  String status = "{";
  status += "\"wifi_configured\":" + String(wifi_configured ? "true" : "false") + ",";
  status += "\"wifi_connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
  status += "\"ssid\":\"" + (wifi_configured ? wifi_ssid : "") + "\",";
  status += "\"ip\":\"" + (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "") + "\"";
  status += "}";
  server.send(200, "application/json", status);
}

void handleRestart() {
  server.send(200, "text/html", "<html><body><h2>Restarting...</h2></body></html>");
  delay(1000);
  ESP.restart();
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
  http.setTimeout(10000);
  
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("Raw payload: " + payload);
    
    updateTopStressData(payload);
    updateWaterChart(payload);
    
    // Create a clean display string for main screen
    String displayText = "";
    for (int i = 0; i < 3; i++) {
      if (topDistricts[i].value > 0) {
        if (i > 0) displayText += " | ";
        displayText += topDistricts[i].name + ": " + String(topDistricts[i].value) + "%";
      }
    }
    
    if (displayText.length() > 0) {
      set_var_water_stress_result(displayText.c_str());
    } else {
      set_var_water_stress_result("No valid data");
    }
    
    // If we're on the high stress screen, restart the cycling
    if (currentScreen == SCREEN_HIGH_STRESS_STATE) {
      if (cycleTimer != NULL) {
        lv_timer_del(cycleTimer);
        cycleTimer = NULL;
      }
      startArcAnimationAfterScreenLoad();
    }
  } else {
    String err = "HTTP ";
    err += httpCode;
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

  currentScreen = SCREEN_HIGH_STRESS_STATE;

  refreshResultLabel();
  setHomeButtonText();
  disableArcTouch();
  startArcAnimationAfterScreenLoad();
}

// ---------- Home button click ----------
extern "C" void action_go_home(lv_event_t *e)
{
  if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    return;

  Serial.println("HOME BUTTON CLICKED");

  // Stop the cycling timer
  if (cycleTimer != NULL) {
    lv_timer_del(cycleTimer);
    cycleTimer = NULL;
  }

  loadScreen(SCREEN_ID_MAIN);

  currentScreen = SCREEN_MAIN_STATE;

  waterSeries = NULL;

  refreshResultLabel();

  makeWaterStressTicker();

  initWaterChart();
}

// ---------- QR Screen ----------
extern "C" void action_go_to_qr_screen(lv_event_t *e)
{
  if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    return;

  Serial.println("QR SCREEN GOTO CLICKED");

  loadScreen(SCREEN_ID_REPORT);

  currentScreen = SCREEN_REPORT_STATE;

  setHomeButtonText();

  refreshResultLabel();
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
  setHomeButtonText();
  makeWaterStressTicker();
  currentScreen = SCREEN_MAIN_STATE;
  initWaterChart();

  // Initialize arc
  if (objects.high_arc != NULL) {
    lv_arc_set_range(objects.high_arc, 0, 100);
    lv_arc_set_value(objects.high_arc, 0);
    Serial.println("high_arc initialized successfully");
  } else {
    Serial.println("ERROR: high_arc is NULL!");
  }

  // Verify txt_highest
  if (objects.txt_highest != NULL) {
    Serial.println("txt_highest initialized successfully");
  } else {
    Serial.println("ERROR: txt_highest is NULL!");
  }

  set_var_water_stress_result("Starting...");
  set_var_highest_stress_result("Configuring WiFi");
  refreshResultLabel();

  // Setup WiFi and web server
  setupWiFi();
  setupWebServer();

  Serial.println("Setup done");
}

void loop() {
  lv_timer_handler();
  ui_tick();
  checkWiFiConnection();
  server.handleClient();
  delay(5);
}