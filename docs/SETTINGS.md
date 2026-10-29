# Dashboard settings

The device is a Dasai-inspired dashboard companion, not a vehicle instrument.
Eyes remain the default page. It does not detect whether the car is moving.
Change settings only while parked; neither touch nor USB is a driving control.
Use a suitable regulated USB power supply, not the vehicle's 12V supply directly.

## Touch menu

Swipe to Settings, or hold a stationary finger on the eyes screen for 1.2 seconds.
Release before tapping a row. Short taps cycle a choice; swipes do not also activate
rows. Changes are live in RAM and do not write flash while browsing.

| Tab | Row | Choices |
| --- | --- | --- |
| LOOK | Eyes | normal, mochi, bloub, trapez |
| LOOK | Color | white, mint, cyan, amber, pink, blue |
| LOOK | Mood | opens a 3-column grid of all 135 expressions/animated moods in five full-height rows; tap a cell, swipe to scroll |
| DISPLAY | Day level | 5, 10, 20, 30, 50, 70, 90% |
| DISPLAY | Night | off / on |
| DISPLAY | Night level | 1, 3, 5, 8, 12, 20% |
| DISPLAY | Idle dim | off / on |
| FACES | Animation | Calm / Normal / Playful (USB values `calm` / `dasai` / `playful`) |
| FACES | Random mood | off / on |
| FACES | Face flow | return / group of 5 / sequence / continuous |
| FACES | Face library | enable/exclude faces globally on display or in a temporary local Wi-Fi browser session; see [Face library](FACE_LIBRARY.md) |
| SENSORS | IMU mood | off / on |
| SENSORS | Shake dizzy | off / on |
| SENSORS | IMU zero | calibrate / keep still / SAVED / FAILED |
| SENSORS | Auto rotate | on / off, session-only, requires calibration |
| SENSORS | Screen flip | normal / flipped |
| SOUND | Sound | off / on |
| SOUND | Level | 1–30% by USB: LCD PWM duty / AMOLED PCM amplitude, not measured loudness |
| SOUND | Night mute | on / off |
| SOUND | Sound options | open separate face/timer/alert/ambient switches |
| SOUND | Test sound | two short bench tones; reports off/night mute/PWM failure |
| SYSTEM | Language | English / Italiano |
| SYSTEM | Dashboard | on / off |
| SYSTEM | Route default | large / details |
| SYSTEM | Firmware update | temporary Wi-Fi and browser upload of a matching release `*-app.bin`; see [OTA](OTA.md) |
| SYSTEM | Wi-Fi settings | temporary browser access to display settings, saved Wi-Fi, automatic NTP and city-based weather; see [Web settings](WEB_SETTINGS.md) and [Network](NETWORK.md) |

Use previous/next arrows to cycle the six sections. Three 44-pixel row targets are visible at a time. Swipe up/down to reach the
remaining rows; SOUND's test is the final row. Section arrows and bottom actions stay fixed.
Each row places its label above its value, giving both the full text width.
The six sections have localized titles; FACES keeps playback controls visible
together; scroll down one row for the Face library.
Scrolling does not edit or save preferences. Horizontal swipes still change the
main page. Each row card is one unambiguous 44-pixel target aligned to its
visible bounds, so the bottom of a card never activates the next row; grid rows
are contiguous full-height targets with only the thin column gutters inert.

The selected eye color also drives the entire UI palette immediately: text,
icons, active buttons, and tinted cards. SAVE persists this through the same color
preference; no separate theme setting or profile migration is needed.

**Screen flip** rotates the whole interface by 180 degrees so the device can be
mounted with its buttons at the top. The canvas renders rotated, touch
coordinates are mirrored to match, and the IMU gaze re-baselines so tilting
still reads in the viewer's frame. The change applies immediately in RAM and
persists only after SAVE; `set flip on|off` is the USB equivalent.

### IMU calibration and automatic rotation

With the device parked, first use **Screen flip** if needed so the screen is
readable. Hold Mochi still in its normal upright mounting position, then choose
**SENSORS → IMU zero**. Twenty stable samples (about one second) establish a gaze
zero and gravity reference. Movement restarts the average; after five seconds
without a stable result it reports FAILED. A missing sensor also reports FAILED.
Retry while still. Do not calibrate while driving. A successful calibration is
saved automatically, including the first one; no separate SAVE is needed.
Choose IMU zero again at any time to replace it with a new stable calibration.
SAVED confirms persistence. FAILED also covers storage/write errors: the new
reference can still work in RAM after a write error, but it was not saved.
Failed sampling or saving never removes the previous saved record.

