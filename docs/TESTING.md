# Build and test

## Native regression tests

These tests use `g++` with C++17 and test doubles for the display, Chronos, I2C,
FreeRTOS mutex, USB serial input, and preferences. They do **not** compile ESP32 firmware, open a serial port,
or replace firmware on a connected device.

From the repository root:

```powershell
# Windows, with g++ in PATH (for example MSYS2 MinGW64)
.\scripts\test-host.ps1
```

```sh
# Linux or macOS, with g++ installed
bash scripts/test-host.sh
```

Checks cover horizontal gestures, line wrapping, bounded messages, text clipping,
empty states, clock synchronization and changed-field rendering, incoming calls,
weather freshness, completed navigation icons, disconnect handling, tap/hold
isolation, night-safe idle dimming, timestamp rollover, IMU setup/scaling,
mount-relative gaze, bump rejection, shake opt-in/cooldown/restoration, preference
validation/save failures, USB command limits, automatic notification/route pages,
preservation of manual page choices on route refresh, and touch-delayed switching.
The real eye renderer also runs against a native raster substitute: all four
styles, all 88 expressions, and both blink states (704 combinations), a proof
that every expression renders differently from NEUTRAL on every style, that no
two names draw the same pixels on any style (pairwise uniqueness, 15312
comparisons), solid-style pupil/highlight exclusion, trapezoid symmetry, IMU
gaze/roll bounds, radius limits, and dirty-region erasure are checked. The
fixed accent palette has its own suite: no accent may equal black, white, or the
counted pupil/highlight sentinels, every accented expression must paint its
accent on all four styles, plain faces must not leak one, and the petting heart
must use the love accent. Panel
adaptation has its own suite: on 240x280, 240x320, 320x480, 368x448, 128x160,
and 160x128 rasters the pair must stay centered, every expression must stay
inside the panel, and the fit must not merge two names into one picture. Pages
and menus are rebuilt per panel as well: the same suite compiled for 240x280,
240x320, and 368x448 (through `-DLCD_HEIGHT` and `-DMOCHI_BOARD_AMOLED_18`)
must render every carousel and tool page without a text overflow, keep the tab,
row, grid, footer, desk-card, and timer taps landing on their widgets at the
scaled coordinates, and ignore the dead strip below the scaled design field on
a taller panel.
Automatic-face tests cover random timing,
restoration, sustained IMU thresholds, cooldowns, and version-one preference
migration.
Additional checks cover contextual tools without extra carousel pages, temporary
route layout versus saved defaults, version-two migration, local timer rollover/
pause/Pomodoro completion, and eye-tap/double-tap/glance/greeting reactions with
automation disabled. Expanded tests cover version-five thirteen-byte profiles with
version-four and older migration, the reboot verb, EN/IT
label completeness and layout, out-of-order weather packets/sync throttling,
animated dirty regions, sound notes/priorities/mute/rollover/failures, and actual
PWM adapters against core-2/core-3 shared-timer mocks.
Additional checks exercise all six shared color palettes, filled touch gutters,
row/tab/footer edges with row hits aligned to their visible cards, contiguous
grid-cell rows, 92-pixel desk hub cards, 50-pixel timer buttons, contextual
vertical-swipe targets, bounded
menu scrolling, vertical/horizontal
axis isolation, tap tolerance, explicit audio-test mute/category behavior,
randomized face-tone family pitch bands with the faces switch, the battery
voltage curve, divider plausibility window, and consecutive-sample trust policy,
plus the petting heart marker's dirty-region wipe and clamped placement.
Mocks do not verify physical LCD rendering, BLE protocol interoperability, physical
pin assignments, or electrical behavior.

## Host LCD emulator (SDL2)

`tools/run-emulator.sh` builds a native window that drives the same UI code the
tests link: real page routing, the production gesture policies, settings, sound
and backlight services, the USB command parser, and the real eye renderer, all
drawn into the 240x280 RGB565 test raster and scaled to a window.

