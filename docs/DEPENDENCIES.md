# Direct dependency and provenance inventory

This is a bounded inventory, checked against `platformio.ini`,
`.github/workflows/checks.yml` and installed package files on 2026-10-27.
It is **not** a complete transitive audit, complete redistribution-notice
bundle, legal certification, hardware compliance assessment or animation-rights
clearance. The project's GPL license does not relicense third-party code.

| Direct component | Pinned version / source | License evidence and boundary |
| --- | --- | --- |
| [ChronosESP32](https://github.com/fbiego/chronos-esp32) | 1.9.1; installed `library.properties` | Installed `LICENSE`: MIT, copyright (c) 2023 Felix Biego |
| [ESP32Time](https://github.com/fbiego/ESP32Time) | 2.0.6; installed `library.properties` | Installed `LICENSE`: MIT, copyright (c) 2021 Felix Biego |
| [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) | 2.5.1; installed `library.properties` | Installed top-level `LICENSE`: Apache-2.0. Bundled `src/nimble/LICENSE` and `ext/tinycrypt/LICENSE` require separate retention/review; not a single-license transitive claim |
| [GFX Library for Arduino](https://github.com/moononournation/Arduino_GFX) | 1.6.9; installed `library.properties` | Installed archive has no top-level LICENSE. README identifies Adafruit_GFX, LovyanGFX, TFT_eSPI and Ucglib origins. Do not assign one guessed license to all source/font files; review file notices and upstream version before redistribution |
| [Arduino ESP32 framework](https://github.com/espressif/arduino-esp32) | CI 3.3.11; PlatformIO 3.3.12 (`framework-arduinoespressif32` 4.30312.261016) | Installed PlatformIO `package.json` declares LGPL-2.1-or-later; bundled ESP-IDF, SDK libraries and core components have additional terms, not audited here |

The local package evidence is under `.pio/libdeps/<environment>/<library>/`;
framework metadata is under PlatformIO's `packages/`. Those generated directories
are not vendored source or committed legal notices. Fetching a different version
requires checking its own license files again.

## Tools and bundled source/font provenance

- Build tools: Arduino CLI **1.2.0** in CI; Espressif32 PlatformIO platform
  **8.0.0**, local PlatformIO Core **6.1.19**. Local firmware compiler package:
  Xtensa ESP ELF **14.2.0+20260121**. Host tests use GCC **12.2.0** and C++17.
  Tool licenses are distinct from linked firmware dependencies.
- `OTA-bot-Mochi/FreeSansBold12pt7b.h` records generation from FreeSans Bold via
  `moononournation/ArduinoFreeFontFile`. The exact original font release and
  full glyph license were not established from that header. Preserve its notice;
  do not treat the GPL wrapper notice as a replacement for font provenance.
- The production UI uses Arduino_GFX's built-in bitmap font. Installed
  `src/font/glcdfont.h` identifies the classic Adafruit_GFX fixed-space font.
  Its origin needs inclusion in a complete graphics/font notice review.
- ES8311 initialization references a pinned Waveshare V2 example in [Sound](SOUND.md).
  A reference URL alone does not establish the license of every borrowed sequence.
- Existing bloub/Dasai/Cozmo-style credits are design references, not evidence of
  trademark permission or animation-rights clearance. No rights claim is added.
- Optional SDL2 emulator tooling is not linked into ESP32 firmware; its installed
  version and notices belong in an emulator distribution inventory if distributed.

## Before distributing a new binary

Retain dependency source notices and applicable license texts, inspect exact
versions (including transitive SDK/font code), resolve the graphics/font and
Waveshare provenance gaps above, and determine any source/notice obligations.
Do not substitute the author's copyright for upstream notices.
The repository currently includes its own `LICENSE`; this inventory deliberately
does not fabricate missing third-party license texts or certify sale readiness.
