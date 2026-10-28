# Contributing to OTA-Bot-Mochi

---

## Quick Start

1. Clone the repository:
```sh
git clone https://github.com/camiu01/OTA-Bot-Mochi.git
cd OTA-Bot-Mochi
```

2. Install Arduino IDE 2.x plus the ESP32 core by Espressif, and install `GFX Library for Arduino` and `ChronosESP32` from the Library Manager.

3. Open `OTA-bot-Mochi/OTA-bot-Mochi.ino` in Arduino IDE. Arduino builds the modules under `src/` recursively.

4. Select `ESP32S3 Dev Module`, enable USB CDC On Boot, and verify your board's flash/PSRAM options before building and uploading. See [Testing](docs/TESTING.md).

Keep the sketch-local 16 MB OTA partition layout. The first installation over an
older layout needs the [USB migration procedure](docs/INSTALLING.md); an
application-only upload cannot migrate it.

---

## Project Layout

- `OTA-bot-Mochi/OTA-bot-Mochi.ino`: setup, loop and Chronos callbacks (module paths below are relative to this sketch directory)
- `src/eyes/`: Eyes Engine sources, public API via `eyes_api.h`
- `src/ui/`: shared theme, BLE snapshots, utility screens, bounded text layout
- `src/clock/`: synchronized, region-based clock face
- `src/power/`: PWM backlight and host-testable brightness policy
- `src/touch/`: touch probing and horizontal gesture tracker
- `src/update/`: local Wi-Fi firmware maintenance, image validation and QR screen
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

### Visual check for new or changed faces

When adding a face or changing its drawing, motion, colors, or backdrop, **generate
and watch its GIF before submitting the change**. Passing native assertions alone
is not enough. Use the [GIF exporter](docs/GIF_EXPORT.md):

```sh
# Linux; replace uwu with the new or changed face's canonical name.
./tools/export-gifs.sh --mood uwu --size 240x280 --seconds 12
./tools/export-gifs.sh --mood uwu --size 368x448 --seconds 12
./tools/export-gifs.sh --mood uwu --from mecha --size 368x448
```

On Windows, use `.\tools\export-gifs.bat` with the same arguments. Inspect a complete
animation cycle on both target panel sizes and all four styles (`--style mochi`,
`bloub`, `normal`, `trapez`). Check silhouette readability, intended colors,
blink/morph behavior, clipping, trails, and clean return to rest. For backdrop
changes, inspect transitions both into and out of the changed face; backgrounds
must fade without dimming the eyes or disappearing abruptly.

Include the checked faces, styles, panel sizes, and visual result in the PR, with
representative GIFs attached when possible. Keep generated exports out of commits.
GIFs are native software previews, not proof of physical display behavior.

---

## Pull Request Rules

- Write title and description in ENGLISH
- Use Conventional Commits (for example `feat(eyes): add sleepy blink rate`)
- Never commit secrets, keys, tokens, or personal data
- Keep changes focused and update docs when behavior changes
- For new or changed faces, complete and report the GIF visual check above.
- Run native tests and both sequential PlatformIO builds from `docs/TESTING.md`.
  CI independently builds with pinned Arduino CLI dependencies.
- Report hardware checks separately; an upload is not required for a code-only PR.
  Never claim physical validation from native mocks or a successful compile.
