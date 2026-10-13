# Contributing to OTA-Bot-Mochi

---

## Quick Start

1. Clone the repository:
```sh
git clone https://github.com/lorenzocamuso/OTA-Bot-Mochi.git
cd OTA-Bot-Mochi
```

2. Install Arduino IDE 2.x plus the ESP32 core by Espressif, and install `GFX Library for Arduino` and `ChronosESP32` from the Library Manager.

3. Open `OTA-bot-Mochi/OTA-bot-Mochi.ino` in Arduino IDE. Arduino builds the modules under `src/` recursively.

4. Select `ESP32S3 Dev Module`, enable USB CDC On Boot, and verify your board's flash/PSRAM options before building and uploading. See [Testing](docs/TESTING.md).

---

## Project Layout

- `OTA-Bot-Mochi.ino`: setup, loop, Chronos callbacks, screens
- `src/eyes/`: Eyes Engine sources, public API via `eyes_api.h`
- `src/ui/`: shared theme, BLE snapshots, utility screens, bounded text layout
- `src/clock/`: synchronized, region-based clock face
- `src/power/`: PWM backlight and host-testable brightness policy
- `src/touch/`: touch probing and horizontal gesture tracker
- `pin_config.h`: all pin mappings, fully remappable
- `gfx_helper.h` / `gfx_helper.cpp`: display helpers, palette, text layout
- `images.h`: splash, icons, bitmaps
- `FreeSansBold12pt7b.h`: UI font

---

## Style

- Use TABS for indentation
- Keep functions under 50 executable lines
- Keep files under 400 lines, split cohesive modules when needed
- Write all documentation in ENGLISH using Doxygen tags `@brief`, `@param`, `@return`
- Never use `delay()` or `Serial` inside `src/eyes`, use `millis()` for non-blocking timing
- Validate inputs at boundaries, never swallow errors

## Local Checks

Run `.\scripts\test-host.ps1` on Windows or `bash scripts/test-host.sh` on Linux.
These tests build native test executables, not ESP32 firmware. The fake display
checks text bounds; sensor tests exercise identity, acknowledged writes, range,
tilt, and shake behavior. Real display, BLE, and hardware behavior still require
the [device checklist](docs/TESTING.md).

---

## Pull Request Rules

- Write title and description in ENGLISH
- Use Conventional Commits (for example `feat(eyes): add sleepy blink rate`)
- Never commit secrets, keys, tokens, or personal data
- Keep changes focused and update docs when behavior changes
- Confirm a clean Arduino build and upload pass before requesting review
