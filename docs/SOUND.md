# Audio, expressive motion, and language

## Optional buzzer

The existing `BUZZER` pin is GPIO34. The user confirmed PWM/tone use; no pin
mapping was changed. Sound is **off by default** and never allocates PWM/GPIO
merely because firmware booted.

In Settings → AUDIO, enable Sound, choose a level, and optionally open Sound
options for independent face, timer, notification/call, and ambient switches.
BACK returns from these options. SAVE explicitly persists the profile.

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

- Eye taps and automatic facial reactions can chirp.
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
