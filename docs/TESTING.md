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
The native runners explicitly select `MOCHI_BOARD_LCD_169` for baseline fixtures
and `MOCHI_BOARD_AMOLED_18` for AMOLED checks, so an Arduino IDE selection in
`pin_config.h` cannot silently change the test board. Defining both is rejected.
An explicit LCD CLI build can likewise use
`--build-property "compiler.cpp.extra_flags=-DMOCHI_BOARD_LCD_169"`.
The real eye renderer also runs against a native raster substitute: all four
styles, all 123 expressions/animated moods, and both blink states (984 combinations), a proof
that every expression renders differently from NEUTRAL on every style, that no
two names draw the same pixels on any style (pairwise uniqueness, 30012
comparisons), solid-style pupil/highlight exclusion, trapezoid symmetry, IMU
gaze/roll bounds, radius limits, and dirty-region erasure are checked. The
configured palette has its own suite: resting emotions and symbols must keep
the selected ink on every style, across three selected body colors.
The petting heart must use the selected body color and erase without trails.
The face catalog checks compile the documented C++17 per-eye preset example and
validate all 123 IDs/names, parameter ranges, USB selection and settings round-trips.
Two frozen pre-refactor raster fixtures check pixels and dirty bounds across six
panels and four styles; the expanded fixture samples 47,232 frames across all
123 expressions. See [Editing and adding a face](ADDING_A_FACE.md) for scope and
the rule against regenerating baselines from the refactored renderer.
Related emotion families must differ in at least ten percent of their combined
occupied-pixel masks, independent of color, on LCD, AMOLED, and a small panel.
The face suite also exercises 24 GIF-informed reaction loops on four panel
sizes: time-only shape changes, exact recovery, timestamp rollover, blink
priority, frame-local color blends, solid-style exclusions, and
dirty-versus-full redraw equality.
The colored
performances also have a dedicated `performance_test.cpp` suite covering all
twenty-six additions on four panel rasters: reversible endpoints, frame-local
semantic colors, clean-redraw equality, unchanged v5 preferences, and complete
six-second automatic-reaction protection, including timestamp rollover. Scene
regressions require exactly twelve ordinary equalizer columns and intact eye
silhouettes throughout the morph, one joined sushi, large inward-facing turbo
housings with moving impellers and smoke bursts, rain entering from the top, and small eyes
at the bottom with an upward gaze beneath a changing star field. Panel
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
automation disabled. Expanded tests cover version-six thirteen-byte profiles with
version-five, version-four and older migration, Face flow, the reboot verb, EN/IT
label completeness and layout, out-of-order weather packets/sync throttling,
animated dirty regions, sound notes/priorities/mute/rollover/failures, and actual
PWM adapters against core-2/core-3 shared-timer mocks.
AMOLED PWR-key tests verify the AXP2101's 4-second off/1-second on timing,
preservation of unrelated power/charging bits, missing-device/read/write failures
and idempotent setup. LCD tests check the original GPIO35 power hold, GPIO36
wake, debounced hold/release, ignored startup press, rollover and wake/sleep
failure paths. Backlight suspension is checked on core 2 and 3 without changing
saved brightness or the buzzer's PWM timer. Physical off/on, USB wake and standby
current still require a board test; see [Boards](BOARDS.md).
Startup tests tie the three-second farewell to the PMU's remaining hold after
its one-second long-key event. Continuous-playback tests reject the last eight
IDs and adjacent visual families across all bases and rollover, including
repeated random inputs. Samurai tests cover its asymmetry, inclined blade,
moving glint, clean return and preservation of every eye pixel while the blade
fades, on six panels and all four styles. Both native runners include these tests.
UwU checks verify white top-open U eyes, separate pink cheeks, the absence of
a mouth and decorative backdrop, and clean reversible cycles. Background-only handoff
checks verify full-strength eye ink while scenery fades, including clean redraws
and timer rollover. Neon checks verify a hollow
center and outward glare falloff relative to each style's actual silhouette,
not just its rectangular bounds.
Continuous playback also faces two simulated minutes of sustained IMU input,
including timer rollover. The regression counts actual random selections,
not repeated IMU reactions. Repeated Shake overrides must expire after two
seconds and allow playback to keep advancing, even when sensor reads fail.
These checks do not establish that every reported physical stall is resolved.
Additional checks exercise all six shared color palettes, filled touch gutters,
row/tab/footer edges with row hits aligned to their visible cards, contiguous
grid-cell rows, 58-pixel desk hub cards, 50-pixel timer buttons, contextual
vertical-swipe targets, bounded
menu scrolling, vertical/horizontal
axis isolation, tap tolerance, explicit audio-test mute/category behavior,
randomized face-tone family pitch bands with the faces switch, the battery
voltage curve, divider plausibility window, and consecutive-sample trust policy,
plus the petting heart marker's dirty-region wipe and clamped placement.
Mocks do not verify physical LCD rendering, BLE protocol interoperability, physical
pin assignments, or electrical behavior.

