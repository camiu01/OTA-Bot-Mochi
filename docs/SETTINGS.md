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
| LOOK | Mood | opens a 3-column grid of all 118 expressions/animated moods in five full-height rows; tap a cell, swipe to scroll |
| LOOK | Route default | large / details |
| LIGHT | Day level | 5, 10, 20, 30, 50, 70, 90% |
| LIGHT | Night | off / on |
| LIGHT | Night level | 1, 3, 5, 8, 12, 20% |
| LIGHT | Idle dim | off / on |
| LIGHT | Screen flip | normal / flipped |
| DRIVE | Dashboard | on / off |
| DRIVE | Shake dizzy | off / on |
| DRIVE | Random mood | off / on |
| DRIVE | IMU mood | off / on |
| MOVE | Animation | calm / dasai / playful |
| MOVE | Language | English / Italiano |
| AUDIO | Sound | off / on |
| AUDIO | Level | PWM duty level, 1–30% by USB |
| AUDIO | Night mute | on / off |
| AUDIO | Sound options | open separate face/timer/alert/ambient switches |
| AUDIO | Test sound | two short bench tones; reports off/night mute/PWM failure |

Three 44-pixel row targets are visible at a time. Swipe up/down to reach the
remaining rows; AUDIO's test is the final row. Tabs and bottom actions stay fixed.
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

The background stays black. MOCHI and TRAPEZ are solid eyes without pupils or
highlights; BLOUB has black pupils and white highlights, and NORMAL has black
pupils. The selected color changes the eye bodies. This avoids bright full-screen
background presets. Fourteen decorated faces (crying, love, fire, star, halo,
devil, moon, sparkle, bolt, diamond, money, low battery, idea, music) additionally
paint a fixed accent mark — blue tears, a rose heart, an orange flame — that no
color setting changes, so those decorations keep reading even on a recolored eye.
Tap EYES to preview the actual animated eyes. This is not a separate preview page.

The `gauge`, `glow`, and `headlights` moods are decorative animations.
Gauge accelerates its needle into the red zone, repeatedly drops back and climbs
again like a rev limiter, then returns to idle. It does not read real speed
or RPM. Glow starts with the selected eye color, cycles body colors and halos,
then returns to that color without editing the Color preference.
Headlights paints slow-pulsing side beams. All
three scale to both LCD and AMOLED and remain separate from the real clock
and playable Pong tools.

Twenty-one [original colored performances](ANIMATIONS.md), from `dancing` through
`stargazing`, add reversible six-second sequences. They restore the selected
eye color at the end and wait for a complete cycle before automatic reactions.
Their semantic colors do not change the saved palette.

Tap SAVE to persist the whole profile. SAVED means the write succeeded; FAILED
means it did not. EYES and swiping away do not save automatically.

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
set imu-face on
set flip on
set route large
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
`weather` prints non-sensitive packet counters; `weather sync` queues an app
synchronization, not a direct weather-provider fetch. See [Audio and motion](SOUND.md).

## Temporary route layout versus saved default

On Route, tap the bottom **VIEW** button to switch LARGE/DETAILS immediately.
This is a session-only override, not an unsaved preference: SAVE for other fields
does not persist it. It survives route updates and page changes but resets on reboot.
Hold a stationary finger on Route for 1.2 seconds to use the configured default again.

In Settings → LOOK, **Route default** selects the preferred layout; SAVE writes
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

Enable **Random mood** and/or **IMU mood** in DRIVE, then SAVE. They change
expressions only, not the selected eye style or color.

Automatic reactions wait for the selected mood to complete a visible cycle:
12 seconds for Gauge/Pong, 9.6 seconds for Glow, 3.6 seconds for Headlights,
and three seconds for other non-neutral expressions. Timing starts when the
renderer actually shows the mood, including after returning from a reaction.
Neutral allows automatic reactions immediately. Touch reactions remain immediate;
decorative tilt/gaze keeps running while automatic mood changes wait.

- **Random:** The first change is after 12 seconds; subsequent intervals are
  12–30 seconds. Each reaction lasts three seconds and returns to the selected
  base mood. The pool holds 27 choices (happy, glee, focused, skeptic, love, awe,
  sleepy eyes, excited, grin, curious, wondering, star, smug, confused, shy,
  sparkle, beaming, mild happy, smirk, daydream, cheeky, zen, music, idea,
  squish, derp, hyper), avoiding the base mood and consecutive duplicates.
- **IMU lateral axis:** A mount-relative X deflection of at least 0.25g in one
  direction for 350ms produces a brief skeptic expression.
- **IMU longitudinal axis:** A mount-relative Y deflection of at least 0.25g
  in one direction for 350ms produces focused.
- **IMU vertical axis:** A mount-relative Z deflection of at least 0.8g for
  100ms produces surprised. One isolated sample is not enough.
- IMU reactions last 2.2 seconds with a five-second cooldown. They take priority
  over random changes. Disabling an option or choosing a manual mood restores
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
4 pixels while leaning. Both effects are decorative and remain unverified on real
hardware; see `TODO.md`.

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

The full profile is stored as a validated thirteen-byte version-five NVS blob
with a checksum; version five adds the Screen flip flag. Twelve-byte
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
