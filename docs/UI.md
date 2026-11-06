# Companion UI

The living eyes remain the default screen. Utility screens use a black background,
rounded tinted cards, eye-colored text and icons, and dimmer secondary labels.
The eye color is the shared primary color, including controls, calls, and weather.
Seven position dots indicate the selected
screen; the header shows a speaker icon (crossed out while sound is off or
night-muted), Bluetooth state, and the battery percentage (`--%` until a
plausible reading exists). While the PMU reports a stable charging state, the
percentage uses the accent color and a battery-with-bolt icon appears beside it.
Stable charge start/stop and percentage changes invalidate the header immediately,
without waiting for a phone packet or a page change. The
configured backlight percentage is shown on the DEVICE STATUS page, not in the
header.

The AMOLED V2 display estimate is normalized using the user's observed nominal
3% shutdown and 92% full-charge readings as 0% and 100%. Values outside that
range are clamped, and intermediate values are rounded to the nearest percent.
The LCD curve is unchanged. This adjusts the display only: it does not configure
the PMU, change charge limits or alter the hardware shutdown voltage. Recheck
both endpoints on the physical device; voltage-based state of charge remains approximate.

Android Chronos mode publishes that same displayed percentage and the stable
PMU charging flag through `ChronosESP32::setBattery` on the main task. Unknown
readings are not sent as 255%; Chronos keeps its last valid cached value and
handles change notifications and reconnect publication. Before the first valid
sample the library's initial value may still be shown. ANCS mode is unaffected.

## Controls

Android mode adds **Phone tools** as the sixth contextual desk tile. Find,
camera, alarms, phone battery, reminders, app schedules, hourly forecasts,
contacts, QR links and explicit app-language association are described in
[Chronos integration](CHRONOS.md). The tile and Chronos-only actions are hidden
in iPhone mode; the seven-page carousel remains unchanged.

The Moon face animates new, waxing crescent, first quarter, waxing gibbous,
full, waning gibbous, last quarter and waning crescent moons over its six-second
decorative cycle. Both eyes share the phase, with golden fill, configured ink
along the edge and the existing star field. Moon stays open without blinking or
returning to ordinary eyes; this animation does not track the real lunar calendar.
Fire also stays open, with no ordinary-eye handoff or configured-color outline.
Its orange and yellow flame tongues flicker independently, with smoothly
interpolated pseudo-random height and sway.

`mecha` closes a complete angular helmet over the configured original eyes,
holds its copper optics, split pale-alloy plates and segmented chin, then retracts the visor,
side guards and jaw in staggered stages to reveal those same eyes again.
`mecha_mask` (ID 135) moves only the lower mask beneath the original live eyes:
no helmet, visor or cover is drawn over them. `mecha_break` (ID 136) releases the visor first, then the
guards and jaw, with diagonally departing pieces. All three use a six-second
mechanical cycle, not a permanently drawn robot face.
The `tic_tac_toe` face uses a cycle-seeded shuffle for legal alternating moves,
stops at a win or draw and changes its opening and outcome between cycles.
The playable desk Tris opponent is unchanged.
IDs 137–143 add `hollow_anger`, `snickering`, `hiccup`, `retro_tv`,
`mecha_pilot`, `comet` and `yakura`. These six-second performances preserve the selected
style and palette at both endpoints. Comet paths, Sakura petals and hiccup
bubbles have deterministic event-based variation, not per-frame randomness.

All faces now receive session-only variation in Dasai and Playful modes:
smooth micro-gaze, gentle stretch, and slight opening differences when asymmetry
is enabled. Idle gestures last 700–1200 ms instead of a fixed beat. Special scenes
retain their stable origin, but their active timing varies smoothly between cycles;
the first/last 800 ms and complete loop durations stay unchanged so transitions
and palette restoration remain predictable. Existing rain, petals, smoke, bubbles,
comets, glitch bursts and stars use the visible face's entry seed. The star field
also chooses a stable new arrangement on entry, not new positions every frame.
No generic confetti, extra pupils or palette changes are added.
Calm disables these additions; dashboard and night settings reduce micro-motion
and rhythm variation. Seeds never enter the saved profile.

