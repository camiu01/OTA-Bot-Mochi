# TODO

## Acceptance and local OTA

Functional confirmations below are scoped to
AMOLED V2 unless another board is explicitly recorded.

- [ ] Compile and physically check random whole-cycle scheduling and live finger gaze on LCD OLD/AMOLED V2: no truncated performances, repeat variability, follow/release and flipped coordinates, unchanged tap/pet/swipe/Settings hold, and Shake recovery. Native checks and GIFs are not proof of physical timing or touch tracking.
- [ ] Investigate two Windows LCD-emulator access violations after writing the final PPM during concurrent default-backend runs. A single debugger run exited normally; all eight repeated sessions passed with `SDL_RENDER_DRIVER=software`. The accelerated-backend cause is not established.

- [ ] Compile and physically check seeded face variation on LCD OLD and AMOLED V2: all styles, Calm/Dasai/Playful, dashboard/night intensity, open Moon/Fire, backdrop handoffs, frame cadence and clean palette restoration. Native timing/raster checks and GIF previews do not establish physical flicker or frame speed.
- [ ] Compile and physically check the Games layer below Scrivania and D&D dice on LCD OLD/AMOLED V2: type/count controls, maximum 100D100 legibility, result paging, navigation, phone-mode visibility and unchanged results after redraw. Native tests do not prove physical touch usability or random-source quality.
- [ ] Physically validate manual microSD firmware updates on AMOLED V2: valid alternate-slot install and retained settings/calibration, audio-to-update ownership handoff, missing/wrong/corrupt files, cancellation, real SD read failures, progress responsiveness and controlled interruption with USB recovery ready. Both builds, the full native suite and a USB installation passed on 2026-11-03; these are not proof of SD installation or power-loss safety.
- [ ] Compile and audition all 20 eye sound variants in each of the five families on LCD OLD and AMOLED V2; check numbered SD WAVs, legacy/generated fallbacks, mute and BLE/display coexistence. Native tests do not establish physical audibility or SD latency.
- [ ] Physically verify all 20 charge-start and 20 unplug variants on AMOLED V2, including numbered SD WAVs, charge completion with VBUS present, real cable removal, noisy/failed PMU reads, independent cooldowns and Charging/master/night mute. USB previews do not prove physical edge detection.
- [ ] Compile and physically check Anime, Kitsune and Headlight DRL on LCD OLD and AMOLED V2: frame cadence, gaze/blink clipping, mood transitions, display/web/USB selection and SAVE/reboot. Native rasters do not validate display speed.
- [ ] Verify single-pass and back-and-forth vertical petting with lateral drift below 64 pixels on both touch controllers; small drifting taps must remain taps.
- [x] Fix the eye-motion reference test by subtracting its same-time backdrop; the complete native runner reports all host tests passed (2026-10-28).
- [x] Compile both profiles with the pinned CI toolchain (Arduino CLI 1.2.0, ESP32
  core 3.3.11), checking the Wi-Fi and charging code, flash size and heap headroom,
  and record the result at the current HEAD.
- [x] Install/update AMOLED V2 over COM6 using unchanged 16 MB OTA layout v1; verify image hashes, reboot and retained saved settings (2026-10-29).
- [x] Verify LCD OLD installation/migration separately and record calibration effects.
- [ ] Update both directions between two trusted OTA-capable firmware images via the browser, retaining saved settings and calibration.
- [x] Test phone/desktop browsers, temporary WPA2 credentials, five-minute expiry, BACK and setup-AP shutdown; enabled saved station must remain connected.
- [ ] Complete remaining web IMU/audio actions and NTP failure cases on both boards: association/channel changes, invalid credentials, unreachable server, BACK/expiry, retained station ownership and time, and later Android/Chronos resync. Native SNTP tests do not exercise real radio or UDP.
- [x] User confirmed Wi-Fi and settings changes working on AMOLED V2 (2026-10-29); this supersedes the earlier access-only report, not the detailed failure/endurance matrix.
- [x] Remove automatic configuration-screen/AP opening; keep background connection and manual access only. Native startup/offline and manual-route checks passed.
- [x] Physically verify saved 2.4 GHz Wi-Fi across reboot, wrong-password/offline background retry without automatic configuration-screen opening, DHCP changes and authorized LAN QR, automatic NTP, city search, HTTPS/Open-Meteo forecasts, stale/offline cache, Forget, five-minute web expiry with station retained, and Wi-Fi/BLE/frame-rate/heap/power behavior on both boards.
- [x] Scan the Wi-Fi QR on LCD OLD and AMOLED V2 with Android/iOS cameras; verify eight-character password entry, QR/details switching, fresh sessions, rotation and manual fallback.
- [ ] Check upload progress, Wi-Fi/BLE coexistence and post-OTA Chronos reconnection on LCD OLD and AMOLED V2.
- [ ] Reject renamed wrong-board, old pre-marker, full, corrupt, oversized and truncated images on real hardware.
- [ ] Test controlled upload interruption/recovery with USB recovery available; do not assume native tests prove power-loss safety.
- [ ] Record whether boot-health rollback or signed firmware is required before wider deployment; neither is enabled.

