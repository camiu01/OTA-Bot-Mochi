# Repeatable hardware validation

## Evidence boundaries

- **Implemented:** source contains the behavior.
- **Automated:** native substitutes or firmware compilation passed at a recorded
  revision. Neither establishes real display, BLE, audio or electrical behavior.
- **User-confirmed:** historical AMOLED V2 bring-up and functional reports in
  [TODO](../TODO.md), including the latest “everything works” confirmation.
  That report does not identify a tested binary hash, every settings combination,
  both boards, continuous duration, consumption or runtime.
- **Measured:** requires the records below and actual instruments/device results.
  No such two-hour, power or runtime result is supplied by this consolidation.

Compilation, installation and functional verification are three different events.
Repeat the procedure separately on LCD OLD and AMOLED V2. Never infer LCD
coverage from the AMOLED report.

## Record for every check

Copy this table into the test report, not a new empty repository file.
Do not include notification text, callers or location in logs/screenshots.

| Field | Required value |
| --- | --- |
| Board and revision | Exact manufacturer/model/revision; panel controller |
| Firmware | Tag, full commit, binary SHA-256; indicate dirty/custom builds |
| Build/install | PlatformIO environment or Arduino FQBN; CLI/platform/core/library versions; PSRAM; installation method and date |
| Setup | USB/battery source, mounting, phone OS and Chronos version; relevant settings |
| Procedure | Check ID below, steps, repetitions and actual duration |
| Expected | Expected result below, plus any acceptance threshold decided before measuring |
| Observed | Actual result, pass/fail/not run, sanitized log or photo reference |
| Executor/date | Person performing the physical check and timestamp |
| Limits | Missing equipment, untested settings, deviations, interruptions |

## Short acceptance sequence

| ID | Procedure | Expected |
| --- | --- | --- |
| H1 | Three cold starts, then USB `reboot`; record startup diagnostics | UI/touch/IMU ready or explicit missing-device status; greeting returns to selected eyes; no unexpected resets |
| H2 | Swipe all six pages; tap corners and settings rows; flip manually; hold a finger during automatic turnover | Touch follows orientation, rotation waits for release; swipes do not edit rows |
| H3 | Hold still, use SENSORS → IMU zero; reboot; tilt gently; recalibrate in a different supported mounting | SAVED reference restores at boot; second successful calibration replaces it; gaze and automatic 180° rotation follow mounting |
| H4 | Pair Chronos, sync time, disconnect/reconnect three times, reboot and resync | Connection state follows link; clock continues offline, waits after reboot until synced |
| H5 | Send synthetic short/long alerts; browse/dismiss eight entries; deliver a ninth | Newest opens Alerts; pagination readable, oldest bounded entry drops; no phone payload logging |
| H6 | Sync current weather/forecast, refresh; start/stop/restart phone navigation, swipe away during updates | Weather/forecast visible, refresh throttled; new route opens Route, ordinary updates preserve manual selection; inactive state after end/disconnect |
| H7 | Enable sound on bench at level 10; test face/timer/alert categories, category-off, master-off, night mute; animate continuously and swipe | Permitted cues audible, forbidden cues silent; no frozen UI, touch interference or backlight flicker; record exact settings |
| H8 | Edit color/language/brightness, reboot without SAVE; repeat with SAVE | Unsaved edits lost; explicitly saved values restored; separate IMU calibration retained |
| H9 | Follow board-specific PWR procedure in BOARDS.md on USB; battery only if safely supported already | AMOLED 4s off/1s on; LCD 4s hold then release, USB light-sleep wake/reboot; no claim of zero standby current |
| H10 | At least **two uninterrupted hours**, phone linked, alternating eyes/scenes and pages; synthetic alerts/weather/routes and sound every 10 minutes; reconnect every 30 minutes | No crash, spontaneous reset, stuck audio/UI or progressive failure; record elapsed time and diagnostic heap/timing trends, not just “works” |

For H3, sampling/storage failures have native coverage, not physical fault
injection. Do not disconnect a live shared bus to manufacture failures. Record
unavailable equipment as not run. Test weather staleness separately at three
hours if needed; a two-hour session cannot establish that transition.

## Optional runtime measurements

Previously the sketch printed total free heap once at `UI ready`; `sound` and
`weather` already expose bounded status/counters. Those do not provide separate
heap minima or render timings. `MOCHI_RUNTIME_DIAGNOSTICS=1` adds an opt-in
summary once per 60 seconds. It defaults to **0**, with no diagnostic code in
normal builds. No new task, framebuffer, dependency or rendering allocation is
introduced.

For PlatformIO, temporarily append `-DMOCHI_RUNTIME_DIAGNOSTICS=1` to the chosen
environment's existing `build_flags`, preserving its board and USB flags.
Build only that environment; remove the flag and rebuild afterward. For Arduino
CLI include the flag alongside the selected board macro in
`compiler.cpp.extra_flags`. Record the flag with the tested binary.

Capture USB at 115200 baud before startup. Summaries contain:

- `internal_free`/`internal_min`: current and since-boot low watermarks for
  `MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT`.
- `psram_free`/`psram_min`: separate `MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT` pool,
  zero when unavailable. Do not add overlapping capability pools.
- Uptime and ESP-IDF reset reason enum; repeated boot messages/uptime restarting
  expose resets. Keep existing startup/panic/backend error diagnostics.
- Last interval's count and maximum **draw call**, **flush call**, and their
  combined duration in microseconds; count exceeding 33 ms.

The timed interval begins immediately before `screens_draw`, includes
`eyes_update` when applicable, and ends when `gfx->flush()` returns.
Unchanged utility draws count too, with zero flush time. The startup intro,
frame scheduling wait, BLE/touch/IMU/sound services, and diagnostic printing are
excluded. **This is not end-to-end frame time, scanout time, FPS or proof that
asynchronous hardware has finished transferring.** Maxima reset each minute;
heap minima do not. Serial output and heap queries add small periodic overhead,
so compare enabled/disabled runs before interpreting tight timing thresholds.
USB disconnect can lose summaries; there is no persistent crash recorder.

Record heap after warm-up (including first audio use), then every minute.
A downward trend warrants investigation; no numerical leak/performance claim is
made without device measurements. Record min/max draw/flush data per workload,
not a single average across unrelated pages.

## Power and battery

Use a suitable external USB meter/current instrument; record voltage, brightness,
sound, BLE state, supply path and sampling method. Measure active and board-specific
off/sleep states separately. USB-powered current is not battery current.
Battery capacity, divider accuracy, charging limits and safe wiring must be
established first. Never connect a guessed battery or use a meter in current
mode across a supply. Autonomy requires an actual controlled discharge test,
not an estimate presented as experimental evidence.

## Real-device demo, 30–60 seconds

Record the actual board, not the emulator, with its revision and installed
version in a caption. Use synthetic phone data and hide personal information.

1. **0–10 s:** power on; show the startup title and return to the selected eyes.
2. **10–25 s:** show blinking, two expressions and one complete animation.
3. **25–40 s:** tap/pet, swipe a page and gently tilt the stationary board.
4. **40–55 s:** show Chronos connected and one synthetic notification or weather
   update appearing on the device.
5. **55–60 s:** caption the board/version and state that this is a functional
   demo, not endurance, consumption or battery-runtime evidence.
