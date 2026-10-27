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
commit, source ref/tag, run URL, CLI/core/library versions, resolved tool paths
and build profile, and `SHA256SUMS.txt` for the `.bin` files. The source ref is
a branch on non-release builds, not necessarily a tag.

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
It uses only artifacts from that run. Upload refuses existing asset names:
do not replace published firmware by rerunning attachment. Prepare corrections
under a new release tag instead. A release created by another workflow using `GITHUB_TOKEN`
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

## Inspected release evidence (2026-10-27)

The public GitHub tag API resolves **v1.2.0** to
`e0ac13ad78ba5467062553ea8833125f7ca6d140`, also the local starting HEAD.
The [release run 40499404191](https://github.com/camiu01/OTA-Bot-Mochi/actions/runs/40499404191)
reports **success** for `host`, `firmware`, and `release`. Its individual steps
include successful native/packaging tests and both Arduino CLI board builds.
These conclusions were read from the run's jobs API, not inferred from uploaded files.

The [v1.2.0 release](https://github.com/camiu01/OTA-Bot-Mochi/releases/tag/v1.2.0)
has the eight assets listed above: four binaries, two build records, flashing
notes and checksums. Both full images are 16,777,216 bytes. The seven published
checksum entries match the corresponding SHA-256 digests reported by GitHub's
asset metadata. This metadata comparison is **not** an independent download and
rehash of the binary bytes. Users should verify their own downloads.

Published AMOLED BUILD-INFO identifies commit/FQBN/profile but omits CLI,
core and library versions; the tagged workflow supplies those (CLI 1.2.0,
core 3.3.11, libraries listed above/in README). It also labels hardware
unverified, predating the consolidated functional-user report. Do not edit
those published assets: the local workflow improves metadata for the **next**
release and removes `--clobber`. At the time of local validation, this changed
workflow had not run remotely; a later push starts a separate run to inspect.

Release notes describe audio stabilization. The latest general user confirmation
supersedes the earlier audio complaint, not the need for a recorded endurance
test. Alert history is phone-notification history, not a persistent system-event
log. No exact installed release hash, settings matrix, two-board certification,
power measurements or two-hour result follows from those notes.

Local packaging tests are not proof that a future remote run or attachment will
succeed. Check every release's own run. For safe flashing, see [Installing](INSTALLING.md).
