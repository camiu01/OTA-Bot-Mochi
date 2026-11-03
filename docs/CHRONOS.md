# Chronos phone tools

Android mode exposes **Clock → swipe down → Phone tools**, the eighth desk
tile. These are contextual tools, not an eighth carousel destination. Horizontal
swipes still navigate the original seven main pages. Swipe up inside a tool to
return to Phone tools, then up again to return to Desk.

## Android features

| Feature | Behavior |
| --- | --- |
| Find Mochi | The app's find request creates a dismissible local alert and wakes the display. Sound obeys master/category/night mute. Requests are limited to one alert per five seconds. |
| Find phone | Start/stop from Phone tools; the local search stops after fifteen seconds while connected. A lost link can prevent delivery of a stop command; the library/app also has its own timeout. |
| Camera | Open the camera in Chronos first, then tap Take photo. Unavailable/offline requests are rejected. The firmware requests a capture; it does not receive or store images. |
| Phone battery | Subscribe on connection and display the phone's reported percentage and charging flag separately from Mochi's own battery. Missing values display `--%`. |
| Alarms | Eight RAM-only slots, configured from Chronos. Minute/hour/repeat values are validated. Monday–Saturday use bits 0–5, Sunday bit 6; 127 is daily and 128 is handled as a one-shot disabled locally after firing. |
| Reminders | Water and movement intervals follow their validated minute-accurate windows, including midnight wrap. Alerts fire once per due absolute minute. |
| Quiet/sleep | Both app schedules are combined with the local night policy. Remote schedule activity does not rewrite manual night settings or NVS. |
| Hourly weather | Twenty-four validated slots, showing temperature, icon, humidity, UV and wind. Old day-of-year data is explicitly marked stale. This uses phone-pushed forecasts, not additional HTTP requests. |
| Contacts | Read-only first sixteen contacts; the cache/received counts are displayed. Names retain at most 40 UTF-8 bytes and numbers 24 bytes. No dialling, contact modification or logging occurs. |
| QR links | Nine slots; exact valid UTF-8 payloads up to 96 bytes. Oversized payloads are rejected rather than truncated into a different QR. Encoding uses the framework component, version six maximum, with a white four-module quiet zone. Links are never automatically opened. |

Alarm and reminder alerts use the existing Alerts page, with its CLEAR action.
They are short optional cues, not a continuously ringing alarm clock. They
require a trusted phone/NTP clock, remain in session RAM, and need app sync after
reboot. Missed minutes are not replayed after a clock jump or maintenance.
The existing firmware maintenance loop pauses normal alarm/reminder processing;
do not rely on it as a safety-critical alarm.

Disconnected phone capabilities, contacts, QR data and hourly forecasts are
cleared from the owned snapshot. Already configured alarms/reminders and
quiet/sleep windows remain available while the local synchronized clock runs.

## Language association

Chronos 1.9.1 supplies an opaque numeric `CF_LANG` value without an authoritative
EN/IT ID table. The firmware deliberately does not guess that mapping.

1. Select English or Italian for the watch in the Chronos app and synchronize.
2. Open **Phone tools → App language**; the received ID is displayed.
3. Tap **SAVE EN** or **SAVE IT** to associate that exact ID with the desired locale.
4. Repeat for the other language if needed.

Only this explicit SAVE action writes the independent `mochi-chronos/language`
binding record. Later known IDs update the normal language preference in RAM;
use the existing settings SAVE to retain the chosen language across restart.
Unknown IDs leave the selected locale unchanged. Failed binding saves leave
the previous association intact.

## iPhone mode

The Phone tools tile and all Chronos-only actions/screens are hidden when
Phone mode is iPhone. ANCS notification and AMS music behavior remains available.
Clock/weather empty states direct the user to Wi-Fi setup instead of Chronos.
Phone battery subscription, find-phone, camera, app alarms/reminders, contacts,
QR and hourly phone weather are not advertised as ANCS features.

## Task ownership and validation

Chronos callbacks retain bounded owned content under a static mutex. They do not
draw, play audio, save preferences or issue BLE commands. A bounded 250 ms main-task
service consumes queued actions and evaluates schedules. Once-only occurrence
keys prevent replay when time moves backward. Personal phone data never enters logs.

Native companion tests exercise malformed packets, minute/day masks, duplicate
suppression, read-only caches, failed storage, disconnected actions and iPhone
guards. Both panel fixtures cover EN/IT text bounds, navigation and QR quiet zones.
The fixtures do not establish actual app support, BLE delivery, phone permissions,
camera operation, QR scanning, acoustic volume or real memory/stack headroom.
Firmware compilation/upload and physical acceptance remain user-operated.

Local verification on 2026-11-03: the full PowerShell host runner completed
all 60 executable checks and syntax checks with exit code 0; all nine Python
build-pin/packaging tests passed. The final iPhone command-discard guard was
also retested independently. EN/IT native previews were inspected on 240x280
and 368x448 after the button/footer corrections. `git diff --check` passed.

Library contract: [ChronosESP32 API](https://github.com/fbiego/chronos-esp32/blob/main/api.md),
checked against the installed 1.9.1 header and packet decoder.
