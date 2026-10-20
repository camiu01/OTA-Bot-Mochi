# Contextual views

The primary carousel still contains six pages: Eyes, Clock, Route, Alerts,
Weather, and Settings. Extra tools are grouped under Clock, not added as more
swipe destinations. The device does not infer whether it is in a car or on a desk.
Use touch controls only while parked.

## Navigation

Route uses the configured large/details default when a new session begins.
Large mode shows a 96x96 received turn icon, emphasized distance, and two larger
instruction lines. Missing/incomplete icons remain placeholders, never guessed
turn arrows. Details retains speed, ETA/duration, and the smaller turn card.

Swipe down on Route to switch temporarily between large and details. Hold on
Route for 1.2 seconds to restore the configured default. Choose and SAVE
**Settings → LOOK → Route default** for future
boots. [Settings](SETTINGS.md) documents USB commands and session/save separation.
Starting real navigation in Chronos opens Route as before; repeated route updates
do not pull you back after a manual page change.

## Desk tools

Swipe down on Clock to open Scrivania, then select:

| Tool | Content |
| --- | --- |
| BIG CLOCK | Large hours/minutes, seconds, AM/PM when applicable, and date |
| DESK OVERVIEW | Time, phone connection, temperature/city, and weather freshness |
| TIMER / POMODORO | Local 5-minute or 15-minute timer, or one 25-minute focus/5-minute rest pair |
| DEVICE STATUS | BLE connection, backlight percentage, IMU initialization, clock sync, preference save state, sound state, and battery estimate |
| PONG | Play against an automatic right paddle; tap the court at the desired height to move the left paddle, RESET starts a new game |
| ANALOG CLOCK | Hour, minute and second hands with synchronized digital time below |

The hub shows six 108x58-pixel cards in a 2x3 grid. Gaps between cards are
not tap targets. Coordinates scale for both the 240x280 LCD and 368x448 AMOLED.
Pong runs at the display frame cadence, pauses off-page or behind a call overlay,
and caps physics catch-up at 100 ms. Scores wrap after 99. Game state stays in
RAM and never writes preferences. Taps move the paddle; holds and swipes keep
their normal navigation behavior.

Swipe up from a tool to return to the hub; swipe up from the hub to return to
Clock. A horizontal swipe from a tool goes to the previous/next primary page
relative to Clock, so the carousel still has only six destinations. The Clock
position dot stays selected in these tools.

Time requires a Chronos sync after boot and then continues while disconnected.
Analog Clock uses 24-hour source time for its hands, regardless of the digital
12/24-hour preference. Missing or malformed time shows a sync prompt rather
than invented hands. The main Clock and BIG CLOCK keep their existing layouts.
Weather uses received phone data and has missing/stale states; it is not measured
by the board. Device status shows initialization, not vehicle telemetry. The
battery row is a best-effort LiPo estimate from an unverified ADC divider on
`BAT_ADC`; implausible readings (no battery fitted) are reported as NOT MEASURED,
and the conversion is native-tested but physically unvalidated.

## Local timer

MODE cycles 5 minutes, 15 minutes, and Pomodoro, resetting paused. START/PAUSE
controls the countdown; RESET resets the chosen mode paused. START/PAUSE is a
full-width 50-pixel accent button; RESET and MODE are 48-pixel buttons beside
each other, all with matching touch zones. Pomodoro automatically
enters a five-minute rest after the focus phase, then stops at DONE.

The timer runs while other pages or call overlays are visible and does not need
BLE. Completion never changes the selected page and is silent by default.
Optional buzzer cues signal rest/completion when enabled; see [Audio](SOUND.md).
The timer is not persisted: reboot resets it. No background flashing is implemented.

## Eye interaction

A short tap on the Eyes page produces a temporary random expression for three
seconds, even with automatic random expressions disabled. It returns to the
selected base mood and does not write flash. Holds/swipes remain separate gestures.

Native tests cover layout bounds, tool navigation, session/default separation,
profile migrations, timer pause/rollover/work-rest completion, and tap restoration.
They also cover Pong bounce/scoring/rollover, paused-session restoration, clock
field validation and both panels' new tool controls. `PONG` and `CLOCK_EYES` in
the Mood grid are separate decorative animations; see the
[eye engine](../OTA-bot-Mochi/src/eyes/README.md).
Physical rendering, touch targets, and actual Chronos behavior still need testing
after a successful firmware build and upload.
