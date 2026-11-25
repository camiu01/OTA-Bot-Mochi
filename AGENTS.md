# Repository guidance

## Target and build

Arduino C++ firmware, not a desktop application. The sketch is
`OTA-bot-Mochi/OTA-bot-Mochi.ino`; Arduino compiles `src/` recursively.

| Profile | Board | Panel | PSRAM |
| --- | --- | --- | --- |
| default (`MOCHI_BOARD_LCD_169`) | Waveshare ESP32-S3-Touch-LCD-1.69 **OLD** | ST7789 240x280 | **disabled** |
| `MOCHI_BOARD_AMOLED_18` | Waveshare ESP32-S3-Touch-AMOLED-1.8 **V2** | CO5300 368x448 | **OPI** |

- `pin_config.h` follows the official schematics. Touch, RTC and IMU share one
  I2C bus driven through `src/bus/i2c_bus.*` (LCD SDA11/SCL10, AMOLED
  SDA15/SCL14); never open a second controller on those pads. LCD buzzer is
  GPIO33 (new revision: GPIO42).
- Never enable OPI PSRAM on the LCD OLD: it claims GPIO33/35/36 (buzzer and
  power control); that build beeped continuously and crashed in the I2C driver
  on 2026-10-13. The AMOLED canvas needs PSRAM=opi.
- Battery/charging circuitry is not verified; per-board validation gaps live in
  `docs/BOARDS.md` and `TODO.md`.

Native checks, from the repository root:

```powershell
.\scripts\test-host.ps1
git -c core.whitespace=cr-at-eol diff --check
```

```sh
bash scripts/test-host.sh
git diff --check
```

The runners need `g++` (C++17, `-Wall -Wextra -Werror`) and take about ten
minutes. `test-host.sh` prints "All host tests passed" even after a failed
assertion: check its exit code and search the output for `Assertion`. Suites
other than `tests/swipe_test.cpp` need the shared source lists in the runners;
keep both runners aligned. Raster fixtures in `tests/fixtures/` are re-recorded
with `-DRECORD_FACE_BASELINE` using the runner's exact flags. Native stubs do not
verify BLE, panels, touch coordinates or electrical behavior.

Compile or upload firmware only when the user asks. Reference builds:

```sh
# LCD 1.69 OLD (default)
arduino-cli compile --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=disabled,FlashSize=16M" \
  --build-property "upload.maximum_size=6553600" \
  --build-property "compiler.cpp.extra_flags=-DMOCHI_BOARD_LCD_169" OTA-bot-Mochi

# AMOLED 1.8 V2
arduino-cli compile --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=opi,FlashSize=16M" \
  --build-property "upload.maximum_size=6553600" \
  --build-property "compiler.cpp.extra_flags=-DMOCHI_BOARD_AMOLED_18" OTA-bot-Mochi
```

`platformio.ini` mirrors them as `esp32s3-lcd` (default) and `esp32s3-amoled`;
CI builds only the Arduino CLI path. Both profiles use `partitions.csv` (16 MB
OTA layout v1); the size override matches each 6.25 MiB slot. Moving from the old
layout needs a user-authorized USB install. CI pins Arduino CLI 1.2.0, ESP32 core
3.3.12, ChronosESP32 1.9.1, GFX Library for Arduino 1.6.9, ESP32Time 2.0.6 and
NimBLE-Arduino 2.5.2; `.github/workflows/checks.yml` gates firmware builds on the
native tests and attaches binaries to releases, without OTA deployment.

## Runtime architecture

- **Main loop.** The sketch owns hardware init and the loop. BLE/USB/touch,
  timers, sync, sound and brightness run between bounded 33 ms frames; an
  `Arduino_Canvas` composes each frame before `flush()`.
- **Callbacks.** Chronos, ANCS, OBD and network callbacks only fill bounded,
  mutex-protected snapshots or counters (`src/ui/ui_state.*`, `obd_state.h`).
  Never draw, play sound, write preferences or issue sync from a callback.
