# TODO

Checked software items are implemented and covered where possible by native
tests. They do not imply that this firmware revision has passed an ESP32 build,
upload, or device test. Those checks are reserved for the user.

## Historical working baseline (before this revision)

- [x] Build and upload the firmware with Arduino CLI.
- [x] Compile the eyes module from `src/eyes`.
- [x] Update color constants for the installed Arduino GFX library.
- [x] Buffer display rendering and limit updates to about 30 FPS.
- [x] Add horizontal swipe navigation between screens.
- [x] Verify smoother eyes and working swipe navigation on the device.

## 1. Chronos pairing and clock

- [ ] Verify pairing, disconnection, and reconnection with the Chronos phone app.
- [x] Display the current Bluetooth connection state.
- [x] Replace the clock screen placeholder with synchronized time and date.
- [x] Show a clear waiting state before the first time synchronization.
- [x] Update the clock without clearing unrelated screen content.

Chronos operation was reported working on the device by the user on 2026-10-12.
Explicit disconnection/reconnection checks remain pending. The clock UI above is
implemented, but firmware compilation, upload, and device validation are pending
and will be performed by the user.

## 2. Notifications and calls

- [x] Display the notification app, title, and message.
- [x] Handle long messages with wrapping and automatic pagination.
- [x] Show incoming calls and clear them when dismissed.
- [x] Show an empty state when there are no notifications.
- [x] Disable full notification and caller logging by default.

## 3. Weather

- [x] Display city, temperature, and weather icon.
- [x] Display forecast highs and lows.
- [x] Show when weather data was last received.
- [x] Handle missing or stale weather data.
- [x] Preserve forecasts when current/forecast packets arrive in either order.
- [x] Add rate-limited app synchronization after pairing and a manual refresh.
- [x] Add USB packet-counter diagnostics without logging city/location or payloads.
- [ ] **Priority: resolve weather on the physical device.** The user reported it
  not working on 2026-10-13. Parser/sync changes are host-tested, not device-verified.
  Check Chronos weather configuration, connectivity, permissions, and received
  packet counters before claiming the problem fixed.

## 4. Navigation

- [x] Display turn directions, remaining distance, and ETA.
- [x] Render the received turn icon only after complete packets and matching CRC.
- [x] Show an inactive state when navigation ends.
- [ ] Verify live updates while navigating with the phone.
- [x] Add large essential navigation with a scaled 96x96 received icon.
- [x] Keep temporary route format separate from the explicitly saved default.
- [x] Open Alerts on new notifications and Route on real navigation start.
- [x] Declare the nearest supported rectangular Chronos profile without changing
  the physical 240x280 canvas.
- [ ] Verify that Chronos refreshes its old round-device metadata after upload.

## 5. Battery and power

- [ ] Verify the board's battery sensing and power-management hardware.
- [x] Start battery percentage scaffolding: plausibility-gated `BAT_ADC` reading,
  Device status row, and USB `show` battery line, reporting NOT MEASURED when the
  divider/battery is absent or implausible.
- [ ] Confirm the divider ratio and calibrate the percentage against a known pack;
  native conversion tests are not physical validation.
- [ ] Display charging state when supported by the hardware.
- [x] Add adjustable backlight brightness.
- [x] Add idle dimming and define how touch wakes the device, without stopping BLE.
- [ ] Measure power consumption and battery runtime.

## 6. IMU and hardware validation

- [x] Correct the IMU to the shared I2C bus (SDA=11, SCL=10) after the official
  schematic showed QMI8658C on the touch/RTC bus and GPIO9/GPIO12 unconnected;
  the previous SDA=12/SCL=9 mapping made startup report "IMU unavailable".
- [x] Check IMU identity and acknowledged configuration; explicitly match range to scaling.
- [ ] Verify QMI8658C detection, wiring, and accelerometer configuration.
- [ ] Confirm that tilting the device moves the pupils in the expected direction.
- [ ] Confirm the cornering gaze: the ±14 px clamp, the ±12/±16 px gains, the 0.1 g
  recenter threshold, and the sign of the ±4 px eye-pair roll all depend on how the
  board is mounted, so they need tuning on the actual mounting.
- [ ] Confirm shake-to-dizzy activation and restoration of the previous expression.
- [ ] Test swipe direction, edge touches, and repeated gestures on the device.
- [ ] Verify all 88 eye styles and expressions on the physical LCD; native mocks only
  prove bounds, symmetry, and distinctness.
- [ ] Test framebuffer allocation failure and missing touch-controller handling.
- [ ] Run an extended session with BLE connected and check for crashes or heap loss.

## 7. Documentation and repeatable checks

- [x] Document Arduino CLI installation, dependencies, build, and upload commands.
- [x] Document Linux serial-port permissions.
- [x] Document supported touch addresses without asserting an unverified chip model.
- [ ] Update the hardware documentation with the touch controller detected on the board.
- [x] Correct README color examples to use the current `RGB565_*` constants.
- [x] Add documented host-side swipe, UI, brightness, and IMU test commands.
- [x] Record installed core/library versions and the proposed board profile.
- [ ] Record the tested board options and library versions.
- [x] Add automated firmware compilation and native tests as a GitHub Actions workflow.
- [ ] Confirm the firmware CI job passes after publication.
- [ ] Verify a different panel or board end to end: `LCD_WIDTH`/`LCD_HEIGHT` in
  `pin_config.h` re-flow the layout macros and the eye pair (native tests cover
  240x320, 320x480, 128x160, and 160x128 rasters), but the touch controller
  resolution, the ST7789 offset tuple in the sketch, and the Chronos screen
  profile remain hardware details that mocks cannot prove.

