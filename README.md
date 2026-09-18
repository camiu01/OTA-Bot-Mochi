<div align="center">

# OTA-Bot-Mochi
### Dasai-Inspired ESP32-S3 Robot Companion with Living Vector Eyes

An expressive smart companion firmware for ST7789 displays (240x280). Combines lively Dasai/Cozmo-style vector eyes and IMU physics with ChronosESP32 BLE features: live clock, turn-by-turn navigation, weather, and notifications.

[![License: GPLv3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![Arduino](https://img.shields.io/badge/Arduino-IDE_2.x-teal.svg)](https://www.arduino.cc/)
[![ESP32-S3](https://img.shields.io/badge/ESP32--S3-Touch--LCD--1.69-red.svg)](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.69)
[![Status](https://img.shields.io/badge/Status-Active-success.svg)](#)

</div>

---

## Overview

**OTA-Bot-Mochi** turns an ESP32-S3 display into an interactive desktop or car companion inspired by Dasai Mochi and Cozmo. 

Instead of a generic smartwatch interface, the core experience is the **Living Eyes Base Screen**: dynamic vector eyes with realistic saccades, idle floats, blinking, and physical reactivity to acceleration or shaking. When paired to your phone via BLE with the Chronos app, it seamlessly extends into a useful utility companion offering turn-by-turn navigation data, real-time clock syncing, weather forecasts, and phone alerts.

### Key Highlights

* **Living Eyes Companion:** 4 eye styles (`MOCHI`, `BLOUB`, `NORMAL`, `TRAPEZ`) and 23 expressive facial moods with dirty-rect redrawing.
* **IMU Motion Reactivity:** Integrated QMI8658C accelerometer dynamically shifts pupils with g-force and triggers a 2-second `DIZZY` state on hard shakes.
* **BLE Turn-by-Turn Navigation:** Receives live route directions, distance, ETA, and direction icons directly from your smartphone.
* **Chronos BLE Core:** Background BLE service managing phone notifications, incoming call rings, time synchronization, and weather forecasts.
* **Modular Screen State Machine:** Built-in UI screens for `BASE` (Eyes), `CLOCK`, `NAVIGATION`, `WEATHER`, and `NOTIFICATION`.

---

## Architecture & Screens

The runtime loop handles BLE updates alongside a clean screen dispatcher:


```text
                  ┌──────────────────────┐
                  │       setup()        │
                  │ Chronos + GFX + IMU  │
                  └──────────┬───────────┘
                             │
                             ▼
                  ┌──────────────────────┐
                  │    current_screen    │
                  └──────────┬───────────┘
        ┌────────────┬───────┼───────────┬────────────┐
        ▼            ▼       ▼           ▼            ▼
     [BASE]       [CLOCK]  [NAV]     [WEATHER]  [NOTIFICATION]
  Living Eyes      Time     Route    Forecast       Alerts
 (IMU Physics)     Sync   & Icons     & Temps      & Callers

```

* **BASE:** Non-blocking rendering loop updating eye saccades, blinks, and IMU-driven pupil offsets.
* **CLOCK:** Synced digital clock face with battery and BLE indicators.
* **NAVIGATION:** Turn directions, speed, distance, and turn icon CRC tracking.
* **WEATHER:** Forecast counts, conditions, temperatures, UV index, and atmospheric pressure.
* **NOTIFICATION:** Phone notifications and incoming call (ringer) display.

---

## Supported Hardware

Default target is the **Waveshare ESP32-S3-Touch-LCD-1.69** (240x280 ST7789). Pinouts are mapped in `pin_config.h`:

| Category | Component | Default Pins |
| :--- | :--- | :--- |
| **Display** | ST7789 SPI LCD (240x280) | `DC=4, CS=5, SCK=6, MOSI=7, RST=8, BL=15` |
| **Touch** | FT6X36 Capacitive I2C | `SDA=11, SCL=10, RST=13, INT=14` |
| **Motion** | QMI8658C 6-Axis IMU | `SDA=12, SCL=9` |
| **RTC / Time** | External I2C RTC | `SDA=11, SCL=10, INT=41` |
| **Buzzer** | Active buzzer GPIO | `BUZZER=34` |
| **Power** | AXP2101 PMIC & Battery | `SYS_EN=35, SYS_OUT=36, BAT_ADC=1` |

---

## Quick Start

### Prerequisites

* Arduino IDE 2.x with the official **ESP32 by Espressif** board package installed.
* Libraries needed (install via Arduino Library Manager):
  * `Arduino_GFX_Library`
  * `ChronosESP32`
* Smartphone with the **Chronos** companion app installed (Android / iOS).

### Setup & Flashing

1. Clone this repository into your Arduino sketchbook folder:
   ```bash
   git clone [https://github.com/camiu01/ota-bot-mochi.git](https://github.com/camiu01/ota-bot-mochi.git)
  

2. Open `OTA-bot-Mochi/OTA-bot-Mochi.ino` in Arduino IDE.
3. Select your board configuration:
  * **Board:** `ESP32S3 Dev Module`
4. Upload to the device.
5. Launch the **Chronos** app on your phone, search for Bluetooth devices, and pair with **"OTA bot"**.

---

## Eyes Engine & IMU Interaction

The eye rendering core supports continuous frame updates, idle blinking, and full palette remapping.

### Styles & Geometries

* `MOCHI`: Tall, cute rounded rectangles (Dasai aesthetic).
* `BLOUB`: Wide, expressive blob eyes (default).
* `TRAPEZ`: Slanted trapezoid geometry with corner-lift tuning (Cozmo look).
* `NORMAL`: Classic oval eyes.

### Key API Calls

```cpp
eyes_init(gfx);                                // Bind display and set default state
eyes_set_style(MOCHI);                         // Change eye geometry
eyes_set_expression(HAPPY);                    // Set emotional mood
eyes_set_expression_by_name("love");           // Set mood by string identifier
eyes_set_eye_colors(BLACK, CYAN, BLACK, WHITE);// Set bg, eye, pupil, highlight colors
eyes_imu_poll(millis());                       // Read IMU tilt & shake
eyes_update(millis());                         // Run animations, physics, and draw

```

---

## Contributing & Governance

Contributions are welcome. Please read the governance docs before opening issues or pull requests.


* **[LICENSE](LICENSE)** - Defines GPLv3 reuse, modification, and redistribution terms.
* **[CONTRIBUTING.md](CONTRIBUTING.md)** - Workflow and code style guidelines.
* **[CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)** - Community expectations.
* **[SECURITY.md](SECURITY.md)** - Vulnerability reporting guidelines.

---

## License & Credits

Copyright (c) 2026 [Camiu](https://github.com/camiu01). Distributed under the [GNU General Public License v3.0 (GPLv3)](LICENSE). Built on Arduino GFX and ChronosESP32, inspired by bloub eyes and Dasai Mochi.