```sh
# Linux or macOS, with g++, pkg-config, and libsdl2-dev (or SDL2-devel)
bash tools/run-emulator.sh
```

It is a preview aid, not a test: it is outside `scripts/test-host.sh`, needs no
board, and does not gate firmware CI. The display test double rasterizes the
firmware's built-in 5x7 bitmap font, so label metrics match the device's 6x8
cells. The window still does not prove backlight level, touch digitizer
accuracy, BLE content, or audio output.

Controls:

- The mouse acts as one contact: a quick press is a tap, a stationary 1.2 s
  press is a hold (opens Settings from Eyes), a drag swipes horizontally and
  scrolls vertically, and a wiggly vertical stroke pets the eyes.
- The mouse wheel flicks between carousel pages.
- `1`..`6` open Eyes, Clock, Route, Alerts, Weather, and Settings; `0` opens
  Desk tools.
- `n` injects a notification (the auto page policy jumps to Alerts), `r` starts
  or stops navigation, `p` writes `mochi-lcd.ppm`, `q` or `Esc` quits.
- `Enter` opens a USB console in the terminal, forwarding lines to the real
  command parser (`help`, `mood names`, `set mood derp`, `brightness 70`).

Headless runs need no display server and are handy for screenshots:

```sh
SDL_VIDEODRIVER=dummy bash tools/run-emulator.sh \
  --page weather --frames 120 --screenshot weather.ppm
```

`--page` accepts `eyes clock route alerts weather settings desk bigclock
overview timer device`, `--command` runs one USB command at startup, and
`--frames` exits after the given number of rendered frames.

`--size WIDTHxHEIGHT` builds a variant for another panel, for example
`--size 240x320` for a 1.8 inch panel, `--size 368x448` for the AMOLED board, or
`--size 128x160` for a 1.44 inch one.
The layout macros and the eye geometry both see that screen, so panel
adaptation can be previewed without editing `pin_config.h`:

```sh
SDL_VIDEODRIVER=dummy bash tools/run-emulator.sh --size 240x320 \
  --page eyes --frames 120 --screenshot eyes-18.ppm
```

Each size gets its own binary under `${TMPDIR:-/tmp}/mochi-emulator`, so
switching sizes does not force a rebuild of the default one. On panels taller
or wider than 240x280 the eye pair recenters; it grows only with the smaller
axis, so it can never overflow horizontally.

## Arduino CLI

