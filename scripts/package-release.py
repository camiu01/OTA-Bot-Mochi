#!/usr/bin/env python3
"""Package the two Arduino build artifacts for a GitHub release."""
import argparse
import hashlib
from pathlib import Path
import shutil

FLASHING_NOTES = """Mochi ESP32-S3 firmware

Choose the files for your board:
- lcd-169: Waveshare ESP32-S3-Touch-LCD-1.69 OLD, PSRAM disabled.
- amoled-18-v2: Waveshare ESP32-S3-Touch-AMOLED-1.8 V2, OPI PSRAM.

*-full.bin is the merged bootloader, partition table and application image.
Flash it at address 0x0. It overwrites flash, including saved settings.
Use it for first installation or recovery; it is not a settings-preserving update.
*-app.bin is only the application, for an existing matching partition layout.
Never flash an app-only image at address 0x0.
For updates, verify the installed partition table and target application offset
first. Do not guess an offset or mix partition layouts, board profiles or PSRAM
settings. An app update with an unchanged NVS layout normally preserves saved
preferences; erase-flash and full images can remove preferences and calibration.

With esptool 5 installed, close serial monitors and replace COM3 with your port:
python -m esptool --chip esp32s3 --port COM3 write-flash 0x0 mochi-lcd-169-full.bin
OR, for AMOLED V2 only:
python -m esptool --chip esp32s3 --port COM3 write-flash 0x0 mochi-amoled-18-v2-full.bin
These commands WRITE the device. Do not run both or use erase-flash.
See docs/INSTALLING.md in the matching source revision for update instructions.

Check SHA256SUMS.txt and the board's BUILD-INFO.txt before flashing.
These files do not perform automatic OTA or update a connected device.
"""


def package(lcd: Path, amoled: Path, output: Path) -> None:
	"""Validate both builds before creating the release asset directory."""
	copies = []
	for board, source in (("lcd-169", lcd), ("amoled-18-v2", amoled)):
		for filename, suffix in (
			("OTA-bot-Mochi.ino.bin", "app.bin"),
			("OTA-bot-Mochi.ino.merged.bin", "full.bin"),
			("BUILD-INFO.txt", "BUILD-INFO.txt"),
		):
			path = source / filename
			if not path.is_file() or path.stat().st_size == 0:
				raise ValueError(f"Missing or empty firmware artifact: {path}")
			copies.append((path, f"mochi-{board}-{suffix}"))
	output.mkdir(parents=True, exist_ok=False)
	for source, name in copies:
		shutil.copyfile(source, output / name)
	(output / "FLASHING.txt").write_text(FLASHING_NOTES, encoding="utf-8")
	checksums = [
		f"{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}\n"
		for path in sorted(output.iterdir())
	]
	(output / "SHA256SUMS.txt").write_text("".join(checksums), encoding="utf-8")


def main() -> None:
	"""Read explicit build/output paths; never access a device or GitHub."""
	parser = argparse.ArgumentParser(description=__doc__)
	for name in ("lcd", "amoled", "output"):
		parser.add_argument(f"--{name}", type=Path, required=True)
	args = parser.parse_args()
	package(args.lcd, args.amoled, args.output)


if __name__ == "__main__":
	main()
