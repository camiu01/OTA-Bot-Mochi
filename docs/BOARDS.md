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
  their pin is `-1`. The AMOLED board instead uses the ES8311/I2S audio
  backend. Battery settles on NOT MEASURED without an ADC divider.
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

The same two profiles are also available as the PlatformIO environments
`esp32s3-lcd` and `esp32s3-amoled` in `platformio.ini`; see
[Testing](TESTING.md#platformio-alternative).

See `docs/TESTING.md` for the toolchain setup and `docs/UI.md` for the shared
layout rules.

## What is already proven (native)

Both profiles run the same host suites; the AMOLED size is not a mock:

- eye geometry: fit at 153% on a 368x448 raster, centered, inside the panel;
- pages and menus: the per-panel suite is compiled with
  `-DMOCHI_BOARD_AMOLED_18`, so the AMOLED branch of `pin_config.h`, battery,
  buzzer, touch reset, and backlight guards must build and link;
- the SDL emulator previews any geometry with `--size 368x448`.

## Physical AMOLED evidence and remaining measurements

The profile compiles, links, and passes every native check. On 2026-10-19 it
also built and uploaded with ESP32 core 3.3.12: the device reported UI/IMU
ready and touch at `0x15`; the user confirmed visible eyes, working horizontal
swipes, and audible ES8311 bench tones at level 10. This is partial bring-up,
not full validation of the checks below. Later user reports confirmed Chronos,
weather, navigation and IMU/UI operation; the latest general “everything works”
confirmation supersedes the earlier audio complaint. It does not establish an
exact installed commit, every settings combination, LCD coverage, a two-hour
session or power measurements. Use [Hardware validation](HARDWARE_VALIDATION.md)
to record the remaining per-board acceptance evidence. The following remain
bring-up/recheck details, not a list of newly reported failures:

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
7. ES8311 speaker playback: the opt-in backend uses the shared I2C bus and
   TX-only 16 kHz I2S, with amplifier enable on GPIO46. Confirm audibility,
   level, mute, and no touch/display interference using `sound test`;
   native tests cannot establish physical sound. See `SOUND.md`.
8. Battery/charging measurements and charging configuration through the AXP2101
   remain unimplemented. The PWR-key registers are configured separately below.

## AMOLED V2 PWR button

The side button marked **PWR**, not BOOT, uses the AXP2101's autonomous power
sequencer. At startup the firmware configures a **4-second hold to power off**
and a **1-second hold to power on**. Release the button between the two actions.
This is PMU power-off, not display blanking or ESP32 deep sleep. It requires a
valid USB or battery supply; removing all power also removes the PMU's state.
USB insertion can power the board on independently of the button.

After a fresh PWR hold reaches one second, the firmware previews shutdown with
the OTA BOT / MOCHI title, closing eyes and a three-second fade to black.
The farewell now fills the remaining hold up to the PMU's four-second cutoff,
rather than ending early and leaving a further hold on a black screen.
Keep holding through the farewell until power cuts. Releasing earlier
cancels the preview and restores the previous page. Boot-time holds and short
clicks do not start it. BLE, USB, touch polling and local timers keep running;
touch actions and automatic page changes are suppressed during the preview.
The PMU's autonomous cutoff remains enabled even if the firmware stalls.
The 40 ms event poll and display scheduling can offset the visual endpoint
slightly; timing and physical cutoff still require verification on the V2 board.

Configuration uses the existing shared I2C bus at address `0x34`, read-modify-write
and read-back verification of `0x22` (long-press enable, power-off rather than
restart), `0x27` (OFFLEVEL/ONLEVEL and one-second IRQ timing), and `0x41`
(PWR edge/long-press IRQ enables). The main task polls `0x49` every 40 ms and
acknowledges only those key events using write-one-to-clear semantics.
Read/acknowledgement failures cancel the preview rather than leaving the UI dark.
Thermal protection, unrelated IRQs, regulator voltages, rail enables and charging
settings are preserved. No GPIO is repurposed.

The configuration follows the official [AXP2101 datasheet](https://files.waveshare.com/wiki/common/X-power-AXP2101_SWcharge_V1.0.pdf),
sections 6.5.4, 6.13.2.20 and 6.13.2.25. Missing PMU, bus failure or failed
configuration produces a startup diagnostic and leaves the UI usable; button
behavior then depends on the PMU's existing configuration.

Power-off discards RAM-only edits, timers and BLE connections. Use explicit
**SAVE** before powering off to retain preference changes; the button does not
write preferences. A short press is not assigned a new firmware action.
Native tests verify registers and error paths, not physical switching or standby
current. Verify PWR off/on on battery and USB on the actual V2 board.

At runtime **BOOT (GPIO0, active low)** advances the six-page carousel once on
release, with 30 ms debounce and no repeat while held. The input uses a pull-up;
it is never driven as an output and its reset-time download-mode role is unchanged.
Startup/shutdown, visible call overlays and touch contact suppress the shortcut.
The LCD profile's BOOT behavior is unchanged.

## LCD 1.69 OLD PWR button

The [original board schematic](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.69/ESP32-S3-Touch-LCD-1.69.pdf)
confirms **SYS_EN=GPIO35**, driving the SS8050/AO3401 battery-power latch, and
**SYS_OUT=GPIO36**, the active-low **PWR/Key2** input. These are not the GPIO40/41
assignments of newer boards. BOOT and RESET retain their original functions.
OPI PSRAM must remain disabled because it conflicts with these original-board pins.

Hold PWR for **4 seconds, then release** to shut down. The startup press is
ignored until release; short clicks and contact bounce do not shut down.
The firmware asserts the battery latch before initializing storage/display.

- **Battery only:** shutdown blanks the display, silences audio, stops BLE and
  releases the power latch. Press PWR again to start, holding it until startup.
- **USB connected:** USB bypasses the battery switch, so physical power removal
  is impossible through that latch. The same action enters light sleep with a
  dark display and BLE/audio stopped. A PWR press wakes the CPU and restarts the
  firmware, restoring only explicitly saved settings.

GPIO36 is not RTC-capable on ESP32-S3, so deep sleep cannot wake from this PWR
input. GPIO light-sleep wake is armed and checked before stopping services.
If wake configuration fails, the UI remains running. A sleep-entry failure
restarts the firmware rather than leaving it dark and unresponsive.

Native tests cover hold/release/debounce/rollover, latch control, wake setup
failures and backlight shutdown on core 2/3. Physical switching, USB wake and
current consumption remain unverified on the original LCD hardware.