Symbol interiors have explicit accents independently of the selected eye-body ink:
Diamond/Ring/Question use cyan, Idea gold, Power green and Swirly violet.
Diamond retains its silhouette around the colored inset; Swirly keeps its
outlined spiral and Ring stays hollow. Solid-style pupil exclusions are unchanged.
Devil (ID 44) now has two external red horns above sly eyes, with no internal
horn glyph. Halo (ID 43) has only one hollow golden aureole above the original
eyes, without wings. Angel (`angel`, ID 145) is a separate face with that aureole,
gentle lower lids and two pale-blue feathered wings outside the eye pair.
Their horns, aureoles and wings ease in over 1.2 seconds from the visible face's
entry clock, rather than appearing instantly. The eye-body ink itself does not
fade. All retain the configured eye-body ink and style, including during blinks
and live gaze. They add no mouth/body, new settings record or renumbered IDs.
Angel is selectable through the existing screen, USB and web catalogs; the
older face allow-list keeps its exclusions and enables the appended ID.

Continuous playback advances to another face after one complete visible loop,
without replaying the same animation two or three times. Return, group-of-five
and sequence modes retain randomized whole-loop dwell, not arbitrary
mid-animation deadlines. The first idle wait is randomized too. Handoffs wait
for the displayed loop to finish, including the entry blink and any idle gesture.
Idle gestures have random initial and repeat delays rather than a fixed first beat.
The nine spontaneous gestures include a double blink, wink, sideways peek,
soft squint, stretch, single nod, double nod, searching glance and curious lean.
Ordinary face handoffs use one blink: sad/tired eyes reopen more slowly within
the same 320-ms handoff, surprised eyes open decisively and happy eyes gently
stretch as they reopen. An overlapping automatic blink is consumed, not replayed.
Scenes and symbols retain their dedicated animation instead of these gestures.
In Dasai/Playful modes, first contact adds a 420-ms surprise stretch while gaze
continues following the finger. With IMU faces enabled, a tilt excursion adds a
700-ms curious lean; it rearms only after returning near center. These are visual
poses, not new selected moods or saved settings. Petting, pickup feedback,
face handoffs and Dizzy take priority; suppressed gestures are not queued.
Calm disables the new gestures, while dashboard/night reduce their intensity.
Tilt sensitivity and appearance still require physical-device validation.

- Settings → SYSTEM → **Firmware update** opens a temporary Wi-Fi maintenance
  screen, outside the seven-page carousel. Its password and browser address are
  shown on the display; normal controls pause until exit, timeout or restart.
  See [OTA](OTA.md) for first-install requirements and safety limits.

- Swipe left/right through Eyes, Clock, Music, Route, Alerts, Weather, and Settings.
- Vertical swipes follow the contextual layers: swipe **down** on Clock to open
  Scrivania (desk tools), then **down** again for **Games / Giochi** (D&D dice,
  Pong and Tris). Swipe **up** from a game to Games, from Games or a desk tool
  to Scrivania, and from Scrivania to Clock. Swipe **down** on Route
  toggles the large/details layout and swipe **down** on Weather requests a
  phone sync and a configured Internet weather refresh. Downward swipes on Games
  and Dice stay there; other utility pages retain the quick return to Eyes.
  Vertical strokes still pet Mochi on Eyes and still scroll rows inside
  Settings.
- Tap the top status strip, above the page title, on Clock, Music, Route, Alerts,
  Weather, or any desk tool to mute or unmute every sound for the session;
  unmuting chirps once. The strip is inactive on Eyes (where taps are face
  reactions) and inside Settings. This edits the RAM profile like the SOUND
  section does, so a Settings SAVE persists it too.
- A downward swipe on Clock opens Scrivania: large clock, overview, local
  timer, device status and analog clock, without adding them to the primary swipe
  carousel. See [Desk tools](DESK.md).
- A short tap on Eyes requests a temporary random face and makes the eyes
  glance toward the touch point for a couple of seconds. It does not change
  saved preferences or require automatic random moods to be enabled.
