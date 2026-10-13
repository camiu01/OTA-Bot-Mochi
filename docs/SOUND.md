# Audio, expressive motion, and language

## Optional buzzer

The `BUZZER` pin is GPIO33, the `Buzz` net of the OLD board revision, matching the
official Waveshare schematic and demo (`beePin = 33`; the new revision uses GPIO42).
Sound is **off by default** and never allocates PWM/GPIO merely because firmware
booted.

In Settings → AUDIO, enable Sound, choose a level, and optionally open Sound
options for independent face, timer, notification/call, and ambient switches.
BACK returns from these options. SAVE explicitly persists the profile.
Outside the menu, tapping the top status strip on Clock, Route, Alerts, Weather,
or desk tools toggles the same master switch for the session and chirps once
when unmuting; the strip is inactive on Eyes and in Settings, and SAVE persists
that RAM value with the rest of the profile. The header speaker icon is crossed
out whenever sound is off, night-muted, or unusable after a PWM failure; on
Eyes, which has no header, the same glyph appears in the top-right corner only
in those silent states.

```text
set sound on
set sound-level 10
set sound-faces on
set sound-timer on
set sound-alerts on
set sound-ambient on
set sound-night-mute on
save
```

Level is a bounded 1–30% PWM duty setting, **not measured loudness**. Actual pitch
and audibility depend on the buzzer/driver. These are generated short chirps,
not recordings or synthesized speech. Verify the OLD board's buzzer on the bench
before using it in the car.

- Eye taps, double taps, pets, and automatic facial reactions can chirp with a
  **randomized two-note tone shaped by the face family**: bright/playful faces
  get an ascending mid-high warble, affectionate faces a warm mid-range chirp,
  sleepy/calm faces a slow low one, surprised/excited faces a quick high bounce,
  and negative/uneasy faces a short low blip that stays flat. Every chirp picks
  a fresh pitch inside its family band, so no two reactions sound identical.
- Pomodoro rest and timer completion can signal once.
- New notifications/calls can signal once, not continuously ring.
- Ambient chirps are quieter, rate-limited to at least a minute, and restricted
  to idle Eyes with no active route/call.
- Night mute defaults on and cancels active/queued sound in night mode.
- Categories, master mute, priority, cooldowns, and stale-queue expiry prevent
  replaying a backlog after sound is enabled.

Notes are scheduled from the main loop without blocking BLE/touch/animation.
Backlight uses channel 0 at 5kHz/8-bit, buzzer uses channel 2 initially at
2.2kHz/10-bit. Distinct timer allocation matters on newer cores, not just distinct
channel numbers. Native core-2/core-3 tests check frequency isolation. PWM failures
stop audio; there is no GPIO-HIGH fallback. Device status shows PWM state, not
proof of physical sound.

## Motion presets

Settings → MOVE offers **calm**, **dasai**, and **playful**. All use smoother gaze
and soft blinks; expression changes hand off through a roughly 320ms close/open
transition rather than a sudden shape replacement. Expressive presets add double
blink, wink, side glance, curious lean, bounce/stretch, and nod.

Dashboard mode caps motion intensity; night reduces it further. Presets do not
change the saved base mood, style, or color. MOCHI/TRAPEZ remain solid and
pupil-free throughout transitions.

```text
set animation dasai
save
```

## English / Italian UI

Settings → MOVE → Language changes labels immediately in RAM; SAVE persists them.

```text
set language it
save
```

Use `en` for English. Existing profiles default to English. Both label tables are
compiled into firmware with a safe English fallback. Built-in labels use ASCII
for the bitmap font; phone text retains the existing accent approximation.
Caller names, messages, city names, and route instructions are received content,
not translated. USB command names/replies and Chronos 12/24-hour choice are unchanged.

## Weather troubleshooting

Firmware now retains forecasts across current-packet updates and handles
forecast-first packet order. It queues one app sync after connection/subscription,
and the Weather footer or `weather sync` requests another, at most once per 30s.

`weather` reports current/forecast/city packet counts, item count, and availability,
never location or payloads. If current packets remain zero, check the paired
Chronos app, weather enablement, internet, and manual city/location permission.
The firmware cannot fetch provider data itself or grant phone permissions.
The user-reported weather failure remains a **device-validation TODO**, not a
confirmed fix on the board.

## No audible sound

Sound is opt-in. Enabling it does not continuously play a tone; ambient cues are
infrequent and suppressed outside idle Eyes. In AUDIO, turn Sound on, check
Night mute, scroll down to **Test sound**, and tap it while testing on the bench.
This explicit two-tone test ignores category switches but respects master/night
mute, duty limits, cooldown, and PWM failures. It does not save anything.

Alternatively use USB at 115200 baud:

```text
show
set sound on
set night off
set sound-level 15
sound test
sound
```

Wait one second between `sound test` and `sound`. These changes stay in RAM until
`save`. The `sound` reply shows GPIO, level, and one of:

- `off`: master switch disabled.
- `night-mute`: night mode and Night mute enabled.
- `armed`: enabled, no PWM allocated yet.
- `pwm-ready`: PWM allocated, **not a measurement of audible sound**.
- `pwm-error`: allocation/write failed. Disable sound, allow a loop to process
  mute, then re-enable before retrying.

If PWM is ready but the test is silent, the electrical buzzer path still needs
physical verification: confirm audible output on GPIO33 before using it in the
car. The buzzer pin was corrected from GPIO34 to GPIO33 on 2026-10-13 after
checking the official Waveshare schematic and demo. Do not infer an
active/passive buzzer model from the PWM-capability description or substitute
GPIO HIGH without confirming the circuit.