Video-reference tests exercise Pong, clock eyes, decorative gauge, cycling glow
and headlight beams on LCD, AMOLED and a smaller 128x160 raster, including
repeated dirty-region clearing, solid-style pupil exclusion, and
USB/v5-profile round-trips for the nine new moods. Pong physics tests cover
paddle limits, both collisions, scoring, score wrap, timestamp rollover and
bounded catch-up. Per-panel UI tests cover the six desk cards, playable Pong
input/frame cadence/off-page pause and synchronized analog-clock validation.

The AMOLED ES8311 suite also executes the codec register sequence and PCM
worker against shared-bus/I2S doubles. It checks fixed-rate stereo output,
pitch and amplitude bounds, continuous phase across DMA blocks, lazy startup,
amplifier mute, command expiry across timestamp rollover, partial-init cleanup,
task-allocation failures, DMA errors/short writes, and reuse after mute.
Sample-clocked cue regressions cover exact note/pause lengths, input bounds,
5 ms fades, both notes with 80/160 ms main-loop stalls, and amplifier drain
versus immediate explicit mute.
Physical speaker audibility and loudness still require the bench test in
`docs/SOUND.md`.

## Host LCD emulator (SDL2)

The host emulator is not an OTA radio/flash simulator. Six native OTA suites
(updater, QR and maintenance screen for LCD and AMOLED) use transport, flash and
encoder doubles. They cover image/auth failures, streaming, timeout/reboot,
credential renewal, QR bounds/quiet zone and EN/IT screen states. Physical QR
scanning and wireless updates remain in the [OTA checklist](../TODO.md).

`tools/run-emulator.sh` builds a native window that drives the same UI code the
tests link: real page routing, the production gesture policies, settings, sound
and backlight services, the USB command parser, and the real eye renderer, all
drawn into the 240x280 RGB565 test raster and scaled to a window.

```sh
# Linux or macOS, with g++, pkg-config, and libsdl2-dev (or SDL2-devel)
bash tools/run-emulator.sh
```

### Windows PowerShell

`tools/run-emulator.ps1` builds and launches a native Windows executable from
PowerShell 5.1 or later. Bash, WSL, and a connected board are not required.

Install MSYS2, then run this once in its **UCRT64** terminal:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-SDL2
```

If your existing compiler is in `C:\msys64\mingw64`, use the **MINGW64**
terminal and install `mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2` instead.
The compiler and SDL2 must belong to the same toolchain.

From the repository root in PowerShell:

```powershell
.\tools\run-emulator.ps1
.\tools\run-emulator.ps1 -Size 368x448 -Page settings
```

The script uses `g++.exe` from PATH, or looks in `C:\msys64\ucrt64\bin`
and `C:\msys64\mingw64\bin`. Select another compiler explicitly if needed:

```powershell
.\tools\run-emulator.ps1 -Compiler C:\msys64\ucrt64\bin\g++.exe
```

If Windows blocks local scripts, use a process-only execution-policy override:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tools\run-emulator.ps1
```