**Auto rotate** defaults on, but has no effect until calibration succeeds.
It compares gravity with the calibrated reference, so it does not assume the
IMU chip's physical axis polarity. A stable upside-down pose for 600ms rotates
the entire UI and touch mapping by 180 degrees; returning upright restores it.
Sideways, flat, strong-acceleration, and interrupted sample sequences do not
qualify. Calibrating flat cannot establish an in-panel up/down reference.
Rotation waits until the finger is released. The IMU is polled on all pages,
and the gaze re-baselines after rotation.

The gravity reference and its display orientation live in a separate versioned,
checksummed NVS record, tagged for the board profile. They are restored at boot
for both gaze zero and automatic rotation. Missing, invalid, or unavailable
storage leaves the manual Screen flip as the fallback until calibration succeeds.
Identical calibration records skip the flash write. Ordinary sensor samples and
automatic turnovers do not write flash. General settings SAVE/defaults do not
overwrite the separate calibration.

Only the **Auto rotate switch** remains session-only and defaults on after reboot.
Changing Screen flip after calibration invalidates the active reference for that
session, allowing a manual override and fresh calibration; the saved reference
will be loaded again on reboot unless replaced by another successful calibration.
Turning Auto rotate off returns to the manual Screen flip.
The user reports general functional operation on AMOLED V2. Exact mounting,
thresholds and touch-alignment acceptance records remain to be captured per board;
no 90/270-degree layout is implemented.

