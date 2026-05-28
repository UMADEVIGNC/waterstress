# 🌱 Soil Moisture Stress Monitor – Edge Visualization

[![Platform](https://img.shields.io/badge/Platform-ESP32--C6-blue)](https://www.waveshare.com/wiki/ESP32-C6-Touch-LCD-1.69)
[![Framework](https://img.shields.io/badge/Framework-Arduino-red)](https://www.arduino.cc/)
[![UI](https://img.shields.io/badge/UI-LVGL-green)](https://lvgl.io/)

A low-cost, battery-capable edge device that visualizes **satellite-derived soil moisture stress** at the district level. Built for the Waveshare ESP32-C6-Touch-LCD-1.69, this device fetches data from a cloud API (powered by Google Earth Engine + Sentinel-1 SAR) and displays stress metrics via an intuitive touchscreen interface.

---

## 📌 Overview

This project is the **edge visualization component** of a larger framework that:

1. Processes Sentinel-1 SAR data in Google Earth Engine
2. Computes a baseline-normalized **Moisture Stress Index (Z-score)**
3. Serves district-level metrics via a REST API
4. Displays them on this ESP32-based device

**Use Case:** Agricultural drought monitoring for farmers, extension officers, and local administrators in off-grid or low-internet settings.

---

## 🖥️ Hardware Requirements

| Component | Specification |
|-----------|---------------|
| **Board** | Waveshare ESP32-C6-Touch-LCD-1.69 |
| **Display** | 1.69" 240×280 ST7789 (touch-capacitive) |
| **Connectivity** | Wi-Fi 6 (2.4 GHz) |
| **Power** | USB-C or 3.7V Li-ion battery |

---

## 📦 Features

- ✅ Fetches **district-wise stress** data from cloud API
- ✅ Displays **mean stress**, **max stress**, and **high-stress percentage**
- ✅ Visualizes historical trends with **bar chart**
- ✅ Shows **highest stress district** with animated arc gauge
- ✅ **Power management** – deep sleep, wake on PWR button
- ✅ Touch navigation – swipe left for detailed view, home button to return
- ✅ Offline-capable – retains last fetched data

---

## 🚀 Getting Started

### 1. Install Dependencies

Add these libraries via Arduino Library Manager:

| Library | Version | Purpose |
|---------|---------|---------|
| `lvgl` | ≥ 8.4.0 | GUI framework |
| `GFX_Library_for_Arduino` | ≥ 1.6.0 | Display driver |
| `OneButton` | ≥ 2.6.1 | Button handling |
| `Arduino_GFX` | latest | SPI display support |

### 2. Configure Wi-Fi & API

Edit the following lines in the code:

```cpp
const char* WIFI_SSID = "Your_SSID";
const char* WIFI_PASS = "Your_Password";
const char* STRESS_URL = "https://your-api.com/stress-compact";