To check mouse input without opening a window, run
`.\tools\run-emulator.ps1 -TestInput` (or
`bash tools/run-emulator.sh --test-input` on Linux/macOS).
The checks cover logical coordinates, fast and double clicks, holds, drags,
letterbox margins, and contact cancellation when the window loses focus.
Add `-Size 240x280` or `-Size 368x448` to check each panel size on Windows.

For a finite run without a window:

```powershell
.\tools\run-emulator.ps1 -Headless -Frames 120 -Page weather -Screenshot weather.ppm
.\tools\run-emulator.ps1 -Headless -Frames 120 -Command 'set mood derp' -Screenshot eyes.ppm
.\tools\run-emulator.ps1 -Size 368x448 -Headless -Frames 120 -Page pong -Screenshot pong.ppm
.\tools\run-emulator.ps1 -Headless -Frames 120 -Page analogclock -Screenshot analogclock.ppm
```

`-Headless` requires `-Frames`. Screenshots use PPM format. Windows builds are
cached per panel size under `$env:TEMP\mochi-emulator`; source/header changes
trigger a rebuild. Use `-Rebuild` to force one. The script adds the compiler's
DLL directory to PATH only while it runs and restores the SDL video driver
afterward.

### Preview behavior and controls

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
- In the terminal running the emulator, type a command and press `Enter`.
  Commands use the real USB parser: `help`, `mood names`, `mood glow`,
  `set mood derp`, or `brightness 70`. Terminal input does not pause rendering.
- Alternatively, focus the graphical window and press `Enter` to enter commands
  there. Typed text and replies appear in the terminal; `Esc` cancels this mode.

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
arduino-cli compile --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=disabled,FlashSize=16M" --build-property "upload.maximum_size=6553600" --build-property "compiler.cpp.extra_flags=-DMOCHI_BOARD_LCD_169" OTA-bot-Mochi
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
  --build-property "upload.maximum_size=6553600" \
  --build-property "compiler.cpp.extra_flags=-DMOCHI_BOARD_AMOLED_18" OTA-bot-Mochi
```

CI compiles and publishes both profiles on every push; the AMOLED artifacts are
build outputs, not physical validation. See [CI evidence](CI.md).

For a verbose **build only** on Windows with the IDE-bundled CLI:

```powershell
$cli = 'C:\Program Files\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe'
& $cli compile --verbose --warnings all --jobs 4 `
  --fqbn 'esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=opi,FlashSize=16M' `
  --build-property 'upload.maximum_size=6553600' `
  --build-property 'compiler.cpp.extra_flags=-DMOCHI_BOARD_AMOLED_18' `
  --build-path "$PWD\build\ota-amoled" `
  --output-dir "$PWD\build\ota-amoled-artifacts" `
  "$PWD\OTA-bot-Mochi"
