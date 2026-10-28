# Audio, expressive motion, and language

## Optional buzzer

The `BUZZER` pin is GPIO33, the `Buzz` net of the OLD board revision, matching the
official Waveshare schematic and demo (`beePin = 33`; the new revision uses GPIO42).
Sound is **off by default** and never allocates PWM/GPIO merely because firmware
booted.

In Settings → SOUND, enable Sound, choose a level, and optionally open Sound
options for independent face, timer, notification/call, and ambient switches.
BACK returns from these options. SAVE explicitly persists the profile.
Outside the menu, tapping the top status strip on Clock, Route, Alerts, Weather,
or desk tools toggles the same master switch for the session and chirps once
when unmuting; the strip is inactive on Eyes and in Settings, and SAVE persists
that RAM value with the rest of the profile. The header speaker icon is crossed
out whenever sound is off, night-muted, or unusable after a PWM failure; on
Eyes has no audio-status glyph, including while muted.

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

## AMOLED V2 speaker (ES8311)

The Waveshare ESP32-S3-Touch-AMOLED-1.8 V2 uses an ES8311 codec at `0x18`,
not a PWM buzzer. The same cues, switches, priority, cooldowns, night mute, and
USB test commands drive its speaker when built with `MOCHI_BOARD_AMOLED_18`.
Sound remains off by default; neither codec nor I2S is initialized until the
first permitted cue. Merely enabling sound leaves the backend `armed`.

The codec uses the existing shared I2C bus on SDA15/SCL14. Playback is
16 kHz, signed 16-bit Philips I2S with identical left/right samples,
MCLK=256× sample rate on GPIO16, BCLK=9, WS=45, and data out=8.
GPIO46 enables the speaker amplifier during a cue and its bounded DMA drain.
No RX channel is allocated and microphone data on GPIO10 is not captured.
Initialization follows the
[official V2 audio example](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/tree/78e13f852929c2ab4f9d5e0ad1c50ea378dbf2b4/examples/arduino-v2/examples/15_ES8311).

A dedicated task owns note timing, sample-level 5 ms fades, PCM generation,
and DMA writes with a 20 ms timeout; only the main task accesses I2C and
settings. Each cue is copied once with its current level, at most five notes
and one second total. It finishes on the sample clock even when rendering
stalls, without the old 100 ms refresh expiry. The low-level single-tone
diagnostic adapter still expires after 100 ms. The amplifier stays enabled
through internal pauses and five silent 5 ms blocks after the cue, allowing
the four DMA descriptors to drain. Master/night/category cancellation immediately
lowers the amplifier enable; DMA errors do so in the worker itself.
Once first used, the worker
and I2S channel remain allocated and stream silence while muted, avoiding
repeated allocation and cross-task teardown.

Native regressions reproduce main-loop polls every 80/160 ms and verify both
notes of short tap/face cues and the longer manual test. These do not measure
actual frame stalls or prove physical speaker audibility on the board.

On this board the level is 1–30% PCM amplitude, not PWM duty or measured
loudness; codec digital volume is fixed at the reference example's 85%.
USB reports `i2s-ready` or `i2s-error`, not `pwm-ready`/`pwm-error`.
The legacy `gpio=-1` field means there is no PWM buzzer, not that the codec
is missing. Allocation, codec-write, DMA-timeout, and short-write failures
mute output; disable/re-enable Sound before retrying. Readiness does not
prove physical audibility, which still requires the bench test.

On 2026-10-19 the AMOLED firmware built and uploaded with Arduino CLI 1.2.0,
ESP32 core 3.3.12, and the library pins in `TESTING.md`. The device reported
`ES8311 ready` and `i2s-ready` through two bench tests at level 10; the user
confirmed audible tones and working horizontal swipes. The native suite passed.
Volume-range calibration, physical master/night mute checks, and extended
BLE/display/audio operation remain unverified. Test settings were RAM-only.

On 2026-10-21 the sample-clocked cue fix passed the full native suite and was
built/uploaded to the AMOLED V2 on COM6 using PlatformIO 6.1.19,
Espressif32 platform 8.0.0, and Arduino core 3.3.12. The existing and generated
partition layouts matched, including NVS. The device answered USB commands and
reported `i2s-ready` after three manual test requests at level 10. The user
then disconnected/powered off the board during RAM-setting restoration; no
`save` was sent, so the saved profile remains unchanged. Physical audibility
and the affected eye reactions were not individually confirmed in that session.

On 2026-10-27 the user's general “everything works” confirmation superseded the
remaining audio complaint, consistent with v1.2.0's audio-stabilization notes.
This is user-reported functional operation, not agent hardware testing or an
enumeration of mute/category/level combinations. A recorded two-hour concurrent
BLE/display/audio session and loudness measurements remain outstanding.

## Motion presets

Settings → FACES offers **Calm**, **Normal**, and **Playful** (USB values
`calm`, `dasai`, `playful`). All use smoother gaze
and soft blinks; expression changes hand off through a roughly 320ms close/open
transition rather than a sudden shape replacement. Decorative backdrops fade
separately; eye ink is never dimmed by the backdrop transition. Expressive presets add double
blink, wink, side glance, curious lean, bounce/stretch, and nod.

Dashboard mode caps motion intensity; night reduces it further. Presets do not
change the saved base mood, style, or color. MOCHI/TRAPEZ remain solid and
pupil-free throughout transitions.

```text
set animation dasai
save
```

## English / Italian UI

Settings → SYSTEM → Language changes labels immediately in RAM; SAVE persists them.

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
The user confirmed weather operation on 2026-10-20, superseding the earlier
failure. The later general functional confirmation is consistent with that report.
Native parser tests alone would not establish phone-to-board delivery.

## No audible sound

Sound is opt-in. Enabling it does not continuously play a tone; ambient cues are
infrequent and suppressed outside idle Eyes. In SOUND, turn Sound on, check
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
