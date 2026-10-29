# Face library

The allow-list applies to manual selection, automatic playback, taps, double taps,
petting, startup greetings, IMU reactions and shake-to-dizzy. At least one face
must remain enabled. Removing the selected face selects the first enabled ID.
An empty reaction pool leaves the current allowed face unchanged; a one-face
library never hangs trying to find a different expression.

## On the display

Open **Settings → FACES → Face library** (scroll down one row).
Tap a face to enable/exclude it; checked cells are enabled. Swipe vertically
through the list. SAVE persists the changes; BACK returns without undoing them.
The usual LOOK → Mood grid only selects enabled faces.

## From a browser

Tap **Wi-Fi** in the library. With saved Wi-Fi connected, scan the web QR from
the same home network. Otherwise join the password-protected network shown
on Mochi, then open `http://192.168.4.1`. The browser list supports search and restoring all
faces. SAVE FACES applies and saves the list, without saving unrelated RAM settings.
No account, internet access or external resources are needed.
The **Settings, Wi-Fi and weather** link opens display preferences, saved Wi-Fi,
automatic time and city selection; see [Web settings](WEB_SETTINGS.md).

This is a manually enabled, five-minute maintenance session, not a permanent web
server. Normal eyes, BLE servicing and utilities pause until BACK or timeout.
The existing USB settings terminal remains available in face/settings mode,
including manual network setup; it stays paused during firmware uploads.
Firmware uploads are rejected in face-library mode unless explicitly switching
to firmware mode from the settings page. Configuration-AP passwords and request
tokens are random per session and never saved. The separately saved home-network
credentials and station connection survive session close. Use only while parked.

The separate version-one `mochi/faces` NVS record contains a catalog length,
bitmap and checksum. The existing thirteen-byte v7 profile and its migrations
are unchanged. Missing/invalid records enable all faces; newly appended IDs are
enabled when loading an older valid bitmap. Defaults restores all faces in RAM.

## Local HTTP contract

Only available while the face-library Wi-Fi session is active, at Host
the displayed local address (AP or LAN). Responses are not cached. No CORS
permission is provided. AP HTML supplies a per-session `X-Mochi-Token` only
on its protected receiving interface; LAN JavaScript reads it from the display's
QR URL fragment. A LAN client cannot obtain it by forging the AP Host.

| Request | Contract |
| --- | --- |
| `GET /` | Self-contained HTML manager; rejects a different Host with 403 |
| `GET /faces` | Requires token; returns `names` and localized `labels` in stable ID order, lowercase hexadecimal `enabled`, numeric `revision`, and boolean `saved` |
| `POST /faces` | Requires token, `Content-Type: text/plain` and `X-Mochi-Faces-Revision` matching the current decimal revision; body is exactly the fixed-width lowercase hexadecimal bitmap |

Bitmap bit `id % 8` of byte `id / 8` means enabled. Bytes are encoded as two hex
digits each, in ascending byte order. For 135 faces the body has 34 characters;
the last byte's unused high bit must be zero. At least one bit must be set.

POST returns the same state on success (200) or failed storage (507, changes
remain live in RAM and can be retried). Invalid masks/body/type return 400 without
changing state, stale revisions return 409, and wrong/expired credentials, Host,
or session mode return 403. A successful save does not restart the device.
The revision detects another browser's edits; reload before overwriting them.

Native callback tests cover authorization, masks, revisions, persistence failures,
session timeout and upload rejection. They do not verify the ESP32 HTTP parser,
radio, phone Wi-Fi behavior or physical touch calibration.
