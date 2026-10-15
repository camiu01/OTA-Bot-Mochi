# Repository guidance

## Target and build

This is an Arduino C++ firmware project, not a desktop application. Open
`OTA-bot-Mochi/OTA-bot-Mochi.ino`; Arduino compiles its `src/` modules recursively.
The default target is Waveshare **ESP32-S3-Touch-LCD-1.69 OLD**, ST7789, 240x280.
A second profile, selected with `-DMOCHI_BOARD_AMOLED_18`, targets the Waveshare
**ESP32-S3-Touch-AMOLED-1.8 V2**, CO5300 AMOLED, 368x448; its hardware details
are unverified and tracked in `docs/BOARDS.md` and `TODO.md`.
For the 1.69 board the numeric assignments in `pin_config.h` follow that board's
official Waveshare schematic: touch, RTC, and IMU share one I2C bus (SDA=11,
SCL=10) driven through `src/bus/i2c_bus.*`, and the buzzer is GPIO33 (new
revision: GPIO42). Preserve them and do not open a second I2C controller on the
shared pads; the AMOLED profile keeps the same single-bus rule on SDA15/SCL14.
Battery/charging circuitry is not verified.

From the repository root, native checks are:

```powershell
.\scripts\test-host.ps1
git -c core.whitespace=cr-at-eol diff --check
```

```sh
bash scripts/test-host.sh
git diff --check
```

The runners require `g++` in PATH and use C++17 with
`-Wall -Wextra -Werror`. They build temporary executables for swipe, UI, IMU,
settings, real eye rasterization, panel geometry, per-panel pages/menus,
audio, and PWM core-2/core-3 branches, plus a syntax check of the device-only
modules in the AMOLED profile. There is no separate lint command. For a single
independent gesture test:

```powershell
g++ -std=c++17 -Wall -Wextra -Werror tests/swipe_test.cpp -o "$env:TEMP\mochi-swipe.exe"
if ($LASTEXITCODE -ne 0) { throw "Swipe build failed" }
& "$env:TEMP\mochi-swipe.exe"
```

Other suites need the shared runtime source lists in the runners, not just the
test file. Keep both runners' source lists aligned when adding dependencies.
Native stubs do not verify real BLE, LCD, touch coordinates, or electrical behavior.

Firmware compilation/upload currently belongs to the user; do not perform it
unless explicitly requested. The reproducible build commands are:

```sh
# LCD board (default)
arduino-cli compile --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=disabled,FlashSize=16M" OTA-bot-Mochi

# AMOLED board (V2, hardware-unverified)
arduino-cli compile --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=opi,FlashSize=16M" \
  --build-property "compiler.cpp.extra_flags=-DMOCHI_BOARD_AMOLED_18" OTA-bot-Mochi
```

Do not enable OPI PSRAM on the 1.69 OLD board: that revision routes the buzzer
to GPIO33 and the power-control lines to GPIO35/GPIO36, which the octal PSRAM
interface claims on the ESP32-S3. That build beeped continuously and crashed
in the shared I2C driver on 2026-10-13; PSRAM must stay disabled there. The
AMOLED profile is the opposite case: its 368x448 canvas needs PSRAM=opi. CI pins
Arduino CLI 1.2.0,
ESP32 core 3.3.11, ChronosESP32 1.9.1, GFX Library for Arduino 1.6.9,
ESP32Time 2.0.6, and NimBLE-Arduino 2.5.1. Setup commands are in
`docs/TESTING.md`. `.github/workflows/checks.yml` gates firmware compilation
on native tests and provides downloadable binaries, not automatic OTA deployment.

## Runtime architecture

- The sketch owns hardware initialization and the main loop. BLE/USB/touch,
  timers, sync requests, sound, and brightness run between bounded 33ms display
  frames. An Arduino_Canvas composes complete frames before `flush()`.
- Chronos callbacks feed bounded, mutex-protected snapshots/event counters in
  `src/ui/ui_state.*`. Do not draw, play sound, write preferences, or issue app
  sync from a BLE callback. Main-task policies consume those events.
- `screens.*` dispatches six swipe destinations: Eyes, Clock, Route, Alerts,
  Weather, Settings. Desk tools are contextual children of Clock, not additional
  carousel entries. New alerts/navigation starts may auto-select a page; ordinary
  updates preserve manual selection, and switching defers while touch is held.
- The eye engine separates configured expression/style from temporary face
  policy, visual-life pose, geometry, and dirty-region rasterization. MOCHI and
  TRAPEZ must stay solid and pupil-free. BLOUB has pupils/highlights. The current
  geometry source is authoritative; `src/eyes/README.md` contains legacy geometry.
- Settings update RAM immediately, apply colors/brightness/eye preferences,
  and increment a renderer revision. Only explicit SAVE writes the NVS profile.
  Version five is thirteen bytes; preserve validated twelve-byte v4 and
  eight-byte v1–v3 migration.
  Session route overrides and local timer state are deliberately not persisted.
- `ui_theme.*` owns the shared palette/primitives; `i18n.*` owns built-in EN/IT
  labels. USB vocabulary remains English, and received phone content is not
  translated. Text is bounded and approximated for the built-in bitmap font.
- Touch uses dedicated I2C probing plus portable gesture policies. Separate
  released taps, stationary holds, horizontal page swipes, and vertical menu
  scrolling; a gesture must not also edit a setting.
- Backlight and optional audio use distinct LEDC channel/timer allocations in
  `pwm_channels.h`. Sound defaults off and night mute defaults on. Do not use
  blocking tone loops or share the backlight timer.
- Weather is pushed by the phone app, not fetched directly by firmware.
  `weather_sync.*` rate-limits main-task app synchronization. Packet diagnostics
  are counters only; host tests are not proof the reported device issue is fixed.

## Project conventions

Follow `CONTRIBUTING.md`: tabs, functions under 50 executable lines, cohesive
files under 400 lines, English documentation and Doxygen API comments.
Do not add `delay()` or `Serial` inside `src/eyes`; use non-blocking timestamps.
Commit titles and PR text use English Conventional Commits.

UI/settings/audio behavior is documented in `docs/UI.md`, `docs/SETTINGS.md`,
`docs/DESK.md`, and `docs/SOUND.md`. Track unverified hardware/CI work in `TODO.md`;
do not mark physical validation complete based on native mocks.
