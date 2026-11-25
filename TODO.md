# TODO

> Last reviewed: 2026-11-23

On 2026-11-16 the user reported that the pending physical display, touch, audio,
power-button, charging-indicator and phone-tool checks had been carried out; the
corresponding items below are ticked on that general report, without per-item
records. Functional confirmations are scoped to AMOLED V2 unless another board is
explicitly recorded. OTA fault/rollback cases, measurements, decisions and
implementation work remain open.

## Drive dashboard and OBD

State (added 2026-11-18): native suite and `git diff --check` passed; PlatformIO `esp32s3-amoled`
compiled (flash 40.9%) and was installed on AMOLED V2 over USB on 2026-11-18.
Nothing has been checked on screen, in a vehicle or with a dongle yet. See also
H14 in `docs/HARDWARE_VALIDATION.md`.

User report, 2026-11-18, AMOLED V2: gyro-fused lean works, ZERO works and lean returns to zero
after tilting. A later report of mirrored L/R and forward/back was withdrawn after rechecking; the
screen-X mapping stays. Mount, angles and whether this was a bench or vehicle check were not
recorded; measurement and tuning items stay open.

- [ ] **Drive page physical check** — Needs hardware: on LCD OLD and AMOLED V2 (AMOLED lean/pitch signs, ZERO and return to zero user-confirmed 2026-11-18): auto-zero in flat, upright and handlebar mounts, lateral/forward signs against real motion, gyro power toggling and lean in real turns (compare against a phone lean app), 100 ms cadence, clock-style home legibility at a glance, Drive → DRIVE TOOLS → tool swipes and tile taps, ZERO taps, carousel position between Weather and Settings, trip view with an active route, call overlay, keep-awake and the eight-dot carousel. Native fixtures only cover math on synthetic vectors; QMI8658 gyro register settings are datasheet-based and unmeasured.
- [ ] **BLE ELM327 client with a real dongle** — Needs hardware: on AMOLED V2 and LCD OLD: discovery by name/service, connect while Chronos (Android) and ANCS (iOS) are linked, init and 0100 search time, PID values against a phone OBD app, ignition-off/ATRV fallback, reconnection after dongle power loss, one-minute hidden disconnect, display cadence during scans and heap/stack headroom. Native tests cover only framing, PID formulas and sequencing; obd_ble.cpp has no host build and NimBLE central/peripheral coexistence is unmeasured. 2026-11-25, user-confirmed on AMOLED V2 with a Vgate iCar Pro BLE 4.0 on a Fiat Punto 1.2 Fire (petrol/CNG): discovery/connect, PID values match a phone OBD app, coexistence with the phone link, reconnection after dongle power loss and the one-minute hidden disconnect; display cadence during scans looked fine but was not measured. Still open: LCD OLD, ignition-off/ATRV fallback, init/search times, heap/stack headroom.
- [ ] **LCD OLD build** — compile and install the Drive/OBD build on LCD OLD (only AMOLED V2 was built and flashed).
- [ ] **CI for Drive/OBD** — confirm the pinned Arduino CLI CI jobs for this change; only a local PlatformIO build ran.
- [x] **Host check for `obd_ble.cpp`** — syntax-checked by both runners against declaration-only NimBLE/FreeRTOS stubs in `tests/stubs/obd` (2026-11-24); real BLE behavior is still unverified.
- [ ] **Gyro scale** — Needs hardware: measure against a known rotation and the first samples after power-on; LCD OLD is unchecked.
- [ ] **Gyro noise and current** — Needs hardware: measure gyro noise, bias drift with temperature and the extra current while Drive is open.
- [ ] **Fusion tuning** — Needs hardware: tune constants on real rides: complementary weight 0.98 (about 2.5 s), turn threshold 0.15 rad/s, tare stillness 0.1 rad/s, G smoothing 0.35, pitch smoothing 0.25.
- [ ] **LCD OLD lean/pitch signs** — Needs hardware: its IMU placement may differ.
- [ ] **Forward/back direction** — Needs hardware: confirm FWD/AVANTI appears when the front tips down and BACK/INDIETRO when it rises, and that "ahead" matches the vehicle's forward direction and the G-ball direction under real acceleration/braking.
- [ ] **Vibration behavior** — Needs hardware: check handlebar vibration, potholes and long sweeping turns; decide whether a vibration filter or a higher gyro rate is needed.
- [ ] **ZERO after remounting** — decide whether a ZERO is needed after remounting, or whether to remember the reference per session.
- [ ] **Dongle compatibility** — Needs hardware: test at least two brands (e.g. OBDLink CX, Vgate iCar Pro BLE, vLinker BLE) and record names, service UUIDs and init/search times. Brand 1 of 2: Vgate iCar Pro BLE 4.0 works on AMOLED V2 (2026-11-25); UUID and timings not recorded.
- [ ] **BLE central coexistence** — Needs hardware: measure Chronos (Android) and ANCS (iOS) stability while Mochi is also a BLE central; check heap, worker stack (6 KiB) and display cadence during 4 s scans.
- [ ] **Dongle selection** — the last discovered dongle address is now remembered and preferred, else the strongest RSSI (2026-11-24); still needs a hardware check, and a picker or FORGET action if a wrong dongle gets remembered.
- [ ] **Pairing/PIN dongles** — support dongles that require pairing or a PIN (no security is configured for the client).
- [x] **OBD scanning setting** — SYSTEM → OBD dongle scan, USB `obd-scan`, web `obd_scan`; profile v11 (2026-11-24). Native tests only.
- [ ] **Dongle sleep** — Needs hardware: check that the one-minute hidden disconnect lets the dongle sleep and does not drain the car battery.
- [ ] **PID values across cars** — Needs hardware: verify against a phone OBD app on petrol, diesel and hybrid cars; some ECUs answer from several modules or need ATSH/protocol hints.
- [ ] **Trouble codes (extension)** — lamp and stored-code count from PID 0101 are shown (2026-11-24, native tests only); listing codes via mode 03 remains open. Never clear codes without an explicit design.
- [ ] **OBD speed (extension)** — cross-check lateral G with speed × yaw rate, and a speed-based lean estimate.
- [ ] **Wi-Fi ELM327 dongles (extension)** — TCP 192.168.0.10:35000; would replace home Wi-Fi and Internet weather while connected.
- [ ] **Direct CAN (extension)** — via the ESP32-S3 TWAI controller with an SN65HVD230-class transceiver on OBD pins 6/14, 12 V regulation and protection; hardware design needed.
- [ ] **Motorcycle support (extension)** — many bikes lack OBD-II; identify the partner's bike connector/protocol (K-line or proprietary) first.
- [ ] **Trip log (extension)** — persistence or export; trip time, peaks and graph are RAM-only today.
- [ ] **Fall/crash detection (extension)** — deliberately not implemented; it needs design, field data and validation before any safety claim.
- [ ] **Handlebar/dash mount and case (extension)** — answer whether the case model is open, then test vibration, sun legibility and water exposure. Touch controls are for parked use only.
- [x] **Commit the Drive work** — the Drive, gyro, carousel and OBD changes were merged into `main` (PR #10).
- [x] Functionally check QMI8658 gyro settings (CTRL3 0x56: ±512 dps, 64 LSB/dps, CTRL7 0x03 only while Drive is visible) on AMOLED V2: user-confirmed 2026-11-18.
- [x] Confirm lean and pitch signs on AMOLED V2: L/R and FWD/BACK follow the tilt with screen X as "right" (user-confirmed 2026-11-18).

## OTA and firmware updates

- [ ] **Browser OTA both directions** — Needs hardware: update both directions between two trusted OTA-capable firmware images via the browser, retaining saved settings and calibration.
- [ ] **OTA progress and coexistence** — Needs hardware: check upload progress, Wi-Fi/BLE coexistence and post-OTA Chronos reconnection on LCD OLD and AMOLED V2.
- [ ] **Reject bad images** — Needs hardware: renamed wrong-board, old pre-marker, full, corrupt, oversized and truncated images on real hardware.
- [ ] **Interrupted upload** — Needs hardware: test controlled upload interruption/recovery with USB recovery available; do not assume native tests prove power-loss safety.
- [ ] **Pending-image confirmation** — Needs hardware: on both physical boards, verify a pending OTA image remains unconfirmed until
  display/UI/BLE setup and fifteen seconds of healthy loop activity. Reset during
  that window and confirm return to the previous valid slot with the installed
  rollback-enabled bootloader; retain USB recovery.
- [ ] **microSD firmware updates** — Needs hardware: physically validate on AMOLED V2: valid alternate-slot install and retained settings/calibration, audio-to-update ownership handoff, missing/wrong/corrupt files, cancellation, real SD read failures, progress responsiveness and controlled interruption with USB recovery ready. Both builds, the full native suite and a USB installation passed on 2026-11-03; these are not proof of SD installation or power-loss safety.
- [ ] **Rollback and signing decision** — record whether boot-health rollback or signed firmware is required before wider deployment; neither is enabled.
- [ ] **Remaining trust changes** — track separately: signed images, LAN transport/client
  binding and physical approval, authenticated pairing and flash/NVS encryption.
- [ ] **Automatic OTA deployment** — implement if requested; binaries are not deployed automatically.
- [x] Install/update AMOLED V2 over COM6 using unchanged 16 MB OTA layout v1; verify image hashes, reboot and retained saved settings (2026-10-29).
- [x] Verify LCD OLD installation/migration separately and record calibration effects.

## Robustness, performance and diagnostics

- [ ] **Worst-case frame time and heap** — Needs hardware: measure worst-case frame time and extended-session heap behavior.
- [ ] **Memory headroom** — Needs hardware: measure internal largest-free-block and task stack headroom during Wi-Fi/BLE
  coexistence and repeated HTTPS work; calibrate the conservative 45 KiB gate.
- [ ] **Companion cache and task stacks** — Needs hardware: measure the new bounded companion cache and BLE/HTTPS/audio task stack
  headroom on LCD (no PSRAM) and AMOLED; native tests are not RAM profiling.
- [ ] **Physical fault injection** — perform it if required; native failure tests
  do not establish real allocation, bus, or storage failure behavior.
- [ ] **LCD emulator shutdown crash** — confirm the cause of two historical Windows LCD-emulator shutdown access violations. On 2026-11-05, 216 short and 16 long baseline runs did not reproduce them. Dummy-video sessions now bypass accelerated-driver probing, and partial/full SDL cleanup has regressions on both panels. This mitigates headless driver-lifetime exposure; the original crash cause remains unproven.
- [ ] **Persistent crash record** — decide whether one is needed; today the summary is
  USB-only and a disconnect loses it.
- [x] Preserve the LCD 1.69 OLD pin assignments and single shared I2C controller.
- [x] Keep PSRAM disabled on LCD 1.69 OLD: OPI claims GPIO33/35/36 and caused
  continuous beeping and I2C crashes on 2026-10-13.
- [x] Require OPI PSRAM for the AMOLED 368x448 canvas.
- [x] Cover both board profiles and multiple panel geometries in native tests.
- [x] Verify the extended SAVE/Back band on the tall AMOLED panel: both buttons are
  tappable to the panel edge, the four-pixel inert margin stays inert, and the band
  never activates the row above it.
- [x] Repeat newly added eye/desk features and audio checks on the LCD OLD board;
  AMOLED confirmation does not certify a different physical board.
- [x] Check bilingual navigation, read-only behavior and layout on three native panel geometries; verify captured heap pools, reset causes, long uptime and SD last-attempt states with controlled fixtures.
- [x] Verify SYSTEM > Diagnostics on both physical panels: scroll/BACK, live BLE/Wi-Fi transitions, heap values, reset reason and SD last-attempt state. Native tests do not establish physical behavior.
- [x] Add the CO5300 368x448 QSPI board profile, TCA9554 reset release,
  controller brightness, CST820 touch path, and shared I2C ownership.
- [x] Bring up the AMOLED V2 display, touch, IMU, and Chronos functionality.
- [x] Record touch response at `0x15` and working horizontal swipes.
- [x] Record user confirmation of functional operation, including the later general audio confirmation.

## Faces and animations

- [ ] **Silhouette/legibility pass** — Needs hardware: physically inspect on LCD OLD and AMOLED V2: the 25 reshaped classic faces (Squint through Distracted, see docs/FACE_LIBRARY.md) in all seven styles, legibility of lid shapes on MOCHI/TRAPEZ, larger symbol eyes without clipping, and the enlarged props on the appended scenes. Re-recorded raster hashes only prove native rendering is stable.
- [ ] **68 themed prop faces** — Needs hardware: physically inspect Snowfall through Astronaut (IDs 186-253) on LCD OLD and AMOLED V2: seven styles, complete cycles, palette restoration, Calm/night, prop legibility on the 1.69 panel, frame cadence with the larger catalog, and retained exclusions/favorites after loading a 186-face SAVE. Native rasters do not prove physical cadence or contrast.
- [ ] **Five anime faces** — Needs hardware: physically inspect Yandere, Nap Bubble, Gloom, Dot Eyes, Steam Blush on LCD OLD and AMOLED V2: seven styles, complete cycles, palette restoration, Calm/night, small-panel legibility of the bubble/ellipsis/steam, and retained exclusions/favorites after loading a 181-face SAVE. Native rasters do not prove physical cadence or contrast.
- [ ] **Nine car faces** — Needs hardware: physically inspect Wipers, Turn Signal, Fuel Low, Drift, Car Wash, Parking Sensor, Tunnel, Gear Shift, Check Engine on LCD OLD and AMOLED V2: seven styles, complete cycles, palette restoration, Calm/night, and retained exclusions/favorites after loading a 172-face SAVE. Native rasters and previews do not prove physical cadence or contrast.
- [ ] **More backdrops** — decide whether to add backdrops to the remaining scenery candidates (TRAFFIC, SAKURA, BAMBOO, UPSIDE_DOWN) and to frozen IDs 1-122, which would require re-recording the raster fixtures.
- [x] Physically validate rare peek/slipping-eye/fly gags, center tickle/edge dodge/repeated-touch sulk, unchanged tap/pet/hold/swipe actions, Calm/dashboard/night, and favorites edit/select/SAVE/reboot on LCD OLD and AMOLED V2. Built and uploaded to AMOLED V2 on COM6 on 2026-11-12; hashes verified and post-boot USB `show` returned the retained saved settings. Native checks passed after correcting the new LCD footer-boundary assertion and rerunning the UI/audio remainder; 84 native previews cover seven styles and both physical panel sizes. LCD firmware build/upload and physical touch/display acceptance remain unverified.
- [x] Physically inspect Rubber Band, Peekaboo, Popcorn, Spring Eyes, Comic Glasses and Balloon Eyes on LCD OLD/AMOLED V2: seven styles, complete gags, retained old exclusions, palette recovery, Calm and frame cadence. Native previews/tests do not prove physical smoothness or contrast. The 172-face batch was built and uploaded to AMOLED V2 on COM6; LCD build/upload and physical acceptance remain unverified.
- [x] Physically inspect the twelve anime-inspired faces (Power Up through Chibi Joy) and gentle seeded scene breathing/drift on both displays: seven styles, complete cycles, Calm/dashboard/night, finger-tracking priority, clean redraw, and retained exclusions after migrating a 154-face SAVE. Native tests and GIFs do not prove physical cadence, contrast or touch response. The subsequent 172-face batch was uploaded to AMOLED V2; LCD build/upload and physical acceptance remain unverified.
- [x] Physically inspect Hacker, Berserk, Portal, Origami, Ghost, Eclipse, Detective and Pixel Crash on both displays: all styles, complete cycles, palette restoration, Calm/playback scheduling and retained old face exclusions after SAVE/reboot. Native GIFs do not prove physical cadence or display colors.
- [x] Physically inspect the Mecha Break mask explosion and Samurai's unbroken sun backdrop on both panels, all styles and custom palettes. Native GIFs do not establish display cadence or physical contrast.
- [x] Physically inspect Sleepy/Curious/Shy/Suspicious/Love/Glitch refinements, single-D20 critical badges and quieter per-face randomized motion: all styles, Calm, tracking, palette restoration and clean display cadence. GIFs and native rasters are software previews only.
- [x] Compile and physically check the expanded robot gestures on LCD OLD/AMOLED V2: nine idle poses, emotional reopening without a duplicate blink, first-contact surprise, tilt excursion/rearm, Calm/dashboard/night, touch/pet/pickup/Dizzy priority and IMU opt-out. Native SDL captures and host checks do not prove physical sensitivity, smoothness or display cadence.
- [x] Compile and physically check the repaired symbols plus Devil/Halo/Angel on LCD OLD/AMOLED V2: colored interiors, two external Devil horns, Halo's aureole without wings, Angel's aureole and paired wings, retained configured bodies, hollow rings, palette restoration and no clipping during blink/gaze. Check Angel selection/SAVE and older allow-list migration. Native rasters do not prove physical display color or contrast.
- [x] Physically tune and validate affection easing and the pickup wake heuristic on LCD OLD/AMOLED V2: gentle strokes, return to rest, real lifts in both orientations, bumps/road motion, touch/Dizzy priority and IMU opt-out. Native traces and LCD captures do not establish physical pickup accuracy.
- [x] Compile and physically check random whole-cycle scheduling and live finger gaze on LCD OLD/AMOLED V2: no truncated performances, repeat variability, follow/release and flipped coordinates, unchanged tap/pet/swipe/Settings hold, and Shake recovery. Native checks and GIFs are not proof of physical timing or touch tracking.
- [x] Compile and physically check seeded face variation on LCD OLD and AMOLED V2: all styles, Calm/Dasai/Playful, dashboard/night intensity, open Moon/Fire, backdrop handoffs, frame cadence and clean palette restoration. Native timing/raster checks and GIF previews do not establish physical flicker or frame speed.
- [x] Compile and physically check Anime, Kitsune and Headlight DRL on LCD OLD and AMOLED V2: frame cadence, gaze/blink clipping, mood transitions, display/web/USB selection and SAVE/reboot. Native rasters do not validate display speed.
- [x] Verify the revised samurai's inclined eyes/blade on both physical panels, including entry/return without detached eye fragments.
- [x] Check tall UwU eyes/cheeks (no mouth) and neon glare appearance/frame time on both physical panels.
- [x] Check the seven themed backdrops (mecha armour, samurai sun, titan wall, yokai night, neon tubes, wheels road, uwu halos) on both physical panels: visibility, palette/night-theme blending, and no eye-ink occlusion.
- [x] Check Continuous playback's recent-ID/visual-family separation during an extended device session.
- [x] Recheck the user-reported temporary-face stall with Continuous, IMU mood and Shake enabled on AMOLED V2; native tests fix reproduced IMU starvation and verify bounded Shake recovery, not physical resolution.
- [x] Fix the eye-motion reference test by subtracting its same-time backdrop; the complete native runner reports all host tests passed (2026-10-28).
- [x] Verify the face-library web editor on a real phone/desktop browser: save,
  the "keep at least one face" rejection, the 409 revision conflict, and offline use.

## Build, CI and repository

- [ ] **Binary hash per acceptance report** — record the exact installed binary hash, phone OS and settings for each physical acceptance report. Inspect remote CI separately; local builds do not establish pinned Arduino CLI CI success.
- [ ] **Pinned core and NimBLE** — compile both board profiles with the shared core 3.3.12 and NimBLE 2.5.2 pins;
  inspect the new CI run, not a previous release run.
- [ ] **Split long files** — `settings_menu.cpp` is split (grid in `settings_grid.h`, 2026-11-24); split
  `OTA-bot-Mochi.ino` (435 lines) to stay under the 400-line guideline in
  `CONTRIBUTING.md`.
- [x] **Device photo or video** — the README now opens with two real-hardware GIFs recorded on a car dashboard (2026-11-23).
- [x] Compile both profiles with the pinned CI toolchain (Arduino CLI 1.2.0, ESP32
  core 3.3.11), checking the Wi-Fi and charging code, flash size and heap headroom,
  and record the result at the current HEAD.
- [x] Document Arduino CLI setup, dependency pins, build/upload commands, and native checks.
- [x] Add GitHub Actions native checks, firmware compilation, and downloadable binaries.
- [x] Document the AMOLED bring-up and user-reported board behavior.
- [x] Record local builds for source checkpoint `8b752ba`: PlatformIO 6.1.19, espressif32 8.0.0, Arduino core 3.3.12, Chronos 1.9.1, NimBLE 2.5.1, GFX 1.6.9 and ESP32Time 2.0.6; both board profiles compiled, LCD PSRAM disabled and AMOLED OPI enabled.
- [x] Run targeted ANCS parser, profile migration, web settings and three-panel layout tests after ANCS integration.
- [x] Confirm remote host and firmware jobs for both profiles:
  run 40499404191 at e0ac13ad78ba5467062553ea8833125f7ca6d140 succeeded.
  This does not validate later workspace changes; see `docs/CI.md`.

See `docs/UI.md`, `docs/SETTINGS.md`, `docs/BOARDS.md`, `docs/TESTING.md` and
`docs/AUDIT_REMEDIATION.md` for behavior, board details, reproducible checks and audit limits.

## Wi-Fi, phone and connectivity

- [ ] **Web IMU/audio actions and NTP failures** — Needs hardware: complete them on both boards: association/channel changes, invalid credentials, unreachable server, BACK/expiry, retained station ownership and time, and later Android/Chronos resync. Native SNTP tests do not exercise real radio or UDP.
- [ ] **AP v1-to-v2 migration** — Needs hardware: unchanged SSID, new twelve-character password,
  failed-write refusal, updated phone Wi-Fi credentials and scannable QR.
- [x] Test phone/desktop browsers, temporary WPA2 credentials, five-minute expiry, BACK and setup-AP shutdown; enabled saved station must remain connected.
- [x] User confirmed Wi-Fi and settings changes working on AMOLED V2 (2026-10-29); this supersedes the earlier access-only report, not the detailed failure/endurance matrix.
- [x] Remove automatic configuration-screen/AP opening; keep background connection and manual access only. Native startup/offline and manual-route checks passed.
- [x] Physically verify saved 2.4 GHz Wi-Fi across reboot, wrong-password/offline background retry without automatic configuration-screen opening, DHCP changes and authorized LAN QR, automatic NTP, city search, HTTPS/Open-Meteo forecasts, stale/offline cache, Forget, five-minute web expiry with station retained, and Wi-Fi/BLE/frame-rate/heap/power behavior on both boards.
- [x] Scan the Wi-Fi QR on LCD OLD and AMOLED V2 with Android/iOS cameras; verify eight-character password entry, QR/details switching, fresh sessions, rotation and manual fallback.
- [x] User confirmed ANCS notifications working on iPhone with AMOLED V2 (2026-10-29).
- [x] Record ANCS reconnect after Bluetooth toggles/reboot and permission changes on AMOLED V2; switch back to Android with SAVE/restart and recheck Chronos. Repeat delivery/reconnect on LCD OLD. Basic delivery confirmation does not prove every recovery case.
- [x] Verify Chronos pairing, disconnection, reconnection, and synchronization.
- [x] Display connection state, synchronized time/date, and 12/24-hour mode.
- [x] Show waiting/offline states and keep the local clock running when disconnected.
- [x] Verify the AMOLED device profile and Chronos operation.
- [x] Display app, timestamp, title, wrapped message, and automatic pagination.
- [x] Retain eight bounded local alerts with newer/older browsing and dismissal.
- [x] Verify alert browsing, dismissal, empty state, and new arrivals.
- [x] Show incoming calls and remove the overlay on dismissal/disconnection.
- [x] Keep notification/caller payload logging disabled.
- [x] Open Alerts on new notifications without repeated switches for ordinary updates.
- [x] Display city, current temperature, condition icon, and forecast highs/lows.
- [x] Display update age and handle missing/stale weather.
- [x] Preserve forecasts regardless of current/forecast packet order.
- [x] Rate-limit phone synchronization and support manual refresh.
- [x] Provide packet-counter diagnostics without logging location or payloads.
- [x] Confirm weather works correctly on the physical AMOLED V2.
- [x] Display turn directions, distance, ETA, and phone-supplied speed.
- [x] Render only complete received icons with a matching CRC.
- [x] Verify live phone navigation updates and inactive/disconnected states.
- [x] Provide large/details layouts and separate session overrides from saved defaults.
- [x] Automatically open Route on a real navigation start; preserve manual page choices on updates.
- [x] Validate Android Clock > Desk > Phone tools on both physical panels:
  find in both directions, timeout/stop, camera permissions/readiness, phone
  battery, eight alarms/repeats, water/movement windows, quiet/sleep across
  midnight, EN/IT ID association, hourly forecasts, sixteen-contact cache
  and nine QR slots. Repeat disconnect/reconnect and mute/night scenarios.
- [x] Verify iPhone mode hides the Chronos-only tile/actions and replaces
  Chronos clock/weather hints while retaining ANCS/AMS and Wi-Fi features.

Chronos was initially reported working on 2026-10-12. The user confirmed all
Chronos functionality and functional device checks on 2026-10-20. The confirmation
on 2026-10-20 supersedes the weather failure reported on 2026-10-13; weather is no
longer an open device issue.

## Power and battery

- [ ] **AXP2101 charging configuration** — implement if needed; voltage/current
  limits and safety settings are untouched. Read-only charge-start audio
  detection is implemented, with hardware checks pending.
- [ ] **Power consumption and runtime** — Needs hardware: measure power consumption and battery runtime.
- [x] Verify the AMOLED V2 PWR farewell on battery and USB: closing/fading eyes,
  cancellation on early release, ignored startup hold, and autonomous four-second
  cutoff with a stalled main loop. Native tests are not physical validation.
- [x] On AMOLED V2, verify the three-second farewell reaches the four-second PWR cutoff without an additional blank-screen hold; check early-release cancellation on USB and battery.
- [x] Verify AMOLED BOOT page cycling, wraparound, held/bouncing presses and
  suppression during startup, shutdown, visible calls and touch contact.
- [x] Add plausibility-gated battery ADC scaffolding and NOT MEASURED fallback.
- [x] Add adjustable brightness and optional idle dimming with touch wake.
- [x] Add AMOLED battery percentage from the AXP2101 battery-voltage ADC behind
  the same plausibility gate; charging configuration remains unimplemented.
- [x] Verify battery circuitry/divider calibration, the AXP2101 percentage reading
  and AMOLED PMU charge-start transitions on hardware; the new readers do not
  validate charger setup.
- [x] Verify the charge indicator on AMOLED V2: the bolt appears only after two
  stable charging samples, the percentage switches to the accent color, `--%`
  still appears without a plausible reading, the DEVICE STATUS bolt matches the
  header, and USB `show` reports `charging=on|off` across plug/unplug and
  read-error transitions. Native tests do not prove the PMU direction bits.
- [x] Confirm a saved version-nine profile survives the update to version ten on a
  real device: charging switch on by default, night schedule retained, and only an
  explicit SAVE rewriting the seventeen-byte record.
- [x] Verify the battery-with-bolt icon appears/disappears on stable charge changes
  without changing pages. Recheck AMOLED displayed 0%/100% against the reported
  nominal 3% shutdown/92% full-charge endpoints and compare Android Chronos
  percentage/charging status after connecting and reconnecting. Native raster
  and setter fixtures do not prove phone delivery or battery calibration.

## Audio and music

A native reproduction on 2026-10-21 found that 80 ms polling could silence short cues
while the long test remained audible. Bounded cues now use the PCM worker's sample clock
and DMA drain. The latest general user confirmation supersedes the remaining functional
complaint; the agent did not listen to or test physical audio.

- [ ] **SD audio measurements** — Needs hardware: measure SD read latency, speaker level, card removal/reinsertion and radio/display coexistence.
- [x] Implement the ES8311/I2S speaker backend with bounded, non-blocking PCM.
- [x] Confirm audible ES8311 bench tones at level 10 on 2026-10-19.
- [x] Record category/master/night-mute/level combinations during animations,
  display, touch and BLE, and a continuous session of at least two hours.
  This is missing structured evidence, not a newly reported sound failure.
- [x] Compile and audition all 20 eye sound variants in each of the five families on LCD OLD and AMOLED V2; check numbered SD WAVs, legacy/generated fallbacks, mute and BLE/display coexistence. Native tests do not establish physical audibility or SD latency.
- [x] Physically verify all 20 charge-start and 20 unplug variants on AMOLED V2, including numbered SD WAVs, charge completion with VBUS present, real cable removal, noisy/failed PMU reads, independent cooldowns and Charging/master/night mute. USB previews do not prove physical edge detection.
- [x] Verify Android Chronos title/artist and previous/toggle/next with real players.
- [x] Verify iPhone AMS alongside ANCS, bonding, service publication changes and reconnects.
- [x] Verify both panels' Music touch targets and long metadata on physical displays.
- [x] Verify AMOLED V2 SD WAV playback, fallback, master/SD/night/category mute and SAVE.
- [x] Verify the single mount with ten-second backoff: an absent or partially
  populated card must not remount on every cue or stall display frames, and a
  later insertion must recover without rebooting the device.
- [x] Verify startup.wav once per restart, charging.wav on a real stable charge
  start, five face-family WAVs and generic/built-in fallback; check mute/priority.
- [x] Verify phone volume controls on real Android/iPhone players and AMS percentage updates.
- [x] Verify automatic night at configured local-hour edges, missing time, NTP/phone corrections and manual override on both physical panels.

## Touch, IMU and settings

- [x] Physically verify larger Settings face-picker/count controls and Dice ROLL/DETAILS lower-strip taps on LCD OLD and AMOLED V2, including edges, finger drift, swipes, and return navigation. Native hit-boundary fixtures do not establish real touch comfort.
- [x] Physically check dice shake/cooldown with face shake disabled and Flappy Bird tap/collision/page-call pause behavior on LCD OLD and AMOLED V2. Host fixtures do not establish touch response, real shake sensitivity or display cadence.
- [x] Compile and physically check the Games layer below Scrivania and D&D dice on LCD OLD/AMOLED V2: type/count controls, maximum 100D100 legibility, result paging, navigation, phone-mode visibility and unchanged results after redraw. Native tests do not prove physical touch usability or random-source quality.
- [x] Verify single-pass and back-and-forth vertical petting with lateral drift below 64 pixels on both touch controllers; small drifting taps must remain taps.
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
