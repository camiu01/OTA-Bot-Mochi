# TODO

## Version 1.3.0 acceptance and local OTA

Version 1.3.0 is the planned next release after v1.2.1, not a published tag.
See [Changelog](CHANGELOG.md). Functional confirmations below are scoped to
AMOLED V2 unless another board is explicitly recorded.

- [x] Fix the eye-motion reference test by subtracting its same-time backdrop; the complete native runner reports all host tests passed (2026-10-28).
- [ ] Compile both profiles with the pinned CI toolchain, checking the new Wi-Fi code, flash size and heap headroom.
- [x] Install/update AMOLED V2 over COM6 using unchanged 16 MB OTA layout v1; verify image hashes, reboot and retained saved settings (2026-10-29).
- [ ] Verify LCD OLD installation/migration separately and record calibration effects.
- [ ] Update both directions between two trusted OTA-capable releases via the browser, retaining saved settings and calibration.
- [ ] Test phone/desktop browsers, temporary WPA2 credentials, five-minute expiry, BACK and setup-AP shutdown; enabled saved station must remain connected.
- [ ] Complete remaining web IMU/audio actions and NTP failure cases on both boards: association/channel changes, invalid credentials, unreachable server, BACK/expiry, retained station ownership and time, and later Android/Chronos resync. Native SNTP tests do not exercise real radio or UDP.
- [x] User confirmed Wi-Fi and settings changes working on AMOLED V2 (2026-10-29); this supersedes the earlier access-only report, not the detailed failure/endurance matrix.
- [x] Remove automatic configuration-screen/AP opening; keep background connection and manual access only. Native startup/offline and manual-route checks passed.
- [ ] Physically verify saved 2.4 GHz Wi-Fi across reboot, wrong-password/offline background retry without automatic configuration-screen opening, DHCP changes and authorized LAN QR, automatic NTP, city search, HTTPS/Open-Meteo forecasts, stale/offline cache, Forget, five-minute web expiry with station retained, and Wi-Fi/BLE/frame-rate/heap/power behavior on both boards.
- [ ] Scan the Wi-Fi QR on LCD OLD and AMOLED V2 with Android/iOS cameras; verify eight-character password entry, QR/details switching, fresh sessions, rotation and manual fallback.
- [ ] Check upload progress, Wi-Fi/BLE coexistence and post-OTA Chronos reconnection on LCD OLD and AMOLED V2.
- [ ] Reject renamed wrong-board, old pre-marker, full, corrupt, oversized and truncated images on real hardware.
- [ ] Test controlled upload interruption/recovery with USB recovery available; do not assume native tests prove power-loss safety.
- [ ] Record whether boot-health rollback or signed firmware is required before wider deployment; neither is enabled.

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

- [x] User confirmed ANCS notifications working on iPhone with AMOLED V2 (2026-10-29).
- [ ] Record ANCS reconnect after Bluetooth toggles/reboot and permission changes on AMOLED V2; switch back to Android with SAVE/restart and recheck Chronos. Repeat delivery/reconnect on LCD OLD. Basic delivery confirmation does not prove every recovery case.

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
- [ ] Implement full AXP2101 battery percentage/configuration support if needed;
  the current firmware still reports NOT MEASURED without a supported ADC path.
  Read-only charge-start audio detection is implemented, with hardware checks pending.
- [ ] Verify battery circuitry/divider calibration and AMOLED PMU charge-start
  transitions on hardware; the new event reader does not validate charger setup.
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
- [x] Record local builds for source checkpoint `8b752ba`: PlatformIO 6.1.19, espressif32 8.0.0, Arduino core 3.3.12, Chronos 1.9.1, NimBLE 2.5.1, GFX 1.6.9 and ESP32Time 2.0.6; both board profiles compiled, LCD PSRAM disabled and AMOLED OPI enabled.
- [x] Run targeted ANCS parser, profile migration, web settings and three-panel layout tests after ANCS integration.
- [ ] Record the exact installed binary hash, phone OS and settings for each physical acceptance report. Inspect remote CI separately; local builds do not establish pinned Arduino CLI CI success.
- [x] Confirm v1.2.0 remote host, firmware (both profiles), and release jobs:
  run 40499404191 at e0ac13ad78ba5467062553ea8833125f7ca6d140 succeeded.
  This does not validate later workspace changes; see `docs/CI.md`.
- [ ] Implement automatic OTA deployment if requested; binaries are not deployed automatically.

See `docs/UI.md`, `docs/SETTINGS.md`, `docs/BOARDS.md`, and `docs/TESTING.md`
for behavior, board details, and reproducible checks.

## Music and SD audio bench verification

- [ ] Verify Android Chronos title/artist and previous/toggle/next with real players.
- [ ] Verify iPhone AMS alongside ANCS, bonding, service publication changes and reconnects.
- [ ] Verify both panels' Music touch targets and long metadata on physical displays.
- [ ] Verify AMOLED V2 SD WAV playback, fallback, master/SD/night/category mute and SAVE.
- [ ] Measure SD read latency, speaker level, card removal/reinsertion and radio/display coexistence.
- [ ] Compile both firmware profiles through the user-owned build workflow.

- [ ] Verify startup.wav once per restart, charging.wav on a real stable charge
  start, five face-family WAVs and generic/built-in fallback; check mute/priority.

- [ ] Verify phone volume controls on real Android/iPhone players and AMS percentage updates.
- [ ] Verify automatic night at configured local-hour edges, missing time, NTP/phone corrections and manual override on both physical panels.
