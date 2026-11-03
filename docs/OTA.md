# Local firmware updates

Mochi can receive a manually downloaded GitHub release application through a
temporary authorized LAN session or Wi-Fi access point. It does not contact GitHub, check for new versions,
or install anything automatically. Both board profiles use the same flow.
AMOLED V2 also supports a manually confirmed microSD installation without Wi-Fi.

## First installation

The first OTA-capable installation must use USB and the matching board profile.
The sketch's `partitions.csv` is the authoritative **16 MB OTA layout v1**;
Arduino CLI/IDE use the sketch-local file and PlatformIO references it explicitly.
It contains two 6.25 MiB application slots, NVS at `0x9000`, OTA metadata at
`0xe000`, SPIFFS and a coredump region. PSRAM settings remain board-specific.
Arduino CLI commands must also set
`--build-property "upload.maximum_size=6553600"` to align the build-time size
check with the custom CSV; see [Testing](TESTING.md).

Do not migrate the old partition table by uploading an application alone.
Back up settings and IMU calibration choices, then follow the full-image USB
procedure in [Installing](INSTALLING.md). A full image can erase both records.
Do not select a different partition layout in local builds. The updater refuses
to start if the running/inactive application slots do not match this layout.
Firmware compilation, uploading and physical validation remain user-operated.

## Update from microSD (AMOLED V2)

First install firmware that includes **Firmware from SD**, using USB or the
existing Wi-Fi updater. This is not a bootloader feature and cannot appear on
an older installation just by inserting a card. LCD OLD does not expose it.

1. Download the trusted release's **`mochi-amoled-18-v2-app.bin`** and verify its
   SHA-256 using the release checksum file. Do not use a full image, bootloader,
   partition table, ZIP, another board's firmware or an older image without the
   embedded board/layout marker.
2. On the same FAT-formatted microSD used for sounds, create `/mochi/update/`.
   Keep the filename unchanged:
   **`/mochi/update/mochi-amoled-18-v2-app.bin`**.
3. Save any changed preferences explicitly. Power Mochi off, insert the card,
   then power it on with stable USB power.
4. Open **Settings → SYSTEM → Firmware from SD** (Italian: **Firmware da SD**).
   Scroll to the last SYSTEM row, below Diagnostics.
5. Wait for the compatibility scan, then tap **START / AVVIA** to confirm the
   installation. Merely opening the screen or inserting the card never writes
   firmware. Checking and writing each show their own progress.
6. Keep the card and power connected. The image is written to the inactive
   application slot and validated by ESP-IDF before selecting it for boot.
   Mochi restarts after success. Check display, touch, Bluetooth and settings.

BACK cancels checking, confirmation or an unfinished write between chunks.
After an error, BACK and reopen the screen to retry. An incomplete write does
not select the new boot slot. Physical power-loss recovery still needs testing;
keep USB recovery available and do not remove power during installation.

The updater opens only the fixed application path for reading. It never formats,
renames, deletes or writes card files: `/mochi/sounds/` and the source firmware
remain intact. Leaving the image on the card does not reinstall it at boot;
every installation requires the menu and confirmation again. Flash NVS,
calibration, partition table and bootloader are not rewritten.

Audio pauses without changing saved sound preferences. A shared exclusive card
lease waits for the audio worker to close its files and unmount SDMMC before
maintenance mounts it. A busy/stalled audio worker fails safely after ten seconds
with a restart/retry message instead of accessing the card concurrently.
The whole manual session expires after five minutes. Reads/writes advance at
most 4096 source bytes per main-loop poll; SD driver and flash operations can
still stall a frame. Normal animations, phone processing and USB commands pause,
while touch/progress remain available between chunks. This mode starts no web
server or setup AP and works with SD sounds disabled.

Native tests cover both profiles, missing/invalid files, split board markers,
size/layout checks, explicit confirmation, cancellation, timeout/rollover,
changed source data, short reads, flash/finalization/boot-selection failures,
card ownership and EN/IT layouts on both panel sizes. These fixtures do not
validate real card removal, flash power loss, physical display responsiveness
or a completed on-device SD installation. Firmware signatures and anti-rollback
remain outside this feature, as with the browser updater.