Install Arduino CLI using its [official installation guide](https://arduino.github.io/arduino-cli/latest/installation/).
Arduino IDE also bundles the executable. On this Windows workstation it is:

```powershell
& "C:\Program Files\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe" version
```

If it is not in `PATH`, replace `arduino-cli` below with that full invocation.

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.11 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "ESP32Time@2.0.6" "NimBLE-Arduino@2.5.1"
arduino-cli lib install --no-deps "ChronosESP32@1.9.1" "GFX Library for Arduino@1.6.9"
arduino-cli board list
arduino-cli compile --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=disabled,FlashSize=16M" OTA-bot-Mochi
arduino-cli upload --port COM3 --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=disabled,FlashSize=16M" OTA-bot-Mochi
arduino-cli monitor --port COM3 --config baudrate=115200
```

The profile selects ESP32S3 Dev Module, native USB CDC on boot, no PSRAM,
and 16 MB flash for the Waveshare ESP32-S3-Touch-LCD-1.69 OLD reference board.
PSRAM must stay disabled on that OLD revision: OPI PSRAM claims the buzzer
(GPIO33) and power-control (GPIO35/GPIO36) pins and caused a continuous beep
plus I2C crashes in an earlier experiment.

The AMOLED board needs the second profile from
[Boards and panels](BOARDS.md); its frame canvas is too large for internal
SRAM, so it compiles with octal PSRAM enabled and the board macro:

```sh
arduino-cli compile --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=opi,FlashSize=16M" \
  --build-property "compiler.cpp.extra_flags=-DMOCHI_BOARD_AMOLED_18" OTA-bot-Mochi
```

CI compiles and publishes both profiles on every push; the AMOLED artifacts are
marked as hardware-unverified.
PSRAM stays disabled: the OLD revision routes the buzzer to GPIO33 and the
power-control lines SYS_EN/SYS_OUT to GPIO35/GPIO36, which the ESP32-S3 octal
PSRAM interface claims. A build with `PSRAM=opi` beeped the buzzer continuously
and crashed in the shared I2C driver on 2026-10-13. The 8 MB onboard PSRAM
therefore remains unused. Upload replaces existing firmware.
On Linux, replace `COM3` with the port reported by `board list`, often `/dev/ttyACM0`.

The core and library versions above were found installed on the workstation on
2026-10-13. They are reproducible dependency pins, **not a claim that the new
firmware has passed a build or upload**. Firmware compilation and upload are
reserved for the user. GitHub Actions includes a firmware build job for future
pushes; it has not been run during this change.
See [GitHub Actions workflow](CI.md) for cached builds and downloadable artifacts.

### Linux serial permissions

Check the owner/group with `ls -l /dev/ttyACM0`. On distributions using `dialout`:

```sh
sudo usermod -aG dialout "$USER"
```

Log out and back in before retrying. Some distributions use `uucp` instead.
Do not make the device world-writable. Close other serial monitors before upload.

## Device validation checklist

1. Confirm startup reports `UI ready`, a touch controller address, and `IMU ready`
   or an explicit missing-sensor diagnostic. Mochi must greet with a brief silent
   face that returns to the selected mood within three seconds. Touch, RTC, and the
   IMU share one I2C bus at SDA=11/SCL=10; if touch answers but `IMU unavailable`
   persists, suspect the sensor identity or wiring rather than the bus pins.
2. Swipe through all screens and test repeated/edge gestures. Swipe down on Clock
   to open Scrivania, down on Route to toggle the layout, down on Weather to
   request a sync, and down on Alerts or a desk tool to confirm the quick return
   to Eyes; confirm upward swipes walk back through Scrivania to Clock, and
   vertical strokes still pet on Eyes and still scroll rows in Settings.
   Tap the top status strip on Clock, Route, Alerts, Weather, or a desk tool to
   toggle sound: unmuting must chirp once, the strip must do nothing on Eyes and
   in Settings, the header speaker icon must follow the state, and a
   Settings SAVE must keep the new state across reboot. Send `reboot` over USB and
   confirm the acknowledgment followed by a restart.
3. Connect Chronos and verify the clock transitions from waiting to synchronized
   time. Test 12/24-hour mode, disconnect, and reconnect.
4. Send a long notification while Eyes is selected and confirm Alerts opens
   automatically. Check that app name, title, and message render at double
   scale, and check six-second message pagination with its page counter and
   accented text. The font approximates common lowercase Latin accents; other
   unsupported glyphs, including emoji, appear as `?`.
5. With dashboard mode on, ring the phone on each page. Confirm it does not cover
   Eyes, Clock, Route, Weather, or Settings. Select Alerts to see the call, then
   dismiss it. Check optional full-screen behavior only after explicitly turning
   dashboard mode off.
6. Sync weather and high/low forecasts. Verify stale state after three hours.
7. Start navigation on the phone and confirm Route opens automatically. Swipe
   away and confirm route/icon updates do not force it open again. A new
   notification should still open Alerts. End and restart navigation to check a
   fresh automatic switch. Check live instructions, distance, ETA, icon changes,
   and inactive state. An incomplete icon shows a neutral placeholder.
8. While parked, hold on Eyes for 1.2 seconds to open Settings. Check all tabs,
   live eye changes, explicit SAVE, USB commands, and preference persistence after
   reboot. Check night brightness and optional idle dimming. A wake swipe also
   changes screens. A swipe must not activate a settings row.
9. Power on while parked to establish the IMU mounting reference. Verify calm
   pupil motion and no reaction to a single bump with shake disabled. On the
   bench only, opt into shake, perform repeated opposing impulses, and confirm
   reaction, cooldown, and mood restoration. Disable shake for road use.
   Test missing sensors and allocation failure.
   Check MOCHI/TRAPEZ have no pupils/highlights, BLOUB is visibly wider, and
   TRAPEZ eyes are mirrored with rounded corners and no protruding spikes.
   Enable Random mood and check temporary expressions and restoration; enable
   IMU mood on the bench to test sustained X/Y/Z deflections rather than isolated
   bumps. Finally, drive a gentle curve and confirm the gaze holds through the bend
   and the pair leans; flip the sign in `eye_geometry.h` if the roll goes the wrong
   way for that mounting.
10. Run an extended BLE session and measure heap, power, and battery runtime.
11. Battery reading is best-effort on unverified hardware. Open Clock → DESK
    TOOLS → DEVICE STATUS with no battery fitted: the row must read NOT MEASURED
    and USB `show` must report `battery=unavailable`. With a known charged LiPo
    and the expected divider on `BAT_ADC`, compare the shown percentage against a
    multimeter across the discharge curve; do not treat the native conversion
    tests as physical validation.

Also test a downward swipe on Clock into Scrivania and all four tools. Verify
large clock legibility, timer continuation while viewing other pages,
default-silent completion, and reboot reset. Confirm every hub card and timer
button is an easy parked-use target, a downward swipe on Route toggles the
layout, a downward swipe on Weather requests a sync, and upward swipes walk
back from tools to Scrivania to Clock; other pages keep the downward-swipe
home to Eyes.
Switch Route layout temporarily, SAVE a different setting, and reboot: the saved
route default must win. Test `route default`, a Route hold, and LOOK → Route default.
A short eye tap must react for three seconds, glance toward the touch point,
and restore the selected mood; a quick double tap must show a playful face for
3.5 seconds; holds and swipes must trigger neither. Petting must keep the same
affectionate face while stroking continues, show the heart marker near the
finger on each recognized stroke, and end within six seconds of the
first stroke. On Eyes, confirm the corner speaker glyph appears only while
audio is off/night-muted and disappears after unmuting elsewhere. Flip LIGHT → Screen flip while parked: the interface, touch
targets, and menus must rotate together, tilting must still read in your frame,
SAVE plus reboot must keep it, and `set flip off` must restore the panel
orientation.

Enable sound only on the bench first. Check tone/level, category switches,
night mute, timer cues, and no backlight flicker while changing tone frequency.
Eye reactions must chirp in varied pitches inside their family band (low and slow
for sleepy faces, high bounce for surprised ones), not one fixed note.
Use AUDIO → Test sound or USB `sound test`, then `sound`, to distinguish mute
from PWM failure. PWM-ready status alone cannot establish physical audibility.
Check three-row scrolling, fixed tabs/actions, edge taps, and no value changes
during vertical gestures. Tap near the bottom edge of a visible row card: it
must change that row, not the one below. In the Mood grid, confirm five rows of
larger cells whose rows are fully tappable with only the column gutters inert.
Cycle every eye color and verify all utility pages
follow it, without losing text readability.
Switch EN/IT, SAVE, reboot, and check language persistence. Verify smooth face
handoffs and all idle gestures. For the reported weather failure, inspect USB
`weather` counters and try `weather sync` plus app weather configuration; do not
mark the physical weather issue resolved from native tests alone.

Battery percentage is a plausibility-gated best-effort estimate, deliberately
reported as NOT MEASURED when readings are implausible. Charging state remains
unavailable until the actual power circuit, ADC path, divider ratio, and
calibration are verified.
