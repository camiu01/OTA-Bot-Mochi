# Companion UI

The living eyes remain the default screen. Utility screens use a black background,
rounded tinted cards, eye-colored text and icons, and dimmer secondary labels.
The eye color is the shared primary color, including controls, calls, and weather.
Six position dots indicate the selected
screen; the header shows a speaker icon (crossed out while sound is off or
night-muted), Bluetooth state, and the configured backlight percentage.
This percentage is **brightness, not battery charge**.

## Controls

- Swipe left/right through Eyes, Clock, Route, Alerts, Weather, and Settings.
- Swipe **down** on Clock, Route, Alerts, Weather, or any desk tool to return
  straight to Eyes. Vertical strokes still pet Mochi on Eyes and still scroll
  rows inside Settings.
- Tap the top status strip, above the page title, on Clock, Route, Alerts,
  Weather, or any desk tool to mute or unmute every sound for the session;
  unmuting chirps once. The strip is inactive on Eyes (where taps are face
  reactions) and inside Settings. This edits the RAM profile like the AUDIO
  tab does, so a Settings SAVE persists it too.
- Clock → DESK TOOLS opens the large clock, overview, local timer, and device
  status without adding them to the primary swipe carousel. See [Desk tools](DESK.md).
- A short tap on Eyes requests a temporary random face and makes the eyes
  glance toward the touch point for a couple of seconds. It does not change
  saved preferences or require automatic random moods to be enabled.
- Double-tap quickly on Eyes for a playful face from a separate excited/star
  pool, with the same glance toward your finger. Dizzy keeps priority.
- Stroke vertically up and down on Eyes to pet Mochi: rubbing the screen
  without drifting sideways requests a temporary affectionate face from the
  happy/love family, and a small heart marker appears near your finger for
  under a second as confirmation. Continued strokes keep the same face and
  extend it up to six seconds. Petting never changes saved preferences,
  leaves Eyes, or opens Settings. Use vertical strokes; horizontal motion
  pages away.
- Eyes has no header, so a small speaker glyph appears in its top-right
  corner only while audio cannot sound (off, night-muted, or PWM failure).
- Settings → LIGHT → **Screen flip** mounts the whole interface upside down
  for bottom-up installations. Touch targets and the IMU gaze follow the
  180-degree rotation, and the choice is saved with the profile.
- On startup, Mochi greets with a brief three-second temporary face played
  silently; the selected mood returns automatically.
- New notifications automatically open Alerts, including while a route is active.
  Starting actual phone navigation automatically opens Route. Regular route/icon
  updates do not pull you back after a manual swipe. If both events arrive in the
  same loop, starting navigation takes priority. Switching waits for a held finger
  to be released, avoiding a page change underneath a settings tap.
- Hold still for 1.2 seconds on the eyes screen to open Settings. A swipe or
  moving finger is not also treated as a tap or hold.
- Use settings only while parked. The firmware cannot determine whether the
  vehicle is stationary and does not lock controls based on vehicle speed.
- Tap LOOK, LIGHT, DRIVE, MOVE, and AUDIO to change appearance and comfort options.
  Tap SAVE to persist them, then EYES to return. Changes apply immediately in RAM.
- Settings show three larger rows at a time. Swipe vertically to reach remaining
  rows; the scroll indicator shows the position. Tabs and SAVE/EYES stay fixed.
  Vertical scrolling neither changes a value nor advances the page carousel.
  Every row card owns its own 44-pixel touch zone aligned to the visible card,
  so touching the bottom of a row never activates the next one. The Mood grid
  shows five rows of 72x24-pixel cells as contiguous full-height targets; only
  the thin column gutters stay inert. Settings categories use previous/next
  arrows with 72x38-pixel targets below the title, a central category name and
  a 1/5 position indicator. They wrap around and reset panel scrolling; tapping
  the name does nothing. SAVE/Back each owns a 120x58-pixel
  target extending to the bottom edge. Their visible buttons are enlarged too,
  with no overlapping actions. Short taps tolerate up to 18 pixels of movement
  and 800ms contact time; longer stationary holds still use 1.2 seconds.
- The default is MOCHI, white on black, neutral mood, dashboard mode, 30% day
  brightness, shake disabled, and idle dimming disabled.
- Night mode is manually selected, with a separate 1–20% brightness (default 5%).
  There is no ambient-light sensor or automatic day/night detection.
- Optional idle dimming caps brightness at 3% after a minute, never increasing
  an already lower night setting. Touch restores it; BLE continues running.
- Active routes and explicitly displayed calls inhibit dimming. Hidden phone
  events do not automatically wake the eyes page in dashboard mode.
- In dashboard mode, calls appear only on Alerts, whether opened by a swipe or
  a new notification. Calls themselves do not trigger automatic page selection;
  they never cover eyes, routes, weather, clock, or settings automatically.
  Switching dashboard mode off restores full-screen call overlays on all pages.
  No on-device answering/rejection is implemented.