Local validation on 2026-11-03: the full PowerShell native runner exited with
code zero and `All host tests passed`. Sequential PlatformIO LCD and AMOLED
builds passed with the pinned core 3.3.12; three existing ChronosESP32 warnings
remain (deprecated service start and two unused variables). The 2,476,976-byte
AMOLED application includes the SD updater and matching board/layout marker.
USB upload on COM6 verified all written hashes; the USB settings terminal
responded afterward with the same saved profile and `unsaved=no`. No physical
SD installation, card-removal/power-loss test or visual/touch check was performed.

## Update from a release

1. Before joining Mochi's network, download the desired version's checksum file
   and **one** application asset from
   [Releases](https://github.com/camiu01/OTA-Bot-Mochi/releases):
   - LCD 1.69 OLD: `mochi-lcd-169-app.bin`.
   - AMOLED 1.8 V2: `mochi-amoled-18-v2-app.bin`.
2. Check SHA-256 as described in [Installing](INSTALLING.md). Use trusted release
   assets only. Keep the original filename. Never select a `*-full.bin`, ZIP,
   bootloader, or partition-table image.
3. Save any changed settings explicitly. Keep Mochi on stable USB power.
4. Open **Settings → SYSTEM → Firmware update**. Swipe the SYSTEM rows upward
   to reach its fifth row, after Phone mode. The Italian label is **Aggiorna firmware**.
5. If Mochi is connected to saved Wi-Fi, keep the phone on that same private
   network and scan the displayed browser QR. It includes the current DHCP
   address and temporary authorization; do not share it.
   Otherwise scan the **Wi-Fi QR** to join `Mochi-…`.
   Alternatively, enter the displayed twelve-character password manually.
   **DETAILS / DETTAGLI** switches to the full text instructions; **WI-FI QR**
   switches back. Stay connected even if the OS reports “no Internet”.
6. On the setup AP, open **`http://192.168.4.1`** directly. On saved Wi-Fi,
   use the authorized QR link. Choose the application and upload.
   Both the browser and the device show progress. Do not disconnect power.
7. After integrity validation, Mochi selects the inactive slot and restarts.
   Verify the version, display, touch, Chronos and saved settings.

The session has a **five-minute total deadline**, including upload time.
BACK closes it when not uploading; after an error, close and reopen the mode
before retrying. Closing the session stops its HTTP server and setup AP,
but retains a configured station connection. Without saved Wi-Fi the radio
is switched off. See [Network](NETWORK.md) for manual setup and automatic reconnection.
Setup AP credentials are generated once per device and stored separately in
`mochi-ap/identity` (unencrypted NVS). The same `Mochi-…` name and password
serve both settings and firmware sessions. Forgetting the home network does
not change them; erasing device NVS does. Storage failure prevents AP startup
rather than silently generating temporary credentials. They are not printed over USB.
Passwords contain twelve random uppercase letters/digits from
`ABCDEFGHJKLMNPQRSTUVWXYZ23456789`, excluding ambiguous `0`, `1`, `I`, `O` and
lowercase letters. Each character uses five unbiased hardware-random bits.
The upload authorization token remains an independent 128-bit random value.
Secret generation requires the Bluetooth controller to be enabled; otherwise
maintenance refuses to start. It does not enable the ADC entropy source while
Bluetooth is running. Existing valid eight-character AP records are upgraded
once to a new twelve-character password while retaining the SSID. Re-scan the
Wi-Fi QR or forget the old saved AP password on the phone after this upgrade.
The Wi-Fi QR remains stable; it connects to Wi-Fi, not to the update
page. Open the displayed browser address after connecting.

QR generation reuses the ESP32 framework's bundled encoder, caches at most a
version-four matrix in RAM, and draws black/white modules at integer scale with
a four-module white quiet zone. It does not use the eye palette or print
credentials to the console. In AP mode, manual Wi-Fi connection instructions
remain available if encoding fails. LAN authorization requires the token-bearing
QR link; there is no separate manual passcode. Upload progress and errors replace
the QR automatically.

Normal animations, sound, page navigation, app synchronization and software
power-button handling pause during maintenance. USB settings commands remain
available in face/settings mode, but pause during firmware-upload mode. BLE callbacks can
still retain bounded snapshots, but the normal Chronos loop is not serviced.
Touch cancellation is unavailable while the synchronous HTTP upload is running.
Do not use PWR/RESET/BOOT during writing; physical power controls cannot be
disabled by firmware. The display orientation stays fixed during maintenance.

## Validation and limits

The upload must have the expected filename, declared size within the inactive
slot, an ESP32-S3 application header, and the embedded board/layout marker.
Renaming another board's firmware is not sufficient to pass. Older releases
(including v1.2.0) do not contain that marker and are rejected, as are builds
using another layout version. Firmware updates do not modify the partition
table, NVS, filesystem, or bootloader.

The complete image is validated by ESP-IDF `esp_ota_end()` before
`esp_ota_set_boot_partition()` selects it. Failed/interrupted uploads do not
select the new slot. This protects against incomplete writes, not every possible
power failure. Mochi defers Arduino's early confirmation and confirms a pending
image only after setup completed with display/UI and BLE initialization and
the main loop observed fifteen seconds of healthy operation. This requires
`CONFIG_APP_ROLLBACK_ENABLE` and `CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE` in the
installed framework/bootloader. A reset before confirmation can return to the
previous valid slot. This is not a complete peripheral self-test and does not
promise power-loss recovery. An older USB-installed bootloader may lack rollback;
an app-only update cannot change it. Keep USB recovery available and validate
both physical boards before relying on this policy.

WPA2 protects the isolated network. A random session token, exact Host check and
same-origin browser requests guard the upload endpoint; no CORS is enabled.
Denied uploads receive 403 without changing the owner's error or upload state.
Maintenance pages share no-store, frame denial, nosniff, no-referrer and CSP headers.
HTTP is not TLS. The board/layout marker and image checksum are **not a digital
signature**: someone with the session's local access and authorization can
install a compatible malicious image. Secure Boot, signed releases, anti-rollback,
Internet exposure and remote deployment are outside this implementation. Do not forward the service or share its password.

Host tests exercise both profiles, header/size validation, byte-split streaming,
wrong board, auth failures, write/finalization failures, timeout and delayed
reboot using fake transport/flash calls. They do not validate the real HTTP
parser, radio coexistence, flash power-loss behavior, or physical boards.
Complete the open checks in [TODO](../TODO.md) before relying on OTA.
Additional native checks cover the readable password alphabet, credential
renewal, Wi-Fi payload format, QR cache failures, integer scaling and quiet
zones on both panel profiles using a fake encoder. Screen tests also cover
EN/IT text bounds, QR/details switching, manual fallback and progress/errors.
These do not prove that a phone can scan the actual framework-generated QR on
either physical display.

Local validation on 2026-10-28: the complete native runner reported
`All host tests passed`, including all six OTA suites and the corrected
same-time backdrop comparison in the eye-motion reference test. All six
release-packaging tests also passed. The background runner's process exit code
was unavailable; this records its complete success output, not a verified exit code.
The browser form was checked at 390px and 1440px, with simulated upload,
progress and error callbacks; no real wireless upload was established by that
browser check.

## Local HTTP contract

The supported Host is the session's displayed IPv4 address (port 80):
`192.168.4.1` on the temporary AP, or the router-assigned address on saved Wi-Fi.
Requests must arrive through that exact local interface. There is no public API
or GitHub API dependency. The server exists only during a manual session.

| Request | Inputs | Result |
| --- | --- | --- |
| `GET /` | Exact displayed Host/interface | `200 text/html`; AP embeds its token, LAN takes authorization from the QR URL fragment; `Cache-Control: no-store`; wrong Host/interface: `403` |
| `POST /update` | Exact displayed Host/interface, `X-Mochi-Token` from the AP page or LAN QR fragment, decimal `X-Mochi-Size` equal to file bytes; multipart form with exactly one `firmware` file | `200 text/plain` only after validation and boot-slot selection, then reboot; `400` for invalid/incomplete images or flash errors; `403` for wrong Host/token or expired session |
| Settings/face/network routes | Authorized settings-mode session | See [Web settings](WEB_SETTINGS.md), [Face library](FACE_LIBRARY.md) and [Network](NETWORK.md); updates remain mode-isolated |
| Unknown paths | None | `404 text/plain` |

The browser supplies the multipart boundary and content type. No credentials
belong in Git. A retry after failure should begin with a new physical session.
