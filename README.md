<div align="center">

# OTA-Bot-Mochi
### Dasai-Inspired ESP32-S3 Robot Companion with Living Vector Eyes

An expressive smart companion firmware for ST7789 displays (240x280) and the
Waveshare 1.8 inch AMOLED panel (368x448). Combines lively Dasai/Cozmo-style
vector eyes and IMU physics with ChronosESP32 BLE features: live clock,
turn-by-turn navigation, weather, and notifications.

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

* **Living Eyes Companion:** 4 eye styles (`MOCHI`, `BLOUB`, `NORMAL`, `TRAPEZ`) and 88 expressive facial moods with dirty-rect redrawing, every one of them verified native against all four styles.
* **Automatic Faces:** Optional timed random expressions and sustained-motion IMU reactions, with cooldowns and automatic restoration of the selected mood.
* **Expressive Motion and Audio:** Soft face handoffs and gaze, double blinks/winks/nods, and optional non-blocking buzzer chirps with night mute and separate categories. English/Italian UI and [controls](docs/SOUND.md).
* **Quiet IMU Reactivity:** Mount-relative, damped gaze clamped to ±14 px with a noise dead zone; sustained lateral load keeps the eyes displaced through a bend and rolls the pair by up to 4 px. Optional deliberate shake reactions require repeated opposing impulses, not a single bump.
* **BLE Turn-by-Turn Navigation:** Receives live route directions, distance, ETA, and direction icons directly from your smartphone.
* **Chronos BLE Core:** Background BLE service managing phone notifications, incoming call rings, time synchronization, and weather forecasts.
* **Modular Screen State Machine:** Eyes, Clock, Navigation, Weather, Alerts, and Settings.
* **Contextual Views:** Large/detail route layouts and a Clock desk-tools menu with large clock, overview, silent timer/Pomodoro, and device status. Extra tools do not lengthen the main carousel; [details](docs/DESK.md).
* **Buffered Display:** Frames are composed in RAM and sent to the LCD at up to 30 FPS. BLE and touch polling continue between frames.
* **Swipe Navigation:** Swipe left for the next screen or right for the previous screen. Navigation wraps around at either end.
* **Companion Cards:** Dark rounded utility cards with a shared eye-color palette, bounded text, screen position dots, and clear empty states.
* **Dashboard First:** MOCHI eyes by default, calmer idle motion, no unsolicited full-screen calls, and no notification/caller payload logging.
* **Automatic Event Pages:** New notifications open Alerts; starting phone navigation opens Route. Regular navigation updates do not override a later manual page change.
* **Saved Preferences:** Parked-use touch menu and USB commands for eye shapes, colors, moods, day/night brightness, and behavior. Flash writes require an explicit save.
* **Backlight Controls:** Manual low-brightness night mode and optional idle dimming, without stopping BLE.

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
* **CLOCK:** Synced time/date, 12/24-hour mode, waiting state, and BLE indicator. Battery charge is estimated best-effort and reads NOT MEASURED without plausible hardware.
* **NAVIGATION:** Turn directions, speed, distance, and turn icon CRC tracking.
* **WEATHER:** City, temperature, condition icon, up to three high/low forecasts, receipt time, and stale-data indication.
* **NOTIFICATION:** Phone notifications and incoming call (ringer) display.
* **SETTINGS:** LOOK, LIGHT, DRIVE, MOVE, and AUDIO tabs with larger touch targets and vertical row scrolling. Hold still on Eyes for 1.2 seconds to open the menu; tap SAVE to persist changes. Use only while parked.

---

## Supported Hardware

Default target is the **Waveshare ESP32-S3-Touch-LCD-1.69 OLD** (240x280 ST7789).
The mappings in `pin_config.h` follow the official Waveshare schematic for that
board revision; dashboard settings leave them unchanged:

