# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

Release notes on GitHub hold the full detail for each version.

## [Unreleased]

## [1.8.0] - 2026-11-25

See the [release notes](https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.8.0).

### Added

- OBD page settings: RPM gauge style (bar, shift light, dial, number only), max RPM and four selectable values,
  in the touch menu and browser settings.
- OBD values: intake air, engine load, manifold pressure, timing advance, short/long fuel trim, O2 voltage,
  ECU voltage, MAF, outside temperature, barometer and engine run time; only the selected ones are polled.
- Adjustable alert display time and back-to-Eyes idle time (or never).
- Desk tally counter and two-player scoreboard.
- Timer modes: tea (3 min), pasta (10 min), a repeating 30-minute break reminder with an alert, and a
  custom countdown set in one-minute steps (1-180 min).
- Eye styles PIXEL, ROUND (with a highlight) and VISOR.
- CAR / AUTO settings tab, before SYSTEM, for every OBD option.

### Changed

- Settings profile version 12 (25 bytes); versions 10 and 11 migrate with defaults.
- Desk tool menu uses a four-row grid.
- README and AGENTS.md rewritten.

## [1.7.0] - 2026-11-24

See the [release notes](https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.7.0).

### Added

- Drive dashboard with lean/G tools and a BLE ELM327 OBD client (PR #10).
- OBD view shows the check-engine lamp and stored-trouble-code count (PID 0101, read-only).
- **OBD dongle scan** setting (SYSTEM, USB `obd-scan`, web form); profile version 11.
- Remembered OBD dongle, preferred on later scans; otherwise the strongest signal.
- Stopwatch mode in the desk Timer; tap the digits to start/pause, hold to reset.
- Drive shortcuts: hold to ZERO; tapping the home reopens the last Drive tool; lean gauge on the home.
- Idle pages return to Eyes after 60 seconds without input, except Drive, an active route, a running timer, calls and unsaved Settings.
- Minimal dark restyle: neutral greys, eye color as accent only, amber warnings, outlined cards, centered buttons, toggle switches, status dots and progress bars.
- Host syntax check for `obd_ble.cpp` against declaration-only NimBLE stubs.

### Changed

- Expression grid split out of `settings_menu.cpp` into `settings_grid.h`.
- Device labels are uppercase in English and Italian (settings rows, face names, OTA/SD, diagnostics), matching the rest of the UI; the web settings form shows the same labels.
- Timer mode labels drop the `MODE:` prefix so they fit the button; STARGAZING stars are brighter.
- Documentation aligned with the shared camiu01 conventions: README layout, TODO grouped by area, this changelog format and the docs/ index.

## [1.6.0] - 2026-11-17

See the [release notes](https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.6.0).

## [1.5.0] - 2026-11-06

See the [release notes](https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.5.0).

## [1.4.0] - 2026-11-02

See the [release notes](https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.4.0).

## [1.3.0] - 2026-10-29

See the [release notes](https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.3.0).

## [1.2.1] - 2026-10-28

See the [release notes](https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.2.1).

## [1.2.0] - 2026-10-27

See the [release notes](https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.2.0).

## [1.1.0] - 2026-10-21

See the [release notes](https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v.1.1.0).

## [1.0] - 2026-10-20

See the [release notes](https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.0).

[Unreleased]: https://github.com/camiu01/OTA-Bot-Mochi/compare/v1.6.0...HEAD
[1.7.0]: https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.7.0
[1.6.0]: https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.6.0
[1.5.0]: https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.5.0
[1.4.0]: https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.4.0
[1.3.0]: https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.3.0
[1.2.1]: https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.2.1
[1.2.0]: https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.2.0
[1.1.0]: https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v.1.1.0
[1.0]: https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.0
