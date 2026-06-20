# Tanbre Watch Project Context & Guidelines

This document provides essential context, hardware specifications, and software architecture guidelines for the **Tanbre Watch** project. Read this before modifying or implementing features.

---

## 1. Project Overview
The **Tanbre Watch** is a custom wearable security testing and diagnostic tool built on top of the **Seeed Studio XIAO ESP32-S3** microcontroller.
* **Base Firmware**: Forked and customized from **Bruce** (which is based on ESP32 Marauder).
* **Target Environment**: Standard Seeed Studio XIAO ESP32-S3 (11 physical GPIOs).
* **Display Size**: 128x64 OLED display running via I2C.

---

## 2. Hardware Architecture & Pin Map
This hardware revision only populates the Seeed Studio XIAO ESP32-S3 module, a 128x64 I2C OLED display, and the battery power path. NRF24, microSD, GPS, IR, haptic motor, microphone, and physical buttons are not populated and must not reserve GPIOs in firmware.

### Active Pin Map

| Physical Pin | GPIO | Connected Device / Module | Protocol / Pin Role |
| :--- | :--- | :--- | :--- |
| **D4** | GPIO 5 | **OLED SDA** | I2C Data |
| **D5** | GPIO 6 | **OLED SCL** | I2C Clock |

All other exposed GPIOs are currently unassigned in the Tanbre firmware profile.

---

## 3. Important Software Entry Points

### Hardware Config & Pins
* **Pins Definition**: Defined in the board-specific header: `firmware/boards/tanbre-watch/pins_arduino.h`.
* **Hardware Initialization**: Handled in `firmware/boards/tanbre-watch/interface.cpp` (configures I2C and display).
* **Configuration Manager**: Managed by `BruceConfigPins` in `firmware/src/core/configPins.h` and `configPins.cpp`. Custom settings are saved to `/brucePins.conf` on LittleFS.

### UI & Display
* **Display Engine**: Driven by **LovyanGFX** configured with `Panel_SH110x` or similar SSD1306/SH1106 driver.
* **Display Constraints**: UI rendering is optimized for **128x64 pixels**. Text, lists, and charts must fit this layout.
* **Menus**: The primary application menu is defined in `firmware/src/core/main_menu.cpp`.

---

## 4. Development & Coding Rules

### Concurrency and Thread Safety
* The BLE scanner (`BLE_Suite` / `native_detector`) runs asynchronously on the NimBLE stack task thread. 
* **Crucial**: Any shared data structures (e.g. lists or vectors of scanned devices) accessed by both the NimBLE scan callback and the main display loop thread **MUST** be protected using a FreeRTOS Semaphore Mutex (`xSemaphoreTake` / `xSemaphoreGive`) to prevent race conditions, memory corruption, and kernel panics.

### Memory & Heap Management
* The ESP32-S3 has PSRAM, but heap allocation on the internal RAM must still be managed carefully.
* Dynamic lists (like detected BLE/WiFi devices) should have a hard safety cap (e.g., max 100 entries) to prevent heap exhaustion.

---

## 5. Build & Flash Commands
The project is built using PlatformIO.

* **Build**:
  ```bash
  pio run -e tanbre-watch
  ```
* **Upload/Flash**:
  ```bash
  pio run -t upload -e tanbre-watch --upload-port /dev/ttyACM0
  ```
* **Monitor Serial**:
  ```bash
  pio device monitor -b 115200
  ```