## 8. Visual design

- [x] Use shared dark cards, eye-color text/icons/controls, and screen dots.
- [x] Enlarge touch targets, fill control gutters, and scroll larger settings rows vertically.
- [x] Use consistent spacing and bounded text across all utility screens.
- [x] In dashboard mode, keep calls on Alerts; optionally overlay other pages.
- [x] Use neutral placeholders rather than inventing missing navigation or battery data.
- [ ] Validate appearance, text readability, and touch controls on the physical LCD.

## 9. Eyes, dashboard preferences, and desk tools

- [x] Distinguish tall solid MOCHI, wide pupilled BLOUB, and mirrored solid TRAPEZ.
- [x] Keep MOCHI/TRAPEZ pupil-free, including whole-eye love/dizzy silhouettes.
- [x] Add bounded random/IMU reactions and a three-second random face on a short tap.
- [x] Add blink-covered transitions, damped gaze, double blink, wink, glance,
  curious lean, bounce/stretch, and nod; reduce intensity in dashboard/night modes.
- [x] Add Clock's contextual menu: big clock, overview, local timer/Pomodoro,
  and non-sensitive device status, without lengthening the six-page carousel.
- [x] Add touch/USB settings and explicit NVS saves with legacy profile migration.
- [x] Add English/Italian UI labels and saved language selection.
- [x] Add petting affection reactions, double-tap playful faces, tap-directed
  glances, and six-second extended petting on Eyes.
- [x] Add the version-five Screen flip setting for 180-degree mounting, with the
  canvas, touch coordinates, and IMU gaze following the rotation.
- [x] Add a session-only status-bar sound toggle, a swipe-down quick return to
  Eyes, a silent startup greeting, and a USB reboot verb.
- [ ] Validate petting, double-tap, glance, Screen flip, status-bar mute,
  swipe-down home, boot greeting, and USB reboot on the physical board.
- [x] Validate the build memory profile on the physical board: OPI PSRAM is
  rejected because it claims the OLD revision's buzzer (GPIO33) and
  power-control (GPIO35/GPIO36) pins, causing continuous beeping and I2C
  driver crashes; the shipped profile keeps PSRAM disabled.

## 10. Buzzer and delivery

- [x] Add opt-in, non-blocking PWM/tone cues on the board buzzer GPIO33.
- [x] Add face tap/automatic reaction, timer/rest/end, notification/call, and
  rate-limited spontaneous sound categories with separate switches.
- [x] Default sound off; default automatic night mute on; bound the duty level.
- [x] Keep audio/backlight PWM allocations separate; cover core 2/3 API branches
  with host tests and stop silently on allocation/write failures.
- [x] Add explicit AUDIO/USB bench tones and off/night-mute/PWM diagnostics.
- [x] Add cached GitHub Actions builds and downloadable firmware artifacts.
- [x] Correct the buzzer pin from GPIO34 to GPIO33, the `Buzz` net of the OLD
  board schematic and the `beePin = 33` value in the official demo.
- [ ] Verify buzzer audibility, pitch, duty level, and lack of backlight
  interference on the actual OLD board before enabling sound in the car.
- [ ] Confirm a green remote firmware CI run, then build/upload and validate the
  full revision on the board. No current ESP32 build/upload has been performed.
- [ ] Automatic OTA deployment and calibrated battery/charging remain deferred.

## 11. Panels and boards

- [x] Adapt eye geometry, lids, touch bounds, and the UI layout to other
  panel sizes: eyes fill larger panels via `EYE_FILL_EXCESS` with a per-style
  fit cap, and the widget design space scales with `ui_s()`; prove both on
  240x280, 240x320, 320x480, 368x448, 128x160, and 160x128 rasters with the
  native suite.
- [x] Add the second build profile for the Waveshare ESP32-S3-Touch-AMOLED-1.8
  V2 (`-DMOCHI_BOARD_AMOLED_18`): QSPI CO5300 368x448, TCA9554 reset release,
  controller brightness, CST820 touch pins, capability guards for missing
  backlight/buzzer/battery paths. See `docs/BOARDS.md` for the unverified
  hardware checklist (panel orientation/MADCTL, touch mapping, shared-bus
  membership, brightness curve, Chronos profile, ES8311 sound, AXP2101).
- [ ] Compile the AMOLED profile with the pinned toolchain and review the
  memory report: the 368x448 canvas needs PSRAM=opi (8MB octal), unlike the
  1.69 OLD profile which must keep PSRAM disabled.
- [ ] Bring up the AMOLED board physically in the order listed in
  `docs/BOARDS.md`; do not mark any of those checks complete from native runs.
- [x] Implement opt-in ES8311/I2S speaker cues for the AMOLED V2 board using
  the shared I2C bus, a bounded PCM worker, immediate amplifier mute, and
  native codec/PCM/failure tests.
- [x] Verify ES8311 bench-tone audibility at level 10 and working horizontal
  swipes on the physical AMOLED V2 board (user-confirmed 2026-10-19).
- [ ] Verify ES8311 volume range, master/night mute, and extended simultaneous
  display/touch/BLE behavior on the physical AMOLED V2 board.
- [ ] Implement an AXP2101 battery driver if needed; the AMOLED battery still
  reads NOT MEASURED.

See `docs/UI.md` and `docs/TESTING.md` for controls, dependency pins, native
coverage, and the remaining physical validation checklist.