- While touching Eyes, the gaze smoothly follows the current finger position.
  Live contact overrides random gaze, micro-gaze and IMU tilt, and defers
  automatic face changes without requesting a face or sound each frame.
  Full-screen scenes retain their stable origin to avoid clipping. Lifting the
  finger resumes idle gaze after a short pause; released taps, vertical petting,
  horizontal swipes and the stationary 1.2-second Settings hold keep their roles.
- Recognized petting strokes also ease the eyelids into a relaxed pose. Repeated
  strokes sustain it; after the last stroke, the gaze and lids return smoothly.
  This visual feedback works in Calm too and never adds pupils or changes ink.
- With IMU faces enabled, a quiet-to-motion-to-settled change of pose can request
  a brief pickup wake gesture. It opens the eyes gently without selecting a new
  expression, changing saved preferences or adding sound. Live contact,
  affection and Dizzy take priority; full-screen scenes retain their origin.
  This is a conservative motion heuristic, not proof that the robot was lifted.
- Double-tap quickly on Eyes for a playful face from a separate excited/star
  pool, with the same glance toward your finger. Dizzy keeps priority.
- Stroke vertically in either direction on Eyes to pet Mochi: a single
  32-pixel pass, with moderate sideways drift, requests an affectionate face from the
  happy/love family, and a small pink heart appears near your finger for
  under a second as confirmation. Continued strokes keep the same face and
  keep it active until three seconds after the last recognized stroke, without
  replaying its sound each time. Slow strokes count too; no return pass is needed.
  Repeated up/down rubbing also works with shorter passes once the finger
  leaves the tap box. Sideways drift below 64 pixels is tolerated, while small finger jitter and
  movement within the 18-pixel tap box do not pet. Petting never changes saved preferences,
  leaves Eyes, or opens Settings. Use vertical strokes; horizontal motion
  pages away.
- Eyes has no header or audio-status icon, even when muted. Other pages keep
  the speaker status in their header, with sound waves when enabled and a
  cross when audio cannot sound.
- Settings → SENSORS → **Screen flip** mounts the whole interface upside down
  for bottom-up installations. Touch targets and the IMU gaze follow the
  180-degree rotation, and the choice is saved with the profile.
- Settings → SENSORS → **IMU zero** calibrates the sensor while parked and still.
  **Auto rotate** follows calibrated 180-degree turnovers, on every page.
  Successful calibration is saved automatically and restored at boot; repeat
  IMU zero to replace it. Only the Auto rotate switch is session-only.
- Startup shows a silent 2.2-second OTA BOT / MOCHI title with opening/blinking
  eyes and a progress line. BLE, USB and touch polling continue; touch actions
  and automatic page changes wait until the intro ends. Mochi then greets with a
  brief three-second temporary face played
  without a face-reaction chirp; the selected mood returns automatically. A
  separate startup cue is requested once when the intro finishes, respecting
  master Sound and Night mute. See [Sound](SOUND.md).
- On AMOLED V2, holding PWR starts the title after one second, then closes the
  eyes and fades over the remaining three seconds to the autonomous power-off.
  The farewell no longer finishes well before the four-second hold. Keep holding to shut
  down; releasing early restores the previous page.
- On AMOLED V2, click and release BOOT to advance through Eyes, Clock, Music, Route,
  Alerts, Weather and Settings, then wrap to Eyes. Desk tools advance from
  their Clock parent to Music. Holding does not repeat. Clicks during startup,
  shutdown, a visible call overlay or a held touch are discarded. Holding BOOT
  during reset still selects the chip's firmware download mode.
- New notifications automatically open Alerts, including while a route is active.
  After 60 seconds from the latest notification, the previous page returns.
  Leaving Alerts manually cancels this return; opening Alerts manually does not
  start a timeout. The return also waits for a held finger to be released.
  Starting actual phone navigation automatically opens Route. Regular route/icon
  updates do not pull you back after a manual swipe. If both events arrive in the
  same loop, starting navigation takes priority. Switching waits for a held finger
  to be released, avoiding a page change underneath a settings tap.
- Hold still for 1.2 seconds on the eyes screen to open Settings. A swipe or
  moving finger is not also treated as a tap or hold.
