# Boards and panels

One codebase drives two Waveshare boards. The build selects the profile through
`pin_config.h`; the default keeps the current board byte-for-byte.

| Board | Profile | Panel | Interface |
| :--- | :--- | :--- | :--- |
| [ESP32-S3-Touch-LCD-1.69](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.69) (OLD revision) | default | ST7789 240x280 | SPI, PWM backlight on GPIO15 |
| [ESP32-S3-Touch-AMOLED-1.8](https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-1.8) (V2) | `-DMOCHI_BOARD_AMOLED_18` | CO5300 368x448 AMOLED | QSPI, brightness as a controller command |

The AMOLED profile targets revision **V2 only**. V1 uses an SH8601 controller and
different touch wiring; it is not covered.

## What a profile changes

- **`pin_config.h`** carries both boards: display, touch, I2C, buzzer, RTC, and
  power pins plus the panel geometry. Absent hardware becomes a capability
  switch, not a guess: `TP_RST=-1` (no touch reset pad), `LCD_BL=-1` (no
  backlight pad), `BUZZER=-1` (no PWM buzzer, ES8311 codec instead), and
  `BAT_ADC=-1` (battery would come from the AXP2101 PMU).
- **The sketch** picks the bus and driver (`Arduino_ESP32SPI`+`ST7789` vs
  `Arduino_ESP32QSPI`+`Arduino_CO5300` with the visible area at RAM column 16).
  On the AMOLED board it first releases the panel reset held by the TCA9554 IO
  expander at I2C `0x20` (bits 0..2); init commands sent while the panel is
  held in reset succeed silently and show a black screen. Brightness is routed
  through `backlight_bind_display()` into `setBrightness()` instead of LEDC.
- **Module guards** keep behavior identical on the default board: the touch
  reset pulse, backlight PWM, buzzer LEDC, and battery ADC compile out when
  their pin is `-1`; the sound service reports its normal failure state without
  a buzzer, and the battery settles on NOT MEASURED without an ADC divider.
- **Memory profile.** The AMOLED frame canvas is about 329 KB, so that profile
  compiles with `PSRAM=opi` (the board stacks 8 MB octal PSRAM). The 1.69 OLD
  board keeps `PSRAM=disabled`: OPI PSRAM claims GPIO33/35/36 there and caused
  the beeping/crash failure documented in `TODO.md`.

## Compile

```sh
# LCD board (default)
arduino-cli compile --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=disabled,FlashSize=16M" OTA-bot-Mochi

# AMOLED board (V2)
arduino-cli compile --fqbn "esp32:esp32:esp32s3:CDCOnBoot=cdc,PSRAM=opi,FlashSize=16M" \
  --build-property "compiler.cpp.extra_flags=-DMOCHI_BOARD_AMOLED_18" OTA-bot-Mochi
```

See `docs/TESTING.md` for the toolchain setup and `docs/UI.md` for the shared
layout rules.

## What is already proven (native)

Both profiles run the same host suites; the AMOLED size is not a mock:

- eye geometry: fit at 153% on a 368x448 raster, centered, inside the panel;
- pages and menus: the per-panel suite is compiled with
  `-DMOCHI_BOARD_AMOLED_18`, so the AMOLED branch of `pin_config.h`, battery,
  buzzer, touch reset, and backlight guards must build and link;
- the SDL emulator previews any geometry with `--size 368x448`.

## What still needs the physical AMOLED board (unverified)

The profile compiles, links, and passes every native check, but none of this
has run on hardware yet. In bring-up order:

1. Expander reset release at `0x20`, then confirm the panel answers (any
   visible fill in `init_screen()`).
2. Panel orientation: upstream `Arduino_CO5300` may need MADCTL `0xA0`; the
   flip setting in the dashboard rotates 180 degrees only.
3. CST820 touch at `0x15`: probe address, coordinate mapping on 368x448, and
   the `TP_INT=21` polarity.
4. Shared I2C membership on `SDA=15/SCL=14`: touch, AXP2101 `0x34`, codec
   `0x18`, expander `0x20`, RTC `0x51`, IMU `0x6B`.
5. Brightness curve of `setBrightness()` and idle dimming visibility.
6. Chronos metadata profile: version 1.9.1 has no 368x448 entry, so
   `CS_410x494_200_RTF` is used as the nearest rectangular one.
7. Sound stays in its failure state until an ES8311 I2S backend exists; the
   codec pins are reserved in `pin_config.h` [S11].
8. Battery/charging through the AXP2101 is not driven at all yet.
