#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <vector>

// =====================
// WIFI
// =====================
const char* ssid = "Your SSID";
const char* password = "Password";

const char* RENDER_URL = "https://gee-esp-dashboard.onrender.com/stress-compact";

// =====================
// OLED
// =====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SCL 14
#define OLED_SDA 15
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// =====================
// PINS
// =====================
#define BUTTON_PIN 13
#define BUZZER_PIN 2

// =====================
// DISPLAY
// =====================
void showMessage(String msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(msg);
  display.display();
}

void beep(int times = 1) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(BUZZER_PIN, LOW);
    delay(120);
  }
}

// =====================
// STARTUP LOGO
// =====================
void showStartupLogo() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.fillCircle(64, 22, 13, SSD1306_WHITE);
  display.fillTriangle(51, 22, 77, 22, 64, 4, SSD1306_WHITE);
  display.fillCircle(59, 17, 3, SSD1306_BLACK);

  display.setTextSize(1);
  display.setCursor(22, 44);
  display.print("TN WATER STRESS");

  display.display();
}

// =====================
// SCROLL TEXT
// =====================
void scrollText(String text) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  int maxLineChars = 21;
  int lineHeight = 8;
  int visibleLines = 7;
  int scrollDelay = 1300;

  std::vector<String> lines;

  String word = "";
  String currentLine = "";

  for (int i = 0; i <= text.length(); i++) {
    char c = text.charAt(i);

    if (c == ' ' || c == '\n' || c == '\0') {
      if (word.length() > 0) {
        if (currentLine.length() + word.length() + 1 <= maxLineChars) {
          if (currentLine.length() > 0) currentLine += " ";
          currentLine += word;
        } else {
          if (currentLine.length() > 0) lines.push_back(currentLine);
          currentLine = word;
        }
        word = "";
      }

      if (c == '\n') {
        if (currentLine.length() > 0) {
          lines.push_back(currentLine);
          currentLine = "";
        } else {
          lines.push_back("");
        }
      }
    } else {
      word += c;
    }
  }

  if (currentLine.length() > 0) lines.push_back(currentLine);

  if (lines.size() <= visibleLines) {
    display.clearDisplay();
    for (int i = 0; i < lines.size(); i++) {
      display.setCursor(0, i * lineHeight);
      display.print(lines[i]);
    }
    display.display();
    delay(3000);
    return;
  }

  for (int i = 0; i <= lines.size() - visibleLines; i++) {
    display.clearDisplay();

    for (int j = 0; j < visibleLines; j++) {
      display.setCursor(0, j * lineHeight);
      display.print(lines[i + j]);
    }

    display.display();
    delay(scrollDelay);
  }
}

// =====================
// WIFI RECONNECT
// =====================
bool ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;

  showMessage("WiFi lost\nReconnecting...");

  WiFi.disconnect();
  delay(500);
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    showMessage("WiFi OK\nIP:\n" + WiFi.localIP().toString());
    delay(1200);
    return true;
  }

  return false;
}

// =====================
// FETCH FROM RENDER
// =====================
String fetchStressData() {
  if (!ensureWiFi()) {
    return "WIFI_ERROR";
  }

  int maxAttempts = 3;

  for (int attempt = 1; attempt <= maxAttempts; attempt++) {
    showMessage("Fetching Render\nAttempt " + String(attempt));

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setReuse(false);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(45000);

    bool beginOK = http.begin(client, RENDER_URL);

    if (!beginOK) {
      Serial.println("HTTP begin failed");
      http.end();
      delay(4000);
      continue;
    }

    int code = http.GET();

    Serial.print("HTTP code: ");
    Serial.println(code);

    if (code == HTTP_CODE_OK) {
      String payload = http.getString();
      http.end();

      Serial.println("Payload:");
      Serial.println(payload);

      if (payload.length() > 5) {
        return payload;
      }
    } else {
      Serial.print("HTTP error: ");
      Serial.println(http.errorToString(code));
    }

    http.end();

    showMessage("Render waking\nRetrying...");
    beep(1);
    delay(7000);
  }

  return "RENDER_SLEEP_OR_ERROR";
}

// =====================
// FORMAT JSON RESULT
// =====================
String formatStressResult(String json) {
  DynamicJsonDocument doc(4096);

  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print("JSON error: ");
    Serial.println(error.c_str());

    return "JSON parse failed\nRaw:\n" + json;
  }

  if (!doc.is<JsonArray>()) {
    return "Expected JSON array";
  }

  JsonArray arr = doc.as<JsonArray>();

  if (arr.size() == 0) {
    return "No district data\nreceived";
  }

  String output = "TN Moisture Stress\n\n";

  for (JsonObject item : arr) {
    String district = item["d"] | "Unknown";

    float highStress = 0.0;

    if (item["h"].is<float>() || item["h"].is<int>()) {
      highStress = item["h"].as<float>();
    } else if (item["h"].is<const char*>()) {
      highStress = String((const char*)item["h"]).toFloat();
    }

    output += district;
    output += "\nHigh Stress: ";
    output += String(highStress, 1);
    output += "%\n\n";
  }

  return output;
}

// =====================
// SETUP
// =====================
void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
    while (true);
  }

  showStartupLogo();
  delay(2500);

  showMessage("Connecting WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    beep(1);
    showMessage("WiFi Connected\nIP:\n" + WiFi.localIP().toString());
    delay(2500);
  } else {
    beep(4);
    showMessage("WiFi failed\nCheck details");
    delay(2500);
  }

  showMessage("Press button\nto fetch result");
}

// =====================
// LOOP
// =====================
void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(80);

    if (digitalRead(BUTTON_PIN) == LOW) {
      showMessage("Fetching from\nRender...");
      beep(1);

      String json = fetchStressData();

      if (json == "WIFI_ERROR") {
        beep(4);
        scrollText("WiFi disconnected. Could not reconnect. Check router or signal.");
      } 
      else if (json == "RENDER_SLEEP_OR_ERROR") {
        beep(4);
        scrollText("Render server is sleeping or not responding. Please wait and press again.");
      } 
      else {
        showMessage("Result received");
        beep(3);

        String resultText = formatStressResult(json);
        scrollText(resultText);
      }

      showMessage("Press button\nto fetch again");

      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }

      delay(300);
    }
  }
}