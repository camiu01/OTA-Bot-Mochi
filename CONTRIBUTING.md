# Contributing to OTA-Bot-Mochi

---

## Quick Start

1. Clone the repository:
```sh
git clone https://github.com/lorenzocamuso/OTA-Bot-Mochi.git
cd OTA-Bot-Mochi
```

2. Install Arduino IDE 2.x plus the ESP32 core by Espressif, and install `Arduino_GFX_Library` and `ChronosESP32` from the Library Manager.

3. Open `OTA-Bot-Mochi.ino` in Arduino IDE. Project tabs auto-load `pin_config.h`, `gfx_helper.*`, `images.h`, and `src/eyes/`.

4. Select board Waveshare ESP32-S3-Touch-LCD-1.69, enable USB CDC On Boot, then build and upload.

---

## Project Layout

- `OTA-Bot-Mochi.ino`: setup, loop, Chronos callbacks, screens
- `src/eyes/`: Eyes Engine sources, public API via `eyes_api.h`
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

---

## Pull Request Rules

- Write title and description in ENGLISH
- Use Conventional Commits (for example `feat(eyes): add sleepy blink rate`)
- Never commit secrets, keys, tokens, or personal data
- Keep changes focused and update docs when behavior changes
- Confirm a clean Arduino build and upload pass before requesting review