```

Run from the repository root. This example targets AMOLED V2 and never opens a
serial port. For LCD OLD, use `PSRAM=disabled`, `MOCHI_BOARD_LCD_169` and separate
LCD output/build directories.

The sketch-local `partitions.csv` supplies the 16 MB OTA layout. The explicit
`upload.maximum_size=6553600` matches one app slot; without it Arduino CLI keeps
the board menu's old 1.25 MiB limit even when using the custom CSV. In Arduino IDE,
select the **Custom** partition scheme (if provided by the core) and keep the
application below 6,553,600 bytes. See [OTA](OTA.md) before migrating a device.
On LCD OLD, PSRAM stays disabled: the OLD revision routes the buzzer to GPIO33 and the
power-control lines SYS_EN/SYS_OUT to GPIO35/GPIO36, which the ESP32-S3 octal
PSRAM interface claims. A build with `PSRAM=opi` beeped the buzzer continuously
and crashed in the shared I2C driver on 2026-10-13. The 8 MB onboard PSRAM
therefore remains unused. Upload replaces existing firmware.
On Linux, replace `COM3` with the port reported by `board list`, often `/dev/ttyACM0`.

The pins above are the CI configuration. The v1.2.0 release run successfully
built both profiles with them; see [CI evidence](CI.md). This does not establish
which firmware is installed on a device. No upload was performed during the
2026-10-27 consolidation. Local verification uses PlatformIO; Arduino CLI remains
the independent CI/reference path.
See [GitHub Actions workflow](CI.md) for cached builds and downloadable artifacts.

### PlatformIO alternative

`platformio.ini` at the repository root mirrors both Arduino CLI profiles with
`src_dir = OTA-bot-Mochi`, the same library pins (ESP32Time 2.0.6,
NimBLE-Arduino 2.5.1, ChronosESP32 1.9.1, GFX Library for Arduino 1.6.9), and
`espressif32 @ 8.0.0` (Arduino core 3.3.12, close to the CI pin 3.3.11). The
default environment is `esp32s3-lcd`, so a bare `pio run` builds only the LCD
profile; pass `-e` explicitly and build/upload only the environment matching
the physical board. Uploading replaces the installed application, as with the
Arduino CLI path.

```sh
pio run -e esp32s3-lcd      # Waveshare LCD-1.69 OLD, PSRAM disabled (default env)
pio run -e esp32s3-amoled   # Waveshare AMOLED-1.8 V2, OPI PSRAM
pio device list
pio run -e esp32s3-amoled --target upload --upload-port COM3
pio device monitor --port COM3 --baud 115200
```

Both environments were built successfully with PlatformIO 6.2.0 on 2026-10-20,
and one AMOLED V2 upload through this path is recorded in `SOUND.md`
(2026-10-21). Arduino CLI remains the CI and reference path: CI does not build
the PlatformIO environments, and building one path does not verify the other.

On 2026-10-27 both environments again reported SUCCESS locally using PlatformIO
Core 6.1.19, Espressif32 8.0.0, Arduino core 3.3.12 and Xtensa compiler package
14.2.0+20260121. These are compilation results, not uploads or hardware results.
Build the environments sequentially; never run competing builds in the same
directory. The generic devkit board banner says 8 MB; the project overrides flash
to 16 MB. Check the generated configuration rather than treating that banner as
the Waveshare hardware specification.

### Linux serial permissions

Check the owner/group with `ls -l /dev/ttyACM0`. On distributions using `dialout`:

```sh
sudo usermod -aG dialout "$USER"
```

Log out and back in before retrying. Some distributions use `uucp` instead.
Do not make the device world-writable. Close other serial monitors before upload.

## Device validation checklist

### Local consolidation evidence, 2026-10-27

- `.\scripts\test-host.ps1` reported **All host tests passed**, including the
  compiled guide/catalog, settings migration, independent frozen raster fixtures,
  both board power-button policies, PWM and ES8311 tests. Fixtures were not changed.
- `python tests/release_package_test.py`: **4 tests passed**. The added diagnostic
  timing test also passed with C++17 and `-Wall -Wextra -Werror`; both native
  runners now include it. POSIX runner syntax was checked, not its full suite run.
- Both PlatformIO environments built with `MOCHI_RUNTIME_DIAGNOSTICS=1`:
  LCD **320.98 s**, AMOLED **314.73 s**, combined command exit **0**.
  See [diagnostic measurement boundaries](HARDWARE_VALIDATION.md).
- Final standard rebuilds, without the diagnostic flag, also passed sequentially:
  `pio run -e esp32s3-lcd -j 4` **219.77 s** and
  `pio run -e esp32s3-amoled -j 4` **222.28 s**, each exit **0**.
  Earlier interrupted standard attempts are not counted as passes.
  Static RAM reports were 51,244 / 51,520 bytes respectively, not runtime heap
  measurements; the final local artifacts use the standard configuration.
- Compiler warnings remain in pinned dependencies: ChronosESP32 calls deprecated
  `NimBLEService::start()` and has two unused `enabled` variables; an Arduino_GFX
  driver hides the core's `Print::flush()` overload. No dependency source was
  modified to hide these warnings.
- An initial diagnostic build was interrupted because PowerShell treated native
  stderr as `NativeCommandError`. Its leftover process tree was stopped before
  retrying. The successful run preserved stderr in logs, allowed warning output,
  and checked actual process exit status. This was not a firmware-code fix.
- Workflow YAML/gates and all shell blocks passed local checks. The metadata
  generation block executed successfully against two temporary fixture artifacts
  with a mocked CLI, including both checksum checks. This does not replace a real
  run of the changed workflow on GitHub; only the existing v1.2.0 run was inspected.
- Before the subsequent user-requested commit/push, no upload, hardware experiment,
  new remote workflow run or publication was performed. Binary asset checks compared published checksums with GitHub digests,
  not independently downloaded binary bytes.

Use the shorter [repeatable acceptance procedure and record format](HARDWARE_VALIDATION.md)
for each board/release. It includes saved IMU calibration, PWR controls, two-hour
operation and opt-in heap/render-call diagnostics. The detailed cases below are
procedures, not uncompleted bug reports; historical user confirmation is recorded
separately in TODO.

1. Confirm startup reports `UI ready`, a touch controller address, and `IMU ready`
   or an explicit missing-sensor diagnostic. Mochi must greet with a brief silent
   face that returns to the selected mood within three seconds. Touch, RTC, and the
   IMU share one I2C bus at SDA=11/SCL=10 on LCD OLD, SDA=15/SCL=14 on AMOLED V2; if touch answers but `IMU unavailable`
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
   bumps. Use controlled stationary bench tilts to check gaze and lean; record
   mounting and observed direction before proposing code changes. Do not operate
   or observe test controls while driving.
10. Run an extended BLE session and measure heap, power, and battery runtime.
11. Battery reading is best-effort on unverified hardware. Open Clock → DESK
    TOOLS → DEVICE STATUS with no battery fitted: the row must read NOT MEASURED
    and USB `show` must report `battery=unavailable`. With a known charged LiPo
    and the expected divider on `BAT_ADC`, compare the shown percentage against a
    multimeter across the discharge curve; do not treat the native conversion
    tests as physical validation.

Also test a downward swipe on Clock into Scrivania and all six tools. Verify
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
first stroke. On Eyes, confirm no audio-status icon appears at startup, while
muted, or after enabling sound elsewhere and returning. Other pages must keep
a recognizable speaker in their header, with waves when enabled and a cross
when muted. Flip SENSORS → Screen flip while parked: the interface, touch
targets, and menus must rotate together, tilting must still read in your frame,
SAVE plus reboot must keep it, and `set flip off` must restore the panel
orientation.

Enable sound only on the bench first. Check tone/level, category switches,
night mute, timer cues, and no backlight flicker while changing tone frequency.
Eye reactions must chirp in varied pitches inside their family band (low and slow
for sleepy faces, high bounce for surprised ones), not one fixed note.
Use SOUND → Test sound or USB `sound test`, then `sound`, to distinguish mute
from PWM failure. PWM-ready status alone cannot establish physical audibility.
Check three-row scrolling, fixed tabs/actions, edge taps, and no value changes
during vertical gestures. Tap near the bottom edge of a visible row card: it
must change that row, not the one below. In the Mood grid, confirm five rows of
larger cells whose rows are fully tappable with only the column gutters inert.
Cycle every eye color and verify all utility pages
follow it, without losing text readability.
Switch EN/IT, SAVE, reboot, and check language persistence. Verify smooth face
handoffs and all idle gestures. If weather fails again, inspect USB
`weather` counters and try `weather sync` plus app weather configuration.
The earlier weather failure was superseded by user confirmation on 2026-10-20,
not by native tests.

Battery percentage is a plausibility-gated best-effort estimate, deliberately
reported as NOT MEASURED when readings are implausible. Charging state remains
unavailable until the actual power circuit, ADC path, divider ratio, and
calibration are verified.
