# Local display and network settings

Open **Settings → SYSTEM → Wi-Fi settings** (sixth row, after Phone mode and Firmware update), or the face library's
**Wi-Fi** button. When connected to a saved network, scan the display's web QR
while your phone is on the same network. Otherwise join Mochi's displayed
configuration network, open `http://192.168.4.1` and follow
**Settings, Wi-Fi and weather** to `/settings-ui`.
Both modes use a five-minute session. Saved Wi-Fi stays connected afterwards.
It exposes every display preference, auto-rotation, IMU calibration, sound test
and an explicit switch to the existing firmware uploader. No account or external
web resources are used. Firmware mode continues to require the installed OTA layout.

API routes require the exact displayed Host and receiving interface, the current
`X-Mochi-Token`, a live settings session, no CORS, and no caching.
The LAN HTML never contains the authorization secret: it comes from the QR URL
fragment. Links preserve it; a new DHCP address rotates the token and updates
the display's QR. The configuration AP embeds its token only for clients on
that password-protected interface.
GET `/settings` returns a localized field schema,
values, `revision` (opaque string) and `saved`. POST `/settings` uses
`application/x-www-form-urlencoded`, every schema key exactly once, canonical
decimal integers, and `X-Mochi-Settings-Revision`. Validation is atomic. Excluded
moods are rejected. SAVE applies RAM settings then persists, like display SAVE.
200 returns current state; 507 retains live unsaved settings for retry; 400 rejects
invalid input; 409 means reload; 403 means wrong/expired session or mode.

POST `/settings/action` accepts one form field `action`: `calibrate`, `test` or
`firmware`. Calibration and sound retain the same availability/mute limits as the
display. GET `/settings` includes their status. Firmware switches maintenance mode
only after checking the installed layout, and the browser then reloads `/`.
Auto-rotation remains session-only, as on the display; IMU calibration uses its
existing independent save. Display rotation follows the new preference after
leaving maintenance mode.

Phone mode is saved with display preferences and takes effect only after a
manual restart. ANCS supplies iPhone notifications; Chronos runs in Android mode.

Network settings have a separate **Save network, time and weather** button,
independent revision, and atomic NVS record. City search is explicit and the
selected result is saved before automatic weather starts.
See [Saved Wi-Fi, automatic time and Internet weather](NETWORK.md) for the
network API, password handling, provider privacy, intervals and manual AP access.

POST `/ntp` with `action=sync` requests time over the saved connected network.
It returns 202, 409 when offline/busy, 429 for a refresh within 30 seconds,
or 503 when unavailable. GET `/ntp` returns `status` and `busy`.
The shared station remains connected on success, timeout or session close.

For legacy clients without managed Wi-Fi, POST `/ntp` still accepts four fields:
`ssid` (1–32 UTF-8 bytes), `password`
(empty for an open network, 8–63 bytes, or 64 hex digits), `server` (ASCII DNS
hostname, max 63 bytes), `zone` (`0` UTC, `1` Europe/Rome). It starts one bounded
asynchronous attempt and returns 202. GET `/ntp` returns `status` and `busy`.
POST while busy returns 409; unavailable clock binding returns 503; invalid
fields return 400. This legacy form never saves the supplied credentials.

For that legacy form only, Mochi temporarily uses AP+station mode, disables automatic reconnection,
and disconnects its home-network station after success, failure, cancellation or
timeout. Changing the station channel can briefly interrupt the browser AP link.
Reconnect to Mochi if necessary. The saved-network path does not disconnect
the station when the temporary web session ends.

NTP uses the core SNTP client with a 25-second overall limit, stopped after one
attempt. Its documented weak clock writer is overridden to publish only an
atomic epoch; only the main task applies valid time. Invalid/late/cancelled
replies cannot silently overwrite the shared clock. It shares the internal ESP32 clock used by Chronos, not the external
board RTC. The later phone/manual synchronization wins. Europe/Rome's daylight
offset is evaluated at synchronization; automatic attempts run every six hours.
Time is not guaranteed across complete power loss. Plain NTP is unauthenticated,
so only use a trusted server/network. No credentials are sent to the NTP server.
