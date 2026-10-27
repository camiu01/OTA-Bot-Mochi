# TODO

## Device status, user-confirmed on 2026-10-20

The user reports that the functional checks on the Waveshare
ESP32-S3-Touch-AMOLED-1.8 V2 work, initially except for some sounds. The completed device
checks below record that report, not physical testing performed by the agent.
Native tests separately cover portable logic and failure handling.
This confirmation does not establish measurements, remote CI results, or
validation of unimplemented hardware capabilities.

**Update, 2026-10-27:** the user reports “everything works”. Treat this as
general functional confirmation superseding the earlier audio complaint,
not as an individually recorded pass for every checked item below. Exact
installed commit, toolchain, settings combinations, two-board coverage and
session duration were not supplied with that confirmation. The detailed lists
combine implemented behavior and historical user reports; use
[Hardware validation](docs/HARDWARE_VALIDATION.md) for repeatable acceptance records.

### Chronos, clock, and connectivity

- [x] Verify Chronos pairing, disconnection, reconnection, and synchronization.
- [x] Display connection state, synchronized time/date, and 12/24-hour mode.
- [x] Show waiting/offline states and keep the local clock running when disconnected.
- [x] Verify the AMOLED device profile and Chronos operation.

Chronos was initially reported working on 2026-10-12. The user confirmed all
Chronos functionality and functional device checks on 2026-10-20.

### Notifications and calls

- [x] Display app, timestamp, title, wrapped message, and automatic pagination.
- [x] Retain eight bounded local alerts with newer/older browsing and dismissal.
- [x] Verify alert browsing, dismissal, empty state, and new arrivals.
- [x] Show incoming calls and remove the overlay on dismissal/disconnection.
- [x] Keep notification/caller payload logging disabled.
- [x] Open Alerts on new notifications without repeated switches for ordinary updates.

### Weather

- [x] Display city, current temperature, condition icon, and forecast highs/lows.
- [x] Display update age and handle missing/stale weather.
- [x] Preserve forecasts regardless of current/forecast packet order.
- [x] Rate-limit phone synchronization and support manual refresh.
- [x] Provide packet-counter diagnostics without logging location or payloads.
- [x] Confirm weather works correctly on the physical AMOLED V2.

The confirmation on 2026-10-20 supersedes the weather failure reported on
2026-10-13; weather is no longer an open device issue.

### Navigation

- [x] Display turn directions, distance, ETA, and phone-supplied speed.
- [x] Render only complete received icons with a matching CRC.
- [x] Verify live phone navigation updates and inactive/disconnected states.
- [x] Provide large/details layouts and separate session overrides from saved defaults.
- [x] Automatically open Route on a real navigation start; preserve manual page choices on updates.

### IMU, eyes, touch, and controls

- [ ] Verify the revised samurai's inclined eyes/blade on both physical panels, including entry/return without detached eye fragments.
- [ ] Check tall UwU eyes/cheeks (no mouth) and neon glare appearance/frame time on both physical panels.
- [ ] Check the seven themed backdrops (mecha armour, samurai sun, titan wall, yokai night, neon tubes, wheels road, uwu halos) on both physical panels: visibility, palette/night-theme blending, and no eye-ink occlusion.
- [ ] Decide whether to add backdrops to the remaining scenery candidates (TRAFFIC, SAKURA, BAMBOO, UPSIDE_DOWN) and to frozen IDs 1-122, which would require re-recording the raster fixtures.
- [ ] Check Continuous playback's recent-ID/visual-family separation during an extended device session.
- [ ] Recheck the user-reported temporary-face stall with Continuous, IMU mood and Shake enabled on AMOLED V2; native tests fix reproduced IMU starvation and verify bounded Shake recovery, not physical resolution.
- [ ] On AMOLED V2, verify the three-second farewell reaches the four-second PWR cutoff without an additional blank-screen hold; check early-release cancellation on USB and battery.

- [x] Verify QMI8658C detection, accelerometer operation, and gaze direction.
- [x] Verify mounting-relative gaze, eye-pair roll, and shake-to-dizzy restoration.
- [x] Calibrate from stable parked samples, with timeout/missing-sensor feedback.
- [x] Automatically save successful calibration in a separate validated NVS record.
- [x] Restore calibration at boot and allow replacement through IMU zero.
- [x] Verify IMU calibration and the associated device checks.
- [x] Verify upright/upside-down automatic rotation, manual fallback, and touch alignment.
- [x] Poll IMU on every page and defer display rotation while touch is held.
- [x] Verify swipe direction, edge touches, repeated gestures, and settings scrolling.
- [x] Verify the eyes, colored mood families, animated performances, and display readability.
- [x] Keep MOCHI/TRAPEZ solid and pupil-free; preserve BLOUB pupils/highlights.
- [x] Verify gauge, glow, headlights, Pong controls, and analog-clock synchronization.
- [x] Verify petting, double-tap, glance, Screen flip, status-bar mute, swipe-down home,
  startup greeting, and USB reboot.