- **Pages.** `screens.*` has eight swipe destinations: Eyes, Clock, Music,
  Route, Alerts, Weather, Drive, Settings. Desk tools and games are children of
  Clock; Drive tools are children of Drive. Eyes is home: idle pages return to it
  after `home_seconds` (Drive, active route/timer, calls and unsaved Settings
  stay). New alerts/routes auto-select a page, alerts return after
  `alert_seconds`, and switching waits while a finger is down.
- **Eyes.** Configured expression/style is separate from temporary face policy,
  visual-life pose, geometry and dirty-region rasterization. MOCHI and TRAPEZ stay
  solid and pupil-free; BLOUB has pupils/highlights. Differentiate moods by shape
  and motion, not color alone. Reactions may fade to temporary colors, then
  restore the selected ink. The geometry source is authoritative;
  `src/eyes/README.md` has legacy geometry.
- **Settings.** Changes update RAM and bump a renderer revision; only explicit
  SAVE writes NVS. Profile v12 is 25 bytes: v11 layout plus byte 16 max RPM in
  hundreds (30–120), bytes 17–20 OBD slots (ObdValue 0–16), 21 alert seconds, 22
  back-to-Eyes seconds (0 = never), 23 gauge style (0–3), 24 checksum. Keep
  decoding v10/v11 (17 bytes; byte 15 bit 1 = OBD scan), v9 (16), v5–v8 (13), v4
  (12) and v1–v3 (8). Route overrides, timers, counters and games are RAM-only.
- **UI.** `ui_theme.*` owns the minimal dark palette and primitives (`ui_card`,
  `ui_box`, `ui_button`, `ui_bar`, `ui_center`, `ui_status`); the accent is the
  eye color. `i18n.*` owns EN/IT labels; on-device labels are uppercase, USB
  vocabulary stays English and phone text is not translated.
- **Touch.** Dedicated I2C probing plus portable gesture policies: released
  taps, stationary holds, horizontal page swipes and vertical scrolling are
  separate; a gesture never also edits a setting.
- **Sound.** Backlight and audio use distinct LEDC channels/timers in
  `pwm_channels.h`. Sound defaults off, night mute on; no blocking tone loops.
  AMOLED charge-start cues read the PMU on the shared bus without configuring
  charging; SD reaction WAVs fall back to `face.wav`, then a generated chirp.
- **Vehicle.** `src/vehicle` is the optional BLE ELM327 client: a priority-1
  worker scans, connects and polls only while a Drive page is visible and OBD
  scan is on, remembers the last dongle in NVS namespace `mochi-obd`, and its
  notify callback only queues bytes. `elm327.h` (framing, PIDs, sequence) is
  host-tested; `obd_ble.cpp` gets a syntax check only.
- **Network.** Weather keeps phone data and optionally uses Wi-Fi/Open-Meteo.
  `src/network` owns reconnection, main-task NTP and a low-priority HTTPS worker.
  Credentials use their own NVS record and never appear in logs or API replies.
  See `docs/NETWORK.md`.

## Project conventions

- Follow `CONTRIBUTING.md`: tabs, functions under 50 executable lines, files
  under 400 lines, English documentation, Doxygen API comments.
- No `delay()` or `Serial` inside `src/eyes`; use non-blocking timestamps.
- Commits and PRs use English Conventional Commits, with no `Co-Authored-By`,
  session links or "generated with" lines.
- Behavior docs: `docs/UI.md`, `docs/SETTINGS.md`, `docs/DESK.md`,
  `docs/SOUND.md`. Track unverified hardware/CI work in `TODO.md`; never mark
  physical validation done from native mocks.
- README, TODO, CHANGELOG and `docs/` follow
  [CONTRIBUTING.md](CONTRIBUTING.md#documentation-conventions); read it before
  editing Markdown.