The background stays black. MOCHI and TRAPEZ are solid eyes without pupils or
highlights; BLOUB has black pupils and white highlights, and NORMAL has black
pupils. Resting emotions use the selected body color. Hearts and stars follow
that color; other symbols use the configured pupil ink or are engraved into
solid eyes. Related emotions differ through curved lids, slopes, width,
opening, height, and [procedural reactions](ANIMATIONS.md#gif-informed-ordinary-reactions).
During reactions, hearts fade to pink, tears to blue, and angry eyes to orange/red.
Each loop restores the selected body color without changing the profile or theme.
Tap EYES to preview the actual animated eyes. This is not a separate preview page.

Automatic faces stay visible for at least six seconds, repeating complete short
animation loops before switching. Longer scenes keep their full duration.
Tap, double-tap and pet reactions keep their existing durations.

The `gauge`, `glow`, and `headlights` moods are decorative animations.
Gauge accelerates its needle into the red zone, repeatedly drops back and climbs
again like a rev limiter, then returns to idle. It does not read real speed
or RPM. Glow starts with the selected eye color, cycles body colors and halos,
then returns to that color without editing the Color preference.
Headlights paints slow-pulsing side beams. All
three scale to both LCD and AMOLED and remain separate from the real clock
and playable Pong tools.

Twenty-six [original colored performances](ANIMATIONS.md), from `dancing` through
`enraged`, add reversible six-second sequences. `mistake` displays `OTA BOT!`;
`encouragement` displays `GO OTA BOT!`. They restore the selected
eye color at the end and wait for a complete cycle before automatic reactions.
Their semantic colors do not change the saved palette.

Tap SAVE to persist the whole profile. SAVED means the write succeeded; FAILED
means it did not. EYES and swiping away do not save automatically.
Phone mode selects Android/Chronos or iPhone/ANCS. After changing it, tap SAVE
and restart Mochi manually; the active BLE mode changes only at boot.

## USB serial commands

Open Serial Monitor at **115200 baud**, with **Newline** or **Both NL & CR**.
Only one serial monitor can own the port at a time. USB CDC must be enabled in
the board configuration. Commands are case-insensitive, bounded to 95 bytes,
and take effect on receipt of a line terminator.

```text
help
show
set style mochi
set color mint
set mood happy
set brightness 30
set night-brightness 5
set night on
set dashboard on
set shake off
set dim off
set random on
set face-flow group5
set imu-face on
set flip on
set route large
set phone iphone
set animation dasai
set language it
set sound on
set sound-level 10
set sound-night-mute on
save
reboot
```

`set brightness` accepts 5–90%; `set night-brightness` accepts 1–20%.
Style/color/mood choices are the names listed in the table. All switches
accept only `on` or `off`. Unknown commands, extra arguments, out-of-range numbers,
and oversized lines are rejected without changing the active settings.
`reboot` restarts the device after confirming; unsaved RAM changes are lost.

`show` prints the selected preferences, whether changes are unsaved, the sound
backend status, and the best-effort battery estimate (`battery=unavailable`
without plausible hardware), never notification or caller data. `defaults`
restores defaults in RAM; use `save`
only if you also want to replace the saved profile.
USB commands and replies remain English in both display languages.
`weather` prints non-sensitive phone packet counters; `weather sync` queues an app
synchronization and requests a rate-limited Internet refresh when configured.
See [Network](NETWORK.md) and [Audio and motion](SOUND.md).

## Temporary route layout versus saved default

On Route, tap the bottom **VIEW** button to switch LARGE/DETAILS immediately.
This is a session-only override, not an unsaved preference: SAVE for other fields
does not persist it. It survives route updates and page changes but resets on reboot.
Hold a stationary finger on Route for 1.2 seconds to use the configured default again.

In Settings → SYSTEM, **Route default** selects the preferred layout; SAVE writes
it to NVS. USB equivalents:

```text
route details
route large
route default
set route details
save
```

`route ...` is temporary; `set route ...` edits the default in RAM and `save`
persists it. An existing override stays active until explicitly reset or rebooted.
`show` distinguishes `route-default` from `route-current`. Eyes still opens at
boot; there is no new saved startup page or automatic vehicle/desk detection.

## Defaults and persistence

Default firmware preferences:

- MOCHI eyes, white on black, neutral mood.
- Dashboard mode on: calls alone do not take over the current page.
- New notifications open Alerts; starting phone navigation opens Route, in both
  dashboard and non-dashboard modes. Regular route updates do not repeat the switch.
- Day brightness 30%; manual night mode off, night brightness 5%.
- Idle dimming off, keeping the eyes visible when used as a powered companion.
- Shake reaction off; ordinary IMU gaze remains active.
- Random and IMU expression changes off until explicitly enabled.
- Large navigation layout by default.
- Screen flip off: touch and IMU use the panel's native orientation.
- Dasai-inspired motion preset, English UI, sound off, PWM level 10%, night mute on.

## Automatic expressions

Enable **Random mood** in FACES and/or **IMU mood** in SENSORS, then SAVE. They change
expressions only, not the selected eye style or saved color. Render-only mood
colors follow the visible expression, including temporary reactions.

Automatic reactions wait for the selected mood to complete a visible cycle:
12 seconds for Gauge/Pong, 9.6 seconds for Glow, 3.6 seconds for Headlights,
six seconds for performances, and three seconds for other non-neutral expressions. Timing starts when the
renderer actually shows the mood, including after returning from a reaction.
Neutral allows automatic reactions immediately. Touch reactions remain immediate;
decorative tilt/gaze keeps running while automatic mood changes wait.

- **Random:** The first change is after 12 seconds. **Face flow** selects playback:
  **Return** keeps the original one-face-then-base behavior (12–30-second
  start intervals); **Group of 5** chains five random faces, then rests on the
  selected base for 12–30 seconds; **Sequence** continuously visits faces in
  enum/menu order; **Continuous** chains random faces without base pauses.
  Continuous excludes the last eight selected IDs and the two most recent
  visual families (for example, happy variants or sleepy variants), as well as
  the current/base family. History resets after an explicit mood change or reboot.
  Sequence, Group of 5 and Return keep their existing selection rules.
  In Continuous, an expired IMU reaction hands control directly back to random
  playback. It does not restart the 12–30-second idle wait, which repeated
  sensor reactions could otherwise postpone indefinitely. Shake still uses
  its bounded two-second override; neither sensor option is disabled.
  All four require Random mood on. USB: `set face-flow return|group5|sequence|continuous`.
  SAVE persists the mode; older profiles default to Return.
  Each face keeps its complete cycle: ordinary faces three seconds, performances
  six seconds, Gauge/Pong twelve, Glow 9.6, and Headlights 3.6.
  Automatic reactions draw from the whole expression set (every
  expression except neutral and the special dizzy state), avoiding the base
  mood and consecutive duplicates. Touch reactions keep a curated playful pool
  of 27 choices (happy, glee, focused, skeptic, love, awe, sleepy eyes,
  excited, grin, curious, wondering, star, smug, confused, shy, sparkle,
  beaming, mild happy, smirk, daydream, cheeky, zen, music, idea, squish,
  derp, hyper).
- **IMU lateral axis:** A mount-relative X deflection of at least 0.25g in one
  direction for 350ms produces a brief skeptic expression.
- **IMU longitudinal axis:** A mount-relative Y deflection of at least 0.25g
  in one direction for 350ms produces focused.
- **IMU vertical axis:** A mount-relative Z deflection of at least 0.8g for
  100ms produces surprised. One isolated sample is not enough.
- IMU reactions last 2.2 seconds with a five-second cooldown. Long automatic
  scenes finish before an ordinary IMU mood can replace them. IMU can interrupt
  ordinary random faces. Disabling Random mood or choosing a manual mood restores
  the base. Shake-to-dizzy remains a separate opt-in and has the highest priority
  once the selected mood's visible cycle is complete.

The sensor axes depend on mounting orientation: these are decorative motion
responses, **not** detection of turns, braking, potholes, or driver fatigue.
Start the device while parked. Thresholds need testing on the actual mounting;
keep IMU/shake reactions off if rough-road motion is distracting.

The same sensor also drives a bounded decorative gaze, separate from the mood
reactions above. It is clamped to ±14 pixels, damped, and only recenters below
about 0.1g of sustained load, so a gentle curve keeps the eyes displaced for the
whole bend instead of averaging back to centre. The eye pair also rolls by up to
4 pixels while leaning. Both effects are decorative; the user confirmed IMU
operation, not measured motion accuracy or every mounting; see `TODO.md`.

For solid MOCHI/TRAPEZ, love, dizzy and star draw whole-eye hearts, crosses and
stars, while crying, the ten symbol faces (halo, devil, fire, moon, sparkle,
bolt, diamond, ring, target, money) and the glyph faces (music, low battery,
power, loading, idea, glitch, question, calculating) engrave their mark into the
body. None of them inserts a pupil or highlight inside another eye body.

A short tap on Eyes requests a three-second random expression even when Random
mood is off, and holds the gaze toward the touch point for about 2.5 seconds.
A quick double tap instead requests a 3.5-second playful face (awe, excited,
star, sparkle, glee, beaming, manic, hyper, squish, derp) with the same glance. Repeated taps select
a different expression without changing the saved base, style, or color. The tap
reactions take priority over ordinary IMU moods; an active dizzy reaction keeps
priority and ignores them. Petting still requests affectionate faces, and
further strokes extend the same face up to six seconds from the first stroke.
Each reaction chirps with a fresh random pitch drawn from its face family's
tone band (see [Audio and motion](SOUND.md)).
A hold still opens Settings and a swipe does not also trigger a random face.
These reactions remain available in dashboard mode, which only calms idle motion.

The full profile is stored as a validated thirteen-byte version-seven NVS blob
with a checksum; version seven adds Phone mode while preserving the prior layout.
Thirteen-byte version-six profiles retain Face flow and default to Android mode;
version-five profiles retain Screen flip and default to Return. Twelve-byte
version-four and eight-byte versions one/two/three profiles retain their
existing choices, with Screen flip defaulting off, sound options disabled by
the master switch, English UI, and Dasai motion. Versions one/two default the new
route layout to large; version-one automatic
face options remain off. The next explicit SAVE upgrades a legacy profile.
Unknown/corrupt blobs fall back to defaults. If storage is
unavailable, settings still work in RAM but SAVE reports failure. Already saved,
unchanged profiles are not written again. Flash erasure or changing the NVS
partition can remove preferences.

Night mode selects brightness, reduces animation intensity, and mutes audio when
Night mute is enabled. It does not apply a blue-light filter or detect ambient
lighting. Optional dimming reduces idle brightness to at most 3%,
and never raises a 1% or 2% night setting. PWM failure is diagnosed over serial;
the existing GPIO fallback uses full brightness, so check this on the board before
using night mode in a car.

Firmware compilation/upload and physical display, touch, IMU, and NVS validation
remain with the user. Native tests cover the logic with hardware substitutes.
