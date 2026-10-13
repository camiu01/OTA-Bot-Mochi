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
styles, all 23 expressions, and both blink states (184 combinations), solid-style
pupil/highlight exclusion, trapezoid symmetry, radius limits, and dirty-region
erasure are checked. Automatic-face tests cover random timing, restoration,
sustained IMU thresholds, cooldowns, and version-one preference migration.
Additional checks cover contextual tools without extra carousel pages, temporary
route layout versus saved defaults, version-two migration, local timer rollover/
pause/Pomodoro completion, and eye-tap reactions with automation disabled.
Expanded tests cover version-four twelve-byte profiles/legacy migration, EN/IT
label completeness and layout, out-of-order weather packets/sync throttling,
animated dirty regions, sound notes/priorities/mute/rollover/failures, and actual
PWM adapters against core-2/core-3 shared-timer mocks.
Additional checks exercise all six shared color palettes, filled touch gutters,
row/tab/footer edges, bounded menu scrolling, vertical/horizontal axis isolation,
tap tolerance, and explicit audio-test mute/category behavior.
Mocks do not verify physical LCD rendering, BLE protocol interoperability, physical
pin assignments, or electrical behavior.

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
arduino-cli compile --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=opi,FlashSize=16M" OTA-bot-Mochi
arduino-cli upload --port COM3 --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=opi,FlashSize=16M" OTA-bot-Mochi
arduino-cli monitor --port COM3 --config baudrate=115200
```

The profile selects ESP32S3 Dev Module, native USB CDC on boot, OPI PSRAM,
and 16 MB flash for the Waveshare ESP32-S3-Touch-LCD-1.69 OLD reference board. The
user confirmed the pin assignments, not this complete memory profile. Confirm those memory
options match your own board before building. Upload replaces existing firmware.
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
   or an explicit missing-sensor diagnostic. Touch, RTC, and the IMU share one I2C
   bus at SDA=11/SCL=10; if touch answers but `IMU unavailable` persists, suspect
   the sensor identity or wiring rather than the bus pins.
2. Swipe through all screens and test repeated/edge gestures.
3. Connect Chronos and verify the clock transitions from waiting to synchronized
   time. Test 12/24-hour mode, disconnect, and reconnect.
4. Send a long notification while Eyes is selected and confirm Alerts opens
   automatically. Check six-second message pagination and accented
   text. The font approximates common lowercase Latin accents; other unsupported
   glyphs, including emoji, appear as `?`.
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
   Check MOCHI/TRAPEZ have no pupils/highlights, BLOUB is visibly wider, and TRAPEZ
   eyes are mirrored without protruding spikes. Enable Random mood and check
   temporary expressions and restoration; enable IMU mood on the bench to test
   sustained X/Y/Z deflections rather than isolated bumps.
10. Run an extended BLE session and measure heap, power, and battery runtime.

Also test Clock → DESK TOOLS and all four tools. Verify large clock legibility,
timer continuation while viewing other pages, default-silent completion, and reboot reset.
Switch Route layout temporarily, SAVE a different setting, and reboot: the saved
route default must win. Test `route default`, a Route hold, and LOOK → Route default.
A short eye tap must react for three seconds and restore the selected mood; holds
and swipes must not trigger it.

Enable sound only on the bench first. Check tone/level, category switches,
night mute, timer cues, and no backlight flicker while changing tone frequency.
Use AUDIO → Test sound or USB `sound test`, then `sound`, to distinguish mute
from PWM failure. PWM-ready status alone cannot establish physical audibility.
Check three-row scrolling, fixed tabs/actions, edge taps, and no value changes
during vertical gestures. Cycle every eye color and verify all utility pages
follow it, without losing text readability.
Switch EN/IT, SAVE, reboot, and check language persistence. Verify smooth face
handoffs and all idle gestures. For the reported weather failure, inspect USB
`weather` counters and try `weather sync` plus app weather configuration; do not
mark the physical weather issue resolved from native tests alone.

Battery percentage and charging state are deliberately unavailable until the
actual power circuit, ADC path, divider ratio, and calibration are verified.