- The desk hub presents six 108x44-pixel cards in Android mode, five in iPhone mode,
  in a two-column, three-row grid (design coordinates). Games lives one layer below;
  Dice supports 1–100 D4/D6/D8/D10/D12/D20/D100 with individual results and a total.
  The
  local timer uses a full-width 50-pixel START/PAUSE accent button with
  48-pixel RESET and MODE buttons, so every desk control is a comfortable
  parked-use target.
- Use settings only while parked. The firmware cannot determine whether the
  vehicle is stationary and does not lock controls based on vehicle speed.
- Use the section arrows for LOOK, DISPLAY, FACES, SENSORS, SOUND and SYSTEM.
  Tap SAVE to persist them, then EYES to return. Changes apply immediately in RAM.
- FACES → Face library lets you enable/exclude faces for every selection and
  reaction. Keep at least one enabled. Its Wi-Fi button opens an offline browser
  editor for five minutes; see [Face library](FACE_LIBRARY.md). Excluded entries
  in LOOK → Mood are crossed out and cannot be selected.
- SYSTEM → Wi-Fi settings opens a temporary browser session. With saved Wi-Fi,
  its QR opens the router-assigned local address and authorizes access; otherwise
  join the displayed configuration network. The browser can save a home network,
  NTP hostname/time zone and a city for automatic Internet weather.
  Network configuration never opens automatically, including when no network is
  configured or association fails. BACK returns to normal operation; saved Wi-Fi stays on.
  See [Web settings](WEB_SETTINGS.md) and [Network](NETWORK.md).
- Settings show three larger rows at a time. Swipe vertically to reach remaining
  rows; the scroll indicator shows the position. Tabs and SAVE/EYES stay fixed.
  Vertical scrolling neither changes a value nor advances the page carousel.
  Every row card owns its own 44-pixel touch zone aligned to the visible card,
  so touching the bottom of a row never activates the next one. The Mood grid
  shows five rows of 72x24-pixel cells as contiguous full-height targets; only
  the thin column gutters stay inert. Settings categories use previous/next
  arrows with 72x38-pixel targets below the title, a central category name and
  a 1/6 position indicator. They wrap around and reset panel scrolling; tapping
  the name does nothing. SAVE/Back each owns a 120-pixel-wide target that
  starts at the 224 design row and reaches the physical panel bottom (keeping a
  four-pixel inert margin), so it stays tall enough to hit on panels whose design
  field scales short of the edge. Their visible buttons are enlarged to match,
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
wake the backlight at the selected day/night level. An automatically opened Alerts page returns to Eyes after 60 seconds without a
new notification or touch; touch postpones the return. A manual page selection
cancels that automatic return. Other pages are not automatically timed out.

See [Settings](SETTINGS.md) for all touch controls, USB commands, and persistence.

## Screen data

- **Clock:** Synced time, seconds, date, connection state, and a clear waiting
  state before the first completed Chronos or NTP sync. Respects Chronos 12/24-hour mode.
  The local clock continues while disconnected; it needs resynchronization after
  a reboot. Only changed text fields are cleared and repainted.
- **Alerts:** A local history of the last eight alerts, newest first, with app,
  timestamp, title, and wrapped message. The bottom **< / CLEAR / >** buttons
  select a newer alert, remove the displayed local copy, or select an older alert.
  The alert counter is separate from message pagination. Removing an alert selects
  the next older entry, or the nearest newer entry at the end. Removing the last
  alert shows the empty state. New arrivals select the newest alert; overflow
  drops the oldest. No read/delete command is sent to the phone.
  Message pages
  advance every six seconds and loop. Up to 1,024 message bytes are retained.
  Browsing restarts the page timer. History survives disconnection but not reboot.
- **Weather:** City, current temperature, vector condition icon, update time,
  and up to three daily high/low forecasts. Missing forecasts are separate from
  missing weather. Weather becomes stale three hours after receipt. The update
  time is the phone receipt time for Chronos, or the provider's local observation
  time for Internet weather. `Open-Meteo` identifies Internet results; stale
  Internet results prompt a Wi-Fi check instead of opening Chronos. Failed
  refreshes retain cached data. Phone weather remains a fallback until the
  first Internet result, or after Internet weather is disabled.