- [x] Apply settings in RAM and persist the profile only on explicit SAVE.
- [x] Preserve legacy settings migration and English/Italian labels.
- [x] Rename the standard animation profile to Normal/Normale in the menu;
  retain the stable `dasai` USB value.

The IMU/functional confirmation above comes from the user. Corrupt records,
unavailable storage, failed writes, sensor timeouts, raster bounds, and solid-eye
exclusions are also covered by native tests, not claims of physical fault injection.
Auto rotate's switch remains session-only; calibration itself is persistent.

### AMOLED V2 board

- [x] Add the CO5300 368x448 QSPI board profile, TCA9554 reset release,
  controller brightness, CST820 touch path, and shared I2C ownership.
- [x] Bring up the AMOLED V2 display, touch, IMU, and Chronos functionality.
- [x] Record touch response at `0x15` and working horizontal swipes.
- [x] Implement the ES8311/I2S speaker backend with bounded, non-blocking PCM.
- [x] Confirm audible ES8311 bench tones at level 10 on 2026-10-19.
- [x] Record user confirmation of functional operation, including the later general audio confirmation.

## AMOLED sounds: user-confirmed functional resolution

The v1.2.0 release describes audio stabilization. A native reproduction on
2026-10-21 found that 80 ms polling could silence short cues while the long
test remained audible. Bounded cues now use the PCM worker's sample clock and
DMA drain. The latest general user confirmation supersedes the remaining
functional complaint; the agent did not listen to or test physical audio.

- [ ] Record category/master/night-mute/level combinations during animations,
  display, touch and BLE, and a continuous session of at least two hours.
  This is missing structured evidence, not a newly reported sound failure.

## Remaining capabilities and engineering work

These are not additional reported failures of the working Chronos/IMU/UI paths.

### Battery, charging, and power measurements

- [ ] Verify the AMOLED V2 PWR farewell on battery and USB: closing/fading eyes,
  cancellation on early release, ignored startup hold, and autonomous four-second
  cutoff with a stalled main loop. Native tests are not physical validation.
- [ ] Verify AMOLED BOOT page cycling, wraparound, held/bouncing presses and
  suppression during startup, shutdown, visible calls and touch contact.
- [x] Add plausibility-gated battery ADC scaffolding and NOT MEASURED fallback.
- [x] Add adjustable brightness and optional idle dimming with touch wake.
- [ ] Implement an AXP2101 battery/charging driver for AMOLED V2 if needed;
  the current firmware still reports NOT MEASURED without a supported ADC path.
- [ ] Verify battery circuitry/divider calibration on applicable hardware and
  add charging status when a supported driver exists.
- [ ] Measure power consumption and battery runtime.

### Other boards and measured robustness

- [x] Preserve the LCD 1.69 OLD pin assignments and single shared I2C controller.
- [x] Keep PSRAM disabled on LCD 1.69 OLD: OPI claims GPIO33/35/36 and caused
  continuous beeping and I2C crashes on 2026-10-13.
- [x] Require OPI PSRAM for the AMOLED 368x448 canvas.
- [x] Cover both board profiles and multiple panel geometries in native tests.
- [ ] Repeat newly added eye/desk features and audio checks on the LCD OLD board;
  AMOLED confirmation does not certify a different physical board.
- [ ] Measure worst-case frame time and extended-session heap behavior.
- [ ] Perform physical fault-injection checks if required; native failure tests
  do not establish real allocation, bus, or storage failure behavior.

### Build, CI, and delivery

- [x] Document Arduino CLI setup, dependency pins, build/upload commands, and native checks.
- [x] Add GitHub Actions native checks, firmware compilation, and downloadable binaries.
- [x] Document the AMOLED bring-up and user-reported board behavior.
- [ ] Record the exact toolchain/board options used for the latest user device checks.
- [x] Confirm v1.2.0 remote host, firmware (both profiles), and release jobs:
  run 40499404191 at e0ac13ad78ba5467062553ea8833125f7ca6d140 succeeded.
  This does not validate later workspace changes; see `docs/CI.md`.
- [ ] Implement automatic OTA deployment if requested; binaries are not deployed automatically.

See `docs/UI.md`, `docs/SETTINGS.md`, `docs/BOARDS.md`, and `docs/TESTING.md`
for behavior, board details, and reproducible checks.
