# TODO

## Working baseline

- [x] Build and upload the firmware with Arduino CLI.
- [x] Compile the eyes module from `src/eyes`.
- [x] Update color constants for the installed Arduino GFX library.
- [x] Buffer display rendering and limit updates to about 30 FPS.
- [x] Add horizontal swipe navigation between screens.
- [x] Verify smoother eyes and working swipe navigation on the device.

## 1. Chronos pairing and clock

- [ ] Verify pairing, disconnection, and reconnection with the Chronos phone app.
- [ ] Display the current Bluetooth connection state.
- [ ] Replace the clock screen header with synchronized time and date.
- [ ] Show a clear waiting state before the first time synchronization.
- [ ] Update the clock without clearing unrelated screen content.

## 2. Notifications and calls

- [ ] Display the notification app, title, and message.
- [ ] Handle long messages with wrapping or scrolling.
- [ ] Show incoming calls and clear them when dismissed.
- [ ] Show an empty state when there are no notifications.
- [ ] Disable full notification and caller logging by default.

## 3. Weather

- [ ] Display city, temperature, and weather icon.
- [ ] Display forecast highs and lows.
- [ ] Show when weather data was last updated.
- [ ] Handle missing or stale weather data.

## 4. Navigation

- [ ] Display turn directions, remaining distance, and ETA.
- [ ] Render the received turn icon and refresh it when its CRC changes.
- [ ] Show an inactive state when navigation ends.
- [ ] Verify live updates while navigating with the phone.

## 5. Battery and power

- [ ] Verify the board's battery sensing and power-management hardware.
- [ ] Read and display a measured battery charge level.
- [ ] Display charging state when supported by the hardware.
- [ ] Add adjustable backlight brightness.
- [ ] Add idle dimming or sleep and define how touch wakes the device.
- [ ] Measure power consumption and battery runtime.

## 6. IMU and hardware validation

- [ ] Verify QMI8658C detection, wiring, and accelerometer configuration.
- [ ] Confirm that tilting the device moves the pupils in the expected direction.
- [ ] Confirm shake-to-dizzy activation and restoration of the previous expression.
- [ ] Test swipe direction, edge touches, and repeated gestures on the device.
- [ ] Verify all eye styles and expressions.
- [ ] Test framebuffer allocation failure and missing touch-controller handling.
- [ ] Run an extended session with BLE connected and check for crashes or heap loss.

## 7. Documentation and repeatable checks

- [ ] Document Arduino CLI installation, dependencies, build, and upload commands.
- [ ] Document Linux serial-port permissions.
- [ ] Update the hardware documentation with the touch controller detected on the board.
- [ ] Correct README color examples to use the current `RGB565_*` constants.
- [ ] Add a documented command for the host-side swipe tests.
- [ ] Record the tested board options and library versions.
- [ ] Add automated firmware compilation and swipe tests.

The clock, weather, navigation, and notification screens currently render headers
only. Receiving data through Chronos does not yet display that data on these screens.
