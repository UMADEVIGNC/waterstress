# 🌱 District Level Water Stress Monitor using Satellite Data – Edge Visualization

[![Platform](https://img.shields.io/badge/Platform-ESP32--C6-blue)](https://www.waveshare.com/wiki/ESP32-C6-Touch-LCD-1.69)
[![Framework](https://img.shields.io/badge/Framework-Arduino-red)](https://www.arduino.cc/)
[![UI](https://img.shields.io/badge/UI-LVGL-green)](https://lvgl.io/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![API](https://img.shields.io/badge/API-Render.com-purple)](https://render.com)

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

Follow these steps in order to set up the complete system from satellite processing to edge device visualization.

---

### Phase 1: Cloud Infrastructure Setup

#### 1.1 Google Earth Engine (GEE) – SAR Processing

1. Go to [Google Earth Engine Code Editor](https://code.earthengine.google.com/)
2. Copy the provided GEE script into a new file
3. Set your analysis years (baseline: 2021–2025, current: 2026)
4. Run the script and verify the district-level stress table output
5. Export the results as CSV/GeoJSON when satisfied

#### 1.2 Create GEE Service Account (for API access)

| Step | Action |
|------|--------|
| 1 | Go to [Google Cloud Console](https://console.cloud.google.com/) |
| 2 | Enable **Earth Engine API** for your project |
| 3 | Create a **Service Account** with Earth Engine access |
| 4 | Generate a **private key** in JSON format – download and save securely |
| 5 | Note your **Project ID** and **Service Account email** |

#### 1.3 Deploy Cloud API (Render.com or similar)

**Required environment variables** in your hosting platform:

```env
GEE_SERVICE_ACCOUNT=your-service-account@project.iam.gserviceaccount.com
GEE_PRIVATE_KEY_JSON={"type":"service_account","project_id":"..."}
GEE_PROJECT_ID=your-project-id

### 2. Configure Wi-Fi & API

Edit the following lines in the Arduino code:

```cpp
const char* WIFI_SSID = "Your_SSID";
const char* WIFI_PASS = "Your_Password";
const char* STRESS_URL = "https://your-api.com/stress-compact";
