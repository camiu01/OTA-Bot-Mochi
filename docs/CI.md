# GitHub Actions firmware workflow

The workflow is `.github/workflows/checks.yml`. It runs on pushes, pull requests,
and **Actions → Firmware and host checks → Run workflow**.

## Pipeline

1. **host:** Build and run all native regression tests, including real eye
   rendering against a raster substitute and automatic-face policies.
2. **firmware:** After tests pass, compile the complete ESP32 sketch with Arduino
   CLI. Core, CLI, and library versions are pinned in the workflow.
3. **delivery:** Attach the firmware output to the successful workflow run as
   `mochi-esp32s3-<commit SHA>`, retained for 14 days.

Core/library downloads and compiled objects are cached. Arduino CLI still runs
on every build and checks source changes; a cache hit does not skip compilation.
New runs cancel superseded runs on the same branch. No GitHub secret is needed,
and repository permissions are read-only.

The artifact includes the available binary/ELF output, `BUILD-INFO.txt` with the
commit and exact build profile, and `SHA256SUMS.txt` for the `.bin` files.

## Activate and download

Commit and push the workflow together with its required firmware, `scripts/`,
and `tests/` files. Review all local changes first; do not push only the workflow
while its new scripts and sources are still missing from Git.

In GitHub, open **Actions**, choose a successful run, and download its artifact.
Read `BUILD-INFO.txt` before using the binaries. The profile is ESP32S3 Dev Module,
USB CDC on boot, 16MB flash, and no PSRAM. PSRAM stays disabled because the OLD
board's buzzer and power-control pins overlap the octal PSRAM interface. See
[Testing](TESTING.md) for manual flashing and device validation.

This is continuous integration and delivery of a downloadable build, **not**
automatic deployment to the board. It does not publish releases, upload through
a serial port, or perform OTA. Do not flash the sketch application binary at
offset zero: bootloader, partition table, and application are different images.

No GitHub workflow run or firmware build of this revision has been completed
locally. A green Actions run is required before describing the firmware build
as verified; native tests alone do not establish that.
