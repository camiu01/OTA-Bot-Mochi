# Local firmware updates

Mochi can receive a manually downloaded GitHub release application through a
temporary Wi-Fi access point. It does not contact GitHub, check for new versions,
or install anything automatically. Both board profiles use the same flow.

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
   to reach its fourth row. The Italian label is **Aggiorna firmware**.
5. If Mochi is connected to saved Wi-Fi, keep the phone on that same private
   network and scan the displayed browser QR. It includes the current DHCP
   address and temporary authorization; do not share it.
   Otherwise scan the **Wi-Fi QR** to join `Mochi-Update-…`.
   Alternatively, enter the displayed eight-character password manually.
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
Setup AP credentials are
generated per session, remain in RAM, and are not printed over USB.
Passwords contain eight random uppercase letters/digits from
`ABCDEFGHJKLMNPQRSTUVWXYZ23456789`, excluding ambiguous `0`, `1`, `I`, `O` and
lowercase letters. Each character uses five unbiased hardware-random bits.
The upload authorization token remains an independent 128-bit random value.
The Wi-Fi QR changes with the session; it connects to Wi-Fi, not to the update
page. Open the displayed browser address after connecting.

QR generation reuses the ESP32 framework's bundled encoder, caches at most a
version-four matrix in RAM, and draws black/white modules at integer scale with
a four-module white quiet zone. It does not use the eye palette or print
credentials to the console. If encoding fails, manual connection instructions
remain available. Upload progress and errors replace the QR automatically.

Normal animations, sound, page navigation, app synchronization, USB commands,
and software power-button handling pause during maintenance. BLE callbacks can
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
power failure or a valid image that crashes at boot. **Automatic boot-health
rollback is not implemented or promised by Mochi.** Framework defaults may
confirm a new image before the sketch runs; that is not a device health check.
USB recovery remains necessary.

WPA2 protects the isolated network. A random session token, exact Host check and
same-origin browser requests guard the upload endpoint; no CORS is enabled.
HTTP is not TLS. The board/layout marker and image checksum are **not a digital
signature**: anyone with physical access to the displayed network password can
install a compatible malicious image. Secure Boot, signed releases, anti-rollback,
Internet exposure, router credentials and remote deployment are outside this
implementation. Do not forward the service or share its password.

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

The only supported host is `192.168.4.1` (port 80), reachable on the temporary
access point. There is no public API or GitHub API dependency.

| Request | Inputs | Result |
| --- | --- | --- |
| `GET /` | `Host: 192.168.4.1` | `200 text/html`, local upload page containing a session token; `Cache-Control: no-store`; wrong Host: `403` |
| `POST /update` | `Host: 192.168.4.1`, `X-Mochi-Token` from the page, decimal `X-Mochi-Size` equal to file bytes; multipart form with exactly one `firmware` file | `200 text/plain` only after validation and boot-slot selection, then reboot; `400` for invalid/incomplete images or flash errors; `403` for wrong Host/token or expired session |
| Other paths | None | `404 text/plain` |

The browser supplies the multipart boundary and content type. No credentials
belong in Git. A retry after failure should begin with a new physical session.