### Chronos, clock, and connectivity

- [x] User confirmed ANCS notifications working on iPhone with AMOLED V2 (2026-10-29).
- [x] Record ANCS reconnect after Bluetooth toggles/reboot and permission changes on AMOLED V2; switch back to Android with SAVE/restart and recheck Chronos. Repeat delivery/reconnect on LCD OLD. Basic delivery confirmation does not prove every recovery case.

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
- [x] Check tall UwU eyes/cheeks (no mouth) and neon glare appearance/frame time on both physical panels.
- [x] Check the seven themed backdrops (mecha armour, samurai sun, titan wall, yokai night, neon tubes, wheels road, uwu halos) on both physical panels: visibility, palette/night-theme blending, and no eye-ink occlusion.
- [ ] Decide whether to add backdrops to the remaining scenery candidates (TRAFFIC, SAKURA, BAMBOO, UPSIDE_DOWN) and to frozen IDs 1-122, which would require re-recording the raster fixtures.
- [ ] Check Continuous playback's recent-ID/visual-family separation during an extended device session.
- [ ] Recheck the user-reported temporary-face stall with Continuous, IMU mood and Shake enabled on AMOLED V2; native tests fix reproduced IMU starvation and verify bounded Shake recovery, not physical resolution.
- [x] On AMOLED V2, verify the three-second farewell reaches the four-second PWR cutoff without an additional blank-screen hold; check early-release cancellation on USB and battery.

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

A native reproduction on
2026-10-21 found that 80 ms polling could silence short cues while the long
test remained audible. Bounded cues now use the PCM worker's sample clock and
DMA drain. The latest general user confirmation supersedes the remaining
functional complaint; the agent did not listen to or test physical audio.

- [x] Record category/master/night-mute/level combinations during animations,
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
- [x] Add AMOLED battery percentage from the AXP2101 battery-voltage ADC behind
  the same plausibility gate; charging configuration remains unimplemented.
- [ ] Implement full AXP2101 charging configuration if needed; voltage/current
  limits and safety settings are untouched. Read-only charge-start audio
  detection is implemented, with hardware checks pending.
- [ ] Verify battery circuitry/divider calibration, the AXP2101 percentage reading
  and AMOLED PMU charge-start transitions on hardware; the new readers do not
  validate charger setup.
- [ ] Verify the charge indicator on AMOLED V2: the bolt appears only after two
  stable charging samples, the percentage switches to the accent color, `--%`
  still appears without a plausible reading, the DEVICE STATUS bolt matches the
  header, and USB `show` reports `charging=on|off` across plug/unplug and
  read-error transitions. Native tests do not prove the PMU direction bits.
- [c] Confirm a saved version-nine profile survives the update to version ten on a
  real device: charging switch on by default, night schedule retained, and only an
  explicit SAVE rewriting the seventeen-byte record.
- [ ] Measure power consumption and battery runtime.

### Other boards and measured robustness

- [x] Preserve the LCD 1.69 OLD pin assignments and single shared I2C controller.
- [x] Keep PSRAM disabled on LCD 1.69 OLD: OPI claims GPIO33/35/36 and caused
  continuous beeping and I2C crashes on 2026-10-13.
- [x] Require OPI PSRAM for the AMOLED 368x448 canvas.
- [x] Cover both board profiles and multiple panel geometries in native tests.
- [ ] Verify the extended SAVE/Back band on the tall AMOLED panel: both buttons are
  tappable to the panel edge, the four-pixel inert margin stays inert, and the band
  never activates the row above it.
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
- [x] Confirm remote host and firmware jobs for both profiles:
  run 40499404191 at e0ac13ad78ba5467062553ea8833125f7ca6d140 succeeded.
  This does not validate later workspace changes; see `docs/CI.md`.
