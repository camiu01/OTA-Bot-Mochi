# Install a release safely

These are user-operated USB flashing instructions. After the first OTA-capable
USB installation, use [local wireless updates](OTA.md) with the matching
`*-app.bin` downloaded from a release. No firmware is uploaded by tests or builds.

## Choose the board first

| Physical board | Asset prefix | PSRAM |
| --- | --- | --- |
| Waveshare ESP32-S3-Touch-LCD-1.69 **OLD** | `mochi-lcd-169-` | Disabled; OPI conflicts with GPIO33/35/36 |
| Waveshare ESP32-S3-Touch-AMOLED-1.8 **V2**, CO5300 | `mochi-amoled-18-v2-` | OPI required for the canvas |

Do not use these images for LCD newer revisions or AMOLED V1/SH8601.
See [Boards](BOARDS.md) before buying or connecting a battery.

## Check the download

1. Open [Releases](https://github.com/camiu01/OTA-Bot-Mochi/releases), select a
   version, and download its `FLASHING.txt`, `SHA256SUMS.txt`, your board's
   `BUILD-INFO.txt` and chosen binary. Do not combine files from different versions.
2. Match the build commit to the tag and its successful Actions run. A green
   run establishes build/tests, not that this binary was installed on your board.
   [CI](CI.md) records the inspected v1.2.0 run and metadata limitations.
3. Compare each downloaded file's SHA-256 with its entry in `SHA256SUMS.txt`:

   ```powershell
   Get-FileHash .\mochi-lcd-169-full.bin -Algorithm SHA256
   ```

   Substitute the actual filename. On Linux, with all listed files downloaded,
   run `sha256sum --check SHA256SUMS.txt`. Checksums detect corruption; they are
   not a signature or hardware compatibility check.

## First installation or recovery

Use the matching **`*-full.bin`**, a merged bootloader/partition/application image.
The Arduino ESP32-S3 merged output starts at **`0x0`**, as used by the release's
`FLASHING.txt`. v1.2.0 full images are 16 MiB and include padding across NVS:
assume preferences and IMU calibration will be lost. Record settings first.
Do not run an additional erase-flash command.

Install esptool 5 in your Python environment (`python -m pip install esptool==5.4.0`),
use a USB data cable, and close serial monitors. Find the port with
`pio device list` or your operating system's device manager. Replace `COM3` below.
Run **only one** command, matching the board:

```powershell
# LCD 1.69 OLD only
python -m esptool --chip esp32s3 --port COM3 write-flash 0x0 mochi-lcd-169-full.bin
# AMOLED 1.8 V2 only
python -m esptool --chip esp32s3 --port COM3 write-flash 0x0 mochi-amoled-18-v2-full.bin
```

These commands write flash. Never unplug during writing. If the port is absent,
use the manufacturer's documented BOOT/RESET download-mode procedure for your
exact board, not guessed GPIO wiring or an erase workaround.

## Update an existing installation

For an installation already using the **16 MB OTA layout v1** and an OTA-capable
firmware, follow [the browser upload procedure](OTA.md). The first transition
from v1.2.0 or another pre-OTA layout requires USB installation of the full image.
Record settings first: a partition migration is not a settings-preserving update.

An **`*-app.bin`** contains only the application. It is not a first-install image
and must **never** be written at `0x0`.

For a conservative update, build the selected source tag and use the matching
Arduino CLI or PlatformIO upload procedure in [Testing](TESTING.md) and the
[README](../README.md#build-and-flash). Compare the existing and generated
partition layouts first. An unchanged NVS region normally retains preferences
and calibration; full images, erase-flash and layout changes can destroy them.
Settings migration is tested in software, not guaranteed by arbitrary flashing.

For manual app-only flashing, obtain the application offset from the **installed**
partition table and matching build configuration, and confirm the correct boot
partition. The published v1.2.0 assets do not include a standalone partition table
or active-slot record. Therefore this guide deliberately gives no universal
app-only flash command or guessed application offset. If that layout is unknown,
record your settings and use the destructive full-image procedure instead.

## After writing

Restart, keep the board still, and check `UI ready`, touch address and IMU status
at 115200 baud. Connect Chronos to `OTA bot` and synchronize time. Verify touch,
sound (off by default), orientation, saved preferences and IMU zero restoration.
An installed binary is not yet a verified device: record the
[hardware checklist](HARDWARE_VALIDATION.md), including exact version and board.