- **Route:** Complete received turn icon, next instruction, remaining distance,
  speed when supplied, and ETA/duration. Long fields are clipped with ellipsis.
  Incomplete or mismatched icon packets show a neutral placeholder, never a
  guessed turn. Ending navigation or disconnecting shows an inactive state.
  The default large layout emphasizes a 96x96 icon, distance, and two larger
  instruction lines. DETAILS retains speed and ETA. The VIEW button is temporary;
  the SYSTEM section's Route default is separately saved.
- **Calls:** Incoming caller label and amber emphasis on the Alerts page in
  dashboard mode. Dismissal or disconnection removes the call view.
- **Settings:** Six parked-use sections for appearance, display, faces, sensors,
  sound and system
  behavior. Style, color, and mood changes are previewed by returning to Eyes.

UI labels support English/Italian; USB command vocabulary and received phone
content are unchanged. Audio is opt-in, night-muted by default, and scheduled
without blocking the UI. See [Audio, motion, and language](SOUND.md).

Eye styles use different silhouettes: MOCHI is a tall solid capsule; BLOUB is
a wide, soft rounded shape with pupils; NORMAL is an oval with pupils; TRAPEZ
is a pair of mirrored, in-bounds trapezoids with no pupils. MOCHI/TRAPEZ also
omit pupil highlights. Their love/dizzy reactions change the whole eye silhouette.
Anime has tall ovals with large reflective irises; Kitsune has Normal-size ovals
and slit pupils; Headlight DRL has filled angular eyes, a selected-color center
and a fixed white rim, without pupils.
All seven are permanent styles, not temporary expressions.
Resting faces keep the selected eye color, including hearts, stars, engraved
symbols. The petting marker remains pink. Emotions differ through silhouette:
curved smiling lids, deeper angry slopes, lowered sad eyes, thin resting eyes,
and round or narrow startled eyes. Related emotions vary in width, opening,
and vertical position. Twenty-four moods also have
[GIF-informed procedural reactions](ANIMATIONS.md#gif-informed-ordinary-reactions):
chevron laughter, flowing tears, sleepy arcs, asymmetric smirks, startled
stretching, and a stretch/squeeze/recovery sneeze. Hearts turn pink, tears blue,
and angry eyes orange/red during their reactions, then return to the selected
color without changing preferences. Explicit effects such as Glow and Police
retain their own temporary scene colors.
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
The optional `-DMOCHI_BOARD_AMOLED_18` profile targets the Waveshare
ESP32-S3-Touch-AMOLED-1.8 V2; `docs/BOARDS.md` lists its differences and the
checks that still need the physical panel.

Widgets are laid out in a 240x280 design space (`UI_DESIGN_W`/`UI_DESIGN_H` in
`ui_theme.h`) and scaled to the actual panel with `ui_scale()`/`ui_s()`. The
240x280 reference panel keeps the original pixel geometry 1:1; a larger panel
grows the whole layout — header, cards, rows, tabs, footer, and the matching
tap bands — so it fills the screen instead of framing a small island in a
corner. Labels and values step one text size up via `ui_text_size()` on
scaled-up panels so they do not drown inside the bigger widgets, while display
sizes (clock, timer, distance) stay as tuned; doubled text outgrows the design
boxes, so row label/value columns and the timer button row trade width in its
favor only when `ui_scale() > 100`. The strip below the scaled design field
stays dead, and the per-panel native suite proves the layout for both boards.

Chronos is explicitly given the rectangular `CS_240x283_172_RTF` profile before
BLE starts. ChronosESP32 1.9.1 has no exact 240x280/1.69-inch profile; this is its
nearest rectangular preset. It is app metadata only: the physical canvas stays
240x280 and the LCD configuration does not change. After uploading this revision,
reconnect in Chronos to refresh device information. If it retains the old round
icon, remove/re-add the device within Chronos. The app may report the approximate
240x283/1.72-inch metadata rather than the exact panel dimensions. The AMOLED
profile gives `CS_410x494_200_RTF` for the same reason: version 1.9.1 has no
368x448 entry, so the nearest rectangular preset by aspect ratio and area is
used as metadata.

The IMU driver checks QMI8658 identity `0x05` at `0x6B` or `0x6A`, verifies
configuration writes, enables only the accelerometer, and explicitly configures
the range to ±16g at 125 Hz, matching the 2,048 LSB/g conversion. Hardware wiring,
orientation, and real motion response still require the device checklist.
The saved calibration supplies the mounting reference. Without one, the first
valid sample supplies a temporary gaze zero: power it on while parked.
Smoothed relative acceleration moves the gaze by at most eight pixels with a
small noise dead zone. Small idle motion is slower in dashboard mode.

Shake-to-dizzy is opt-in and requires three opposing strong impulses within
800ms between impulses, followed by a five-second cooldown. A single bump or
steady acceleration does not trigger it. This heuristic is not a pothole or
driving-maneuver classifier; repeated rough-road motion can still resemble a
shake, so leave it disabled in the car. The reaction lasts two seconds and
restores the selected mood. Disabling shake cancels a pending reaction.

AXP2101 PWR-key setup probes and checks the PMU, but no charging configuration
is implemented: voltage/current limits and safety registers stay untouched. The
read-only charge direction from the PMU drives the header charge bolt and the
DEVICE STATUS indicator; it never writes PMU registers. Battery percentage is a
best-effort, plausibility-gated estimate from `BAT_ADC` on LCD or the AXP2101
battery-voltage ADC on AMOLED V2; without a plausible reading the Device status
page shows NOT MEASURED. The LCD SYS_EN/SYS_OUT pins
in `pin_config.h` have board-specific PWR handling described in [Boards](BOARDS.md);
they do not establish battery calibration or measured standby behavior.

### iPhone notifications (ANCS)

Select iPhone/ANCS under Phone mode, tap SAVE, and restart Mochi manually. Pair `OTA bot` from iPhone Bluetooth settings and approve notification access when iOS asks. Incoming iPhone alerts appear in the existing Alerts history; the on-screen Dismiss control removes only Mochi's local copy. If iOS does not offer notification access, remove the pairing and pair again. The user confirmed ANCS notification delivery on AMOLED V2 on 2026-10-29.
Reconnect, permission-revocation and long-session checks remain separate; LCD
hardware coverage is not implied. ANCS availability is controlled by iOS. Chronos is used only in Android mode; iPhone mode has no phone-provided navigation or call overlay.

## Music

Music is a primary carousel page between Clock and Route. It shows the current
track title, artist and playback state, with Previous, Play/Pause and Next touch
buttons, plus phone-volume minus/plus controls. iPhone shows a percentage only
when AMS reports it; Android does not invent a volume estimate. Unsupported
commands remain disabled. Long metadata advances in bounded windows every two seconds. The page
uses the selected Mochi palette; it does not change the configured eye expression
or draw an audio spectrum. Track updates never select the page automatically.

Android uses Chronos music packets and remote commands. Enable media access in
the phone companion app as required. iPhone mode uses Apple Media Service (AMS)
on the existing bonded ANCS connection; no second BLE stack is started. Open a
media player on the iPhone if Music reports unavailable. AMS commands remain
disabled until the active player advertises them. Phone volume is separate from Mochi's own Sound level
and the speaker/mute badge in the header. Supported apps and commands
vary; successful transport delivery does not prove that a player acted on a
command. Disconnecting clears metadata and queued commands. Incoming calls
block media actions.

Native tests cover parsing, bounded text, both panels, command mapping, and
controlled AMS discovery/writes. Real phone reconnection, notification/media
coexistence, service changes, and player behavior still require bench validation.
See [Apple's AMS specification](https://developer.apple.com/library/archive/documentation/CoreBluetooth/Reference/AppleMediaService_Reference/Specification/Specification.html).

## Diagnostics

Settings > SYSTEM > Diagnostics opens a read-only, vertically scrollable page
with board identity and live device status on both panels. BACK returns
to SYSTEM without saving or changing preferences. See [Settings](SETTINGS.md#device-diagnostics)
for field meanings and the limits of SD and connectivity observations.