- [ ] Implement automatic OTA deployment if requested; binaries are not deployed automatically.

See `docs/UI.md`, `docs/SETTINGS.md`, `docs/BOARDS.md`, and `docs/TESTING.md`
for behavior, board details, and reproducible checks.

## Music and SD audio bench verification

- [x] Verify Android Chronos title/artist and previous/toggle/next with real players.
- [x] Verify iPhone AMS alongside ANCS, bonding, service publication changes and reconnects.
- [x] Verify both panels' Music touch targets and long metadata on physical displays.
- [ ] Verify AMOLED V2 SD WAV playback, fallback, master/SD/night/category mute and SAVE.
- [ ] Measure SD read latency, speaker level, card removal/reinsertion and radio/display coexistence.
- [ ] Verify the single mount with ten-second backoff: an absent or partially
  populated card must not remount on every cue or stall display frames, and a
  later insertion must recover without rebooting the device.

- [ ] Verify startup.wav once per restart, charging.wav on a real stable charge
  start, five face-family WAVs and generic/built-in fallback; check mute/priority.

- [x] Verify phone volume controls on real Android/iPhone players and AMS percentage updates.
- [ ] Verify automatic night at configured local-hour edges, missing time, NTP/phone corrections and manual override on both physical panels.

## Diagnostics acceptance

- [x] Check bilingual navigation, read-only behavior and layout on three native panel geometries; verify captured heap pools, reset causes, long uptime and SD last-attempt states with controlled fixtures.
- [ ] Verify SYSTEM > Diagnostics on both physical panels: scroll/BACK, live BLE/Wi-Fi transitions, heap values, reset reason and SD last-attempt state. Native tests do not establish physical behavior.
- [ ] Decide whether a persistent crash/reset record is needed; today the summary is
  USB-only and a disconnect loses it.

- [x] Verify the face-library web editor on a real phone/desktop browser: save,
  the "keep at least one face" rejection, the 409 revision conflict, and offline use.

## Documentation and repository hygiene

- [ ] Add a device photo or short video to the repository; the README still has
  only native emulator renders.
- [ ] Split `src/settings/settings_menu.cpp` (415 lines) and
  `OTA-bot-Mochi.ino` (435 lines) to stay under the 400-line guideline in
  `CONTRIBUTING.md`.

## Audit patch acceptance

- [ ] Validate Android Clock > Desk > Phone tools on both physical panels:
  find in both directions, timeout/stop, camera permissions/readiness, phone
  battery, eight alarms/repeats, water/movement windows, quiet/sleep across
  midnight, EN/IT ID association, hourly forecasts, sixteen-contact cache
  and nine QR slots. Repeat disconnect/reconnect and mute/night scenarios.
- [ ] Verify iPhone mode hides the Chronos-only tile/actions and replaces
  Chronos clock/weather hints while retaining ANCS/AMS and Wi-Fi features.
- [ ] Measure the new bounded companion cache and BLE/HTTPS/audio task stack
  headroom on LCD (no PSRAM) and AMOLED; native tests are not RAM profiling.

- [ ] Verify the battery-with-bolt icon appears/disappears on stable charge changes
  without changing pages. Recheck AMOLED displayed 0%/100% against the reported
  nominal 3% shutdown/92% full-charge endpoints and compare Android Chronos
  percentage/charging status after connecting and reconnecting. Native raster
  and setter fixtures do not prove phone delivery or battery calibration.

- [ ] Compile both board profiles with the shared core 3.3.12 and NimBLE 2.5.2 pins;
  inspect the new CI run, not a previous release run.
- [ ] On both physical boards, verify a pending OTA image remains unconfirmed until
  display/UI/BLE setup and fifteen seconds of healthy loop activity. Reset during
  that window and confirm return to the previous valid slot with the installed
  rollback-enabled bootloader; retain USB recovery.
- [ ] Verify AP v1-to-v2 migration: unchanged SSID, new twelve-character password,
  failed-write refusal, updated phone Wi-Fi credentials and scannable QR.
- [ ] Measure internal largest-free-block and task stack headroom during Wi-Fi/BLE
  coexistence and repeated HTTPS work; calibrate the conservative 45 KiB gate.
- [ ] Track remaining trust changes separately: signed images, LAN transport/client
  binding and physical approval, authenticated pairing and flash/NVS encryption.

See `docs/AUDIT_REMEDIATION.md` for implemented findings and validation limits.
