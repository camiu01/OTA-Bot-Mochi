# GitHub Actions firmware workflow

The workflow is `.github/workflows/checks.yml`. It runs on pushes, pull requests,
published releases (including prereleases), and
**Actions → Firmware and host checks → Run workflow**.

## Pipeline

1. **host:** Build and run all native regression tests, including real eye
   rendering against a raster substitute and automatic-face policies.
2. **firmware:** After tests pass, compile both LCD and AMOLED profiles with
   Arduino CLI. Core, CLI, and library versions are pinned in the workflow.
   The PlatformIO environments in `platformio.ini` are not built by CI.
3. **artifacts:** Attach both builds to the workflow run as
   `mochi-esp32s3-<commit SHA>` and `mochi-esp32s3-amoled-<commit SHA>`,
   retained for 14 days.
4. **release:** Only on a published release, download both artifacts from this
   run and attach named binaries, build information, flashing notes, and checksums
   to that release. A failed test or build prevents this job from running.

Core/library downloads and compiled objects are cached. Arduino CLI still runs
on every build and checks source changes; a cache hit does not skip compilation.
New push/PR runs cancel superseded runs for the same event and branch. Release
runs are separate and are not canceled by a push or a newer run. No custom
GitHub secret is needed. Build jobs have read-only repository permissions;
only the release job has `contents: write` through `GITHUB_TOKEN`.

The artifact includes the available binary/ELF output, `BUILD-INFO.txt` with the
commit and exact build profile, and `SHA256SUMS.txt` for the `.bin` files.

## Activate and download

Commit and push the workflow together with its required firmware, `scripts/`,
and `tests/` files. Review all local changes first; do not push only the workflow
while its new scripts and sources are still missing from Git.

In GitHub, open **Actions**, choose a successful run, and download its artifact.
Read `BUILD-INFO.txt` before using the binaries. Both profiles use ESP32S3 Dev
Module, USB CDC on boot, and 16MB flash. The LCD OLD profile disables PSRAM
because its buzzer and power-control pins overlap the octal PSRAM interface.
The AMOLED V2 profile enables OPI PSRAM for its larger framebuffer. See
[Testing](TESTING.md) for manual flashing and device validation.

## Automatic release binaries

Commit and push this workflow, the packaging script, and its tests before
creating the release tag. In GitHub, create a release from a tag that includes
these changes, then choose **Publish release**. Saving a draft does not start
the build. Publishing a prerelease does.

After the workflow succeeds, the release's Assets section contains:

| File | Use |
| --- | --- |
| `mochi-lcd-169-app.bin` | Application for the LCD 1.69 OLD board |
| `mochi-lcd-169-full.bin` | Merged flash image for the LCD 1.69 OLD board |
| `mochi-amoled-18-v2-app.bin` | Application for the AMOLED 1.8 V2 board |
| `mochi-amoled-18-v2-full.bin` | Merged flash image for the AMOLED 1.8 V2 board |
| `mochi-*-BUILD-INFO.txt` | Commit and board build options |
| `SHA256SUMS.txt` | Checksums for binaries and accompanying notes |
| `FLASHING.txt` | Board selection, image types, and flashing warnings |

The workflow builds the release tag's commit, not the current tip of `main`.
It uses only artifacts from that run. Re-running the release job replaces
assets with the same names; it does not change the release notes or create a
second release. A release created by another workflow using `GITHUB_TOKEN`
does not trigger a new release workflow; this setup expects a user-published
release.

Flash a `*-full.bin` image at `0x0`. It contains the bootloader, partition table
and application, and can overwrite saved settings. An `*-app.bin` file requires
an existing matching partition layout; never flash it at `0x0`. Do not mix
LCD and AMOLED builds.

This workflow attaches builds to a release you publish. It does not create
releases, upload over USB, or deploy OTA to a board.

## Local packaging check

Run `python tests/release_package_test.py`. It uses fake artifacts to check board
names, contents, checksums, missing/empty input failures, and refusal to overwrite
an existing output directory. CI runs it before compiling firmware.

The release workflow has not yet been run on GitHub. Local packaging tests are
not proof that remote compilation, repository permissions, or release attachment
has succeeded. Check the release's Actions run before using its binaries.