| Category | Component | Default Pins |
| :--- | :--- | :--- |
| **Display** | ST7789 SPI LCD (240x280) | `DC=4, CS=5, SCK=6, MOSI=7, RST=8, BL=15` |
| **Touch** | Auto-probed CST816 / FT6X36 family | `SDA=11, SCL=10, RST=13, INT=14` |
| **Motion** | QMI8658C 6-Axis IMU (shared I2C bus) | `SDA=11, SCL=10` |
| **RTC / Time** | External I2C RTC (shared I2C bus) | `SDA=11, SCL=10, INT=41` |
| **Buzzer** | PWM/tone output; electrical model unverified | `BUZZER=33` |
| **Backlight** | GPIO/PWM | `BL=15` |
| **Battery / Power** | Circuit and sensing path not yet verified | `BAT_ADC=1` sampled best-effort with plausibility gating; `SYS_EN=35, SYS_OUT=36` remain unused |

Touch probes the CST816 family at `0x15` and FT6X36 family at `0x38`; the startup
log reports the responding address. This is not an exact chip-model identification.
Do not assume that an AXP2101 exists on this board. Battery percentage uses a
guessed 2:1 divider ratio on `BAT_ADC` with a plausibility window, so missing
hardware reports NOT MEASURED instead of a wrong value; the ratio and the
percentages are not physically validated yet.

Building with `-DMOCHI_BOARD_AMOLED_18` selects the **Waveshare
ESP32-S3-Touch-AMOLED-1.8 V2** profile instead (CO5300 368x448 AMOLED over QSPI,
CST820 touch, brightness as a controller command, no buzzer pad). Its pin table,
compile commands, and the hardware checks still pending are listed in
[Boards and panels](docs/BOARDS.md).

---

## Quick Start

### Prerequisites

* Arduino IDE 2.x with the official **ESP32 by Espressif** board package installed.
* Libraries needed (install via Arduino Library Manager):
  * `GFX Library for Arduino` (provides `Arduino_GFX_Library.h`)
  * `ChronosESP32`
* Smartphone with the **Chronos** companion app installed (Android / iOS).

### Setup & Flashing

1. Clone this repository into your Arduino sketchbook folder:
   ```bash
   git clone https://github.com/camiu01/ota-bot-mochi.git
   cd ota-bot-mochi
   ```

2. Open `OTA-bot-Mochi/OTA-bot-Mochi.ino` in Arduino IDE.
3. Select your board configuration:
  * **Board:** `ESP32S3 Dev Module`
4. Upload to the device.
5. Launch the **Chronos** app on your phone, search for Bluetooth devices, and pair with **"OTA bot"**.

See [build and test instructions](docs/TESTING.md) for pinned dependencies,
Arduino CLI commands, Linux serial permissions, and native regression tests.
See [UI and controls](docs/UI.md) for screen behavior and hardware limits.
See [dashboard settings](docs/SETTINGS.md) for touch controls, USB commands, and saving.
The new utility screens have native regression coverage; firmware compilation,
upload, and physical validation remain with the user.

---

## Eyes Engine & IMU Interaction

The eye rendering core supports continuous frame updates, idle blinking, and full palette remapping.

### Styles & Geometries

* `MOCHI`: Tall solid capsules, no pupils or highlights (default firmware profile, Dasai aesthetic).
* `BLOUB`: Wide, soft rounded eyes with pupils and highlights (the standalone `eyes_init()` API default).
* `TRAPEZ`: Solid mirrored trapezoids, no pupils or highlights, with bounded top slopes (Cozmo look).
* `NORMAL`: Classic oval eyes.

### Key API Calls

```cpp
eyes_init(gfx);                                // Bind display and set default state
eyes_set_style(MOCHI);                         // Change eye geometry
eyes_set_expression(HAPPY);                    // Set emotional mood
eyes_set_expression_by_name("love");           // Set mood by string identifier
eyes_set_eye_colors(RGB565_BLACK, RGB565_CYAN, RGB565_BLACK, RGB565_WHITE);
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