Automatic page changes run in the main task, never inside BLE callbacks. They
wake the backlight at the selected day/night level. No timed return to Eyes is
implemented; after an event you can swipe to another page.

See [Settings](SETTINGS.md) for all touch controls, USB commands, and persistence.

## Screen data

- **Clock:** Synced time, seconds, date, connection state, and a clear waiting
  state before the first completed phone sync. Respects Chronos 12/24-hour mode.
  The local clock continues while disconnected; it needs resynchronization after
  a reboot. Only changed text fields are cleared and repainted.
- **Alerts:** Latest app, timestamp, title, and wrapped message. Message pages
  advance every six seconds and loop. Up to 1,024 message bytes are retained.
  The latest alert remains visible after disconnection.
- **Weather:** City, current temperature, vector condition icon, update time,
  and up to three daily high/low forecasts. Missing forecasts are separate from
  missing weather. Weather becomes stale three hours after receipt. The update
  time is a receipt time, not the weather provider's observation timestamp.
- **Route:** Complete received turn icon, next instruction, remaining distance,
  speed when supplied, and ETA/duration. Long fields are clipped with ellipsis.
  Incomplete or mismatched icon packets show a neutral placeholder, never a
  guessed turn. Ending navigation or disconnecting shows an inactive state.
  The default large layout emphasizes a 96x96 icon, distance, and two larger
  instruction lines. DETAILS retains speed and ETA. The VIEW button is temporary;
  the LOOK tab's Route default is separately saved.
- **Calls:** Incoming caller label and amber emphasis on the Alerts page in
  dashboard mode. Dismissal or disconnection removes the call view.
- **Settings:** Five parked-use tabs for appearance, lighting, dashboard, motion/language,
  and optional audio
  behavior. Style, color, and mood changes are previewed by returning to Eyes.

UI labels support English/Italian; USB command vocabulary and received phone
content are unchanged. Audio is opt-in, night-muted by default, and scheduled
without blocking the UI. See [Audio, motion, and language](SOUND.md).

Eye styles use different silhouettes: MOCHI is a tall solid capsule; BLOUB is
a wide, soft rounded shape with pupils; NORMAL is an oval with pupils; TRAPEZ
is a pair of mirrored, in-bounds trapezoids with no pupils. MOCHI/TRAPEZ also
omit pupil highlights. Their love/dizzy reactions change the whole eye silhouette.
Random and IMU mood options are described in [Settings](SETTINGS.md); they never
change the saved base mood or style.

Rendering uses the built-in bitmap font for predictable spacing and bounded
layout. Common lowercase accented vowels are converted to ASCII; unsupported
Unicode glyphs appear as `?`. BLE callbacks store bounded, mutex-protected copies
and never draw, change the selected screen, or log full notification/caller/route
payloads. Utilities refresh on events and time-dependent updates rather than
flushing the same canvas 30 times per second.

## Hardware limits

Touch probes `0x15` (CST816 family) and `0x38` (FT6X36 family). Read the boot
diagnostic to identify which address answered on the actual board; a probe alone
does not verify the exact chip model.

The reference board is the Waveshare ESP32-S3-Touch-LCD-1.69 **OLD**. The user
confirmed its pin assignments in `pin_config.h`; this change leaves them intact.

Chronos is explicitly given the rectangular `CS_240x283_172_RTF` profile before
BLE starts. ChronosESP32 1.9.1 has no exact 240x280/1.69-inch profile; this is its
nearest rectangular preset. It is app metadata only: the physical canvas stays
240x280 and the LCD configuration does not change. After uploading this revision,
reconnect in Chronos to refresh device information. If it retains the old round
icon, remove/re-add the device within Chronos. The app may report the approximate
240x283/1.72-inch metadata rather than the exact panel dimensions.

The IMU driver checks QMI8658 identity `0x05` at `0x6B` or `0x6A`, verifies
configuration writes, enables only the accelerometer, and explicitly configures
the range to ±16g at 125 Hz, matching the 2,048 LSB/g conversion. Hardware wiring,
orientation, and real motion response still require the device checklist.
The first valid sample is a mounting reference: power it on while parked.
Smoothed relative acceleration moves the gaze by at most eight pixels with a
small noise dead zone. Small idle motion is slower in dashboard mode.

Shake-to-dizzy is opt-in and requires three opposing strong impulses within
800ms between impulses, followed by a five-second cooldown. A single bump or
steady acceleration does not trigger it. This heuristic is not a pothole or
driving-maneuver classifier; repeated rough-road motion can still resemble a
shake, so leave it disabled in the car. The reaction lasts two seconds and
restores the selected mood. Disabling shake cancels a pending reaction.

No AXP2101 presence or charger status is assumed. Battery percentage is a
best-effort, plausibility-gated estimate from `BAT_ADC`; without plausible
hardware the Device status page shows NOT MEASURED. Legacy power-pin candidates
in `pin_config.h` remain unused.
