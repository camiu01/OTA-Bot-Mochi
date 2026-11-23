# OTA-Bot-Mochi documentation

[← README](../README.md)

## Documents

| Document | What it covers |
| :--- | :--- |
| [Editing and adding a face](ADDING_A_FACE.md) | Every existing expression has one file in `OTA-bot-Mochi/src/eyes/faces/`: 254 files, named after the canonical USB names. |
| [Original colored animations](ANIMATIONS.md) | The additions here are original procedural drawings. |
| [Audit follow-up — 2026-11-02](AUDIT_REMEDIATION.md) | Scope approved for this change: immediate corrections and runtime robustness. |
| [Boards and panels](BOARDS.md) | One codebase drives two Waveshare boards. |
| [Chronos phone tools](CHRONOS.md) | Android mode exposes **Clock → swipe down → Phone tools**, the eighth desk tile. |
| [GitHub Actions firmware workflow](CI.md) | The workflow is `.github/workflows/checks.yml`. |
| [Direct dependency and provenance inventory](DEPENDENCIES.md) | This is a bounded inventory, checked against `platformio.ini`, `.github/workflows/checks.yml` and installed package files on 2026-10-27. |
| [Contextual views](DESK.md) | The primary carousel contains eight pages: Eyes, Clock, Music, Route, Alerts, Weather, Settings and Drive. |
| [Face library](FACE_LIBRARY.md) | The allow-list applies to manual selection, automatic playback, taps, double taps, petting, startup greetings, IMU reactions and shake-to-dizzy. |
| [Export bot animations as GIFs](GIF_EXPORT.md) | The exporter runs Mochi's real eye renderer and animation update loop on the PC. |
| [Repeatable hardware validation](HARDWARE_VALIDATION.md) | Evidence levels and repeatable checks for validating firmware on real boards. |
| [Install a release safely](INSTALLING.md) | These are user-operated USB flashing instructions. |
| [Saved Wi-Fi, automatic time and Internet weather](NETWORK.md) | The ESP32 joins a user-saved 2.4 GHz network at startup using router DHCP (HTTP port 80, not port forwarding). |
| [Local firmware updates](OTA.md) | Mochi can receive a manually downloaded GitHub release application through a temporary authorized LAN session or Wi-Fi access point. |
| [Dashboard settings](SETTINGS.md) | The device is a Dasai-inspired dashboard companion, not a vehicle instrument. |
| [Audio, expressive motion, and language](SOUND.md) | The `BUZZER` pin is GPIO33, the `Buzz` net of the OLD board revision, matching the official Waveshare schematic and demo (`beePin = 33`; the new revision… |
| [Build and test](TESTING.md) | The runners require Python 3 (`python` on Windows, `python3` on Unix) for board profile contracts, including custom selection and rejected configurations. |
| [Companion UI](UI.md) | The living eyes remain the default screen. |
| [Local display and network settings](WEB_SETTINGS.md) | Open **Settings → SYSTEM → Wi-Fi settings** (sixth row, after Phone mode and Firmware update), or the face library's |
