<div align="center">

# OTA-Bot-Mochi
### ESP32-S3 Watch UI with Living Robot Eyes

A ChronosESP32 smartwatch UI for ST7789 240x280 with vector eyes inspired by bloub and Dasai Mochi.

[![License: GPLv3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![Arduino](https://img.shields.io/badge/Arduino-IDE_2.x-teal.svg)](https://www.arduino.cc/)
[![ESP32-S3](https://img.shields.io/badge/ESP32--S3-Touch--LCD--1.69-red.svg)](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.69)
[![Status](https://img.shields.io/badge/Status-Active-success.svg)](#)

</div>

---

## Overview

OTA-Bot-Mochi is a smartwatch interface for ESP32-S3 driving an ST7789 display. It combines ChronosESP32 BLE services (notifications, calls, weather, navigation) with a non-blocking Eyes Engine rendering white vector eyes on black.

Default target is Waveshare ESP32-S3-Touch-LCD-1.69. All pins live in `pin_config.h` and are fully remappable.

### Key Highlights

* **Eyes Engine:** Four geometries (`NORMAL`, `MOCHI`, `BLOUB`, `TRAPEZ`) with idle float, saccades, and blink.
* **Expressive Faces:** 23 Cozmo-style expressions from `NEUTRAL` to `SAD_UP` with dirty-rect redraw.
* **IMU Reactive:** QMI8658C tilt drives pupils; shake forces `DIZZY` for 2s.
* **Chronos BLE Stack:** Push notifications, ringer, connection, weather, and navigation callbacks.
* **Watch Screens:** Base, Clock, Weather, Navigation, and Notification views over Arduino GFX.

---

## Supported Hardware

| Category | Component | Default | Pins |
| :--- | :--- | :--- | :--- |
| **Display** | LCD ST7789 240x280 SPI | Waveshare 1.69 inch | `DC=4, CS=5, SCK=6, MOSI=7, RST=8, BL=15` |
| **Touch** | FT6X36 capacitive | I2C shared bus | `RST=13, INT=14, SDA=11, SCL=10` |
| **Motion** | IMU QMI8658C | Dedicated I2C | `SDA=12, SCL=9` |
| **Time** | RTC | I2C + interrupt | `SDA=11, SCL=10, INT=41` |
| **Audio** | Active buzzer | GPIO drive | `BUZZER=34` |
| **Power** | System and battery | AXP2101 board | `SYS_EN=35, SYS_OUT=36, BAT_ADC=1` |
| **COM** | USB native, UART0 | Debug and flashing | `USB_DN=20, USB_DP=19, TX=43, RX=44` |

---

## Quick Start

### Prerequisites

* Arduino IDE 2.x with ESP32 core by Espressif
* Libraries: `Arduino_GFX_Library`, `ChronosESP32`
* Board: Waveshare ESP32-S3-Touch-LCD-1.69

### Workflow

1. **Install dependencies:**
```cpp
// Library Manager: install Arduino_GFX_Library + ChronosESP32
// Boards Manager: install esp32 by Espressif
```

2. **Open the project:**
```cpp
// Open main.ino in Arduino IDE
// Tabs auto-load: pin_config.h, gfx_helper.*, images.h, src/eyes/*
```

3. **Configure board and pins:**
```cpp
// Tools: Board ESP32S3 Dev Module, USB CDC On Boot Enabled
// Edit pin_config.h only if your wiring differs
#include "pin_config.h"
```

4. **Flash and monitor:**
```cpp
eyes_init(gfx);
eyes_imu_init();
// Upload main.ino, open Serial Monitor at 115200
```

5. **Pair with companion app:**
```cpp
ChronosESP32 watch("OTA bot");
// Pair via Chronos app, then in loop:
eyes_update(millis());
```

---

## Eyes Engine

Styles: `NORMAL` (classic ovals), `MOCHI` (tall rounded Dasai look), `BLOUB` (wide bloub blobs, default), `TRAPEZ` (corner-lifted trapezoid, slant pulls high-left up).

| Expression | Mood | Expression | Mood |
| :--- | :--- | :--- | :--- |
| `NEUTRAL` | Idle stare | `HAPPY` | Arched cheer |
| `SAD` | Drooped lids | `ANGRY` | Slanted lids |
| `SURPRISED` | Wide open | `SLEEPY` | Half closed |
| `LOVE` | Heart pupils | `DIZZY` | Crossed wobble |
| `GLEE` | Joyful arcs | `WORRIED` | Raised inner lids |
| `FOCUSED` | Intense bars | `ANNOYED` | Half-lid flat |
| `SKEPTIC` | Big/small mix | `FRUSTRATED` | Angled asym |
| `UNIMPRESSED` | Flat half-mast | `SUSPICIOUS` | High/low mix |
| `SQUINT` | Narrow slits | `FURIOUS` | Thick glare |
| `SCARED` | Small tall | `AWE` | Huge round |
| `SLEEPY_EYES` | Heavy droop | `SAD_DOWN` | Downcast sad |
| `SAD_UP` | Upcast sad | — | — |

IMU behavior: accelerometer tilt offsets pupils in pixels; sudden shake calls `eyes_imu_trigger_shake()` and locks `DIZZY` for 2s before restoring the saved expression.

| Function | Description |
| :--- | :--- |
| `eyes_init(gfx)` | Bind display, reset to BLOUB NEUTRAL |
| `eyes_set_style(s)` | Switch NORMAL MOCHI BLOUB TRAPEZ |
| `eyes_set_expression(e)` | Switch emotional face by enum |
| `eyes_set_expression_by_name(n)` | Switch face by name, true on match |
| `eyes_set_slant(v)` | TRAPEZ corner-lift offset in px |
| `eyes_set_lid(v)` | Eyelid line offset in px |
| `eyes_set_asym(v)` | Toggle per-eye asymmetric geometry |
| `eyes_set_eye_colors(bg, fg, pupil, hl)` | Set full palette in one call |
| `eyes_set_colors(bg, fg)` | Set bg + fg only |
| `eyes_set_bg/fg/pupil/highlight(c)` | Set single palette channel |
| `eyes_get_colors(bg, fg, pupil, hl)` | Bulk getters, nullable pointers |
| `eyes_get_bg/fg/pupil/highlight()` | Single getters |
| `eyes_update(now)` | Saccades, blink, IMU, redraw |
| `eyes_imu_poll(now)` | Tilt pupils, shake to DIZZY |

---

## Customizing Eyes

* Change style at runtime: `eyes_set_style(TRAPEZ);`
* Change mood: `eyes_set_expression(HAPPY);` or `eyes_set_expression_by_name("glee");`
* Recolor: `eyes_set_colors(BLACK, WHITE);` or full palette below.
* Tune geometry via `eyes_config()` fields `eyeW`, `eyeH`, `eyeGap`, `radius`, `slant`, `lidTop`, `asym`.
* Tune UI palette in `gfx_helper.h`: `BACKGROUND`, `TEXT_COLOR`, `ELEMENT_COLOR`, `MARGIN`.
* Swap splash and icons in `images.h`; font is `FreeSansBold12pt7b.h`.

```cpp
// Cozmo cyan + TRAPEZ: cyan eyes, black pupils, white highlight
eyes_set_style(TRAPEZ);
eyes_set_eye_colors(BLACK, CYAN, BLACK, WHITE);
eyes_set_expression_by_name("awe");
```

Engine sources live only in `src/eyes/` (Arduino IDE 2.x compiles `*.ino` + root + `src/` recursive).
Canonical build path is `#include "src/eyes/eyes_api.h"`.
`pin_config.h` is reached from `src/eyes/*.cpp` via `../../pin_config.h`.

---

## Contributing & Governance

Contributions are welcome. Please read the governance docs before opening issues or pull requests.

* **[LICENSE](LICENSE)** - Defines GPLv3 reuse, modification, and redistribution terms.
* **[CONTRIBUTING.md](CONTRIBUTING.md)** - Describes workflow, code style, and pull request requirements.
* **[CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)** - Sets expected behavior and reporting process for participants.
* **[SECURITY.md](SECURITY.md)** - Explains supported versions and vulnerability disclosure process.

---

## License & Authors

Copyright (c) 2026 [Camiu](https://github.com/camiu01). Distributed under the [GNU General Public License v3.0 (GPLv3)](LICENSE). Built on Arduino GFX and ChronosESP32, inspired by bloub eyes and Dasai Mochi.
