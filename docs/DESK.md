# Contextual views

[← README](../README.md) · [Docs index](README.md)

The primary carousel contains eight pages: Eyes, Clock, Music, Route, Alerts,
Weather, Settings and Drive. Extra tools are grouped under Clock, not added as more
swipe destinations. The device does not infer whether it is in a car or on a desk.
Use touch controls only while parked.

## Drive dashboard

**DRIVE / CRUSCOTTO** is a primary page between Weather and Settings (two swipes
right from Eyes, or BOOT after Weather). It is meant for a dash or handlebar
mount; operate it only while stopped.

Drive uses the Clock layout with lean in place of the time: a large L/R value,
then total G with FWD/AVANTI or BACK/INDIETRO pitch, session lean maxima, and
live OBD speed/RPM when a dongle reports them. Like Clock → Scrivania, **swipe
down** for **DRIVE TOOLS / STRUMENTI GUIDA**, a tile menu:

| Tile | Content |
| --- | --- |
| BIG LEAN / PIEGA GRANDE | Huge L/R lean, pitch and session lean maxima |
| G-METER | Large friction circle (acceleration, not felt force; 0.5 g / 1 g rings), total G, max acceleration/braking/total |
| LEAN GRAPH / GRAFICO PIEGA | Ten seconds of lean (accent, ±60°) and lateral G (warm, ±1.2 g) |
| TRIP / VIAGGIO | Synchronized clock, trip time since zero, lean/acceleration/braking/total peaks, phone next turn when a route is active |
| OBD | Engine data from a BLE ELM327 dongle (see below) |

Tap a tile to open it; swipe up from a tool to DRIVE TOOLS and from DRIVE TOOLS
to Drive. Swiping down on a tool or on the menu stays there; horizontal swipes
move relative to Drive. Every motion tool has a **ZERO / AZZERA** button that
resets the reference, trip time, peaks and graph. Values are RAM-only.
Shortcuts: **hold** on the Drive home or on a motion tool does the same ZERO;
**tap** the Drive home (below the status row) to reopen the last tool opened
from DRIVE TOOLS in this session, or DRIVE TOOLS itself the first time. The
home also shows a lean gauge: a dot slides left/right up to 60°.

The first stable reading after the page opens or after ZERO defines "up" and
the gyroscope bias (the device must be still: under about 6°/s). Screen X
projected on the horizontal plane is "right" and "ahead" completes the frame,
so any mount works without settings. Lean is positive when the right side goes
down; pitch shows FWD/AVANTI when the front is lower than at ZERO. Zero it with
the vehicle upright and stopped.

Lean fuses the QMI8658 gyroscope with the accelerometer: the roll rate is
integrated and pulled (about 2.5 s time constant) toward a reference. While
turning faster than about 9°/s the reference is the ratio of the body-right to
body-up yaw-rate components, which equals tan(lean) in a coordinated turn and
needs no speed; on straights it is accelerometer tilt. A car's body roll is
small, so in a car the G-ball is the useful part. The gyroscope is powered only
while this page is visible and the accelerometer keeps its usual 50 ms poll; the
page redraws every 100 ms and keeps the backlight from idle-dimming. No GPS
speed is read.

### BLE OBD dongle

Mochi can read a **BLE** ELM327 OBD-II dongle (for example OBDLink CX, Vgate
iCar Pro BLE, vLinker BLE). Classic-Bluetooth-only dongles cannot connect: the
ESP32-S3 has BLE only. Wi-Fi dongles are not supported.

While the Drive page is visible and **Settings → SYSTEM → OBD dongle scan** is on
(default; USB `set obd-scan on|off`), a priority-1 worker scans for four seconds
every ten seconds. Candidates are devices whose name contains OBD, ELM,
VLINK, VGATE, ICAR, VEEPEAK, KONNWEI or OBDLINK, or that advertise the serial
services FFF0, FFE0, 18F0 or E7810A71-…. The last dongle that completed GATT
discovery is remembered (its address only, in the `mochi-obd` NVS namespace) and
preferred; otherwise the strongest signal wins. With the scan off the OBD tool
shows OBD SCAN OFF and no BLE central activity starts. It subscribes to the first notify
characteristic and writes to the first write characteristic of that service. No
pairing is configured. The Chronos/ANCS phone link stays as is; Mochi is central
and peripheral at once.

The session sends ATZ, ATE0, ATL0, ATS0, ATH0, ATSP0, probes 0100, then polls
RPM (010C) and speed (010D) every round plus one of coolant (0105), throttle
(0111), fuel level (012F), adapter voltage (ATRV) or monitor status (0101).
0101 gives the check-engine lamp and the stored-trouble-code count, shown as
CHECK ENGINE n DTC (warm while the lamp is on) or NO STORED FAULTS; codes are
not listed and never cleared. A PID unsupported three
times is skipped until the next probe. Three empty engine replies mean ignition
off: the view shows IGNITION OFF / QUADRO SPENTO and keeps reporting battery
voltage while it re-probes. Values older than three seconds turn grey; missing
ones show `--`.

Leaving the page stops polling; the link is dropped after one minute hidden so
the dongle can sleep. Only the dongle address is stored; vehicle values are not. Only ELM327 text commands are sent; no
DTC clearing or writes to the vehicle are implemented. Motorcycles often lack
standard OBD-II; check the bike's connector and protocol first.

## Navigation

Route uses the configured large/details default when a new session begins.
Large mode shows a 96x96 received turn icon, emphasized distance, and two larger
instruction lines. Missing/incomplete icons remain placeholders, never guessed
turn arrows. Details retains speed, ETA/duration, and the smaller turn card.

Swipe down on Route to switch temporarily between large and details. Hold on
Route for 1.2 seconds to restore the configured default. Choose and SAVE
**Settings → SYSTEM → Route default** for future
boots. [Settings](SETTINGS.md) documents USB commands and session/save separation.
Starting real navigation in Chronos opens Route as before; repeated route updates
do not pull you back after a manual page change.

## Desk tools

Swipe down on Clock to open Scrivania, then select:

| Tool | Content |
| --- | --- |
| BIG CLOCK | Large hours/minutes, seconds, AM/PM when applicable, and date |
| DESK OVERVIEW | Time, phone connection, temperature/city, and weather freshness |
| TIMER / POMODORO | Local 5-minute or 15-minute timer, one 25-minute focus/5-minute rest pair, or a stopwatch |
| DEVICE STATUS | BLE connection, backlight percentage, IMU initialization, clock sync, preference save state, sound state, battery estimate, and a charge bolt while the PMU reports charging |
| ANALOG CLOCK | Hour, minute and second hands with synchronized digital time below |
| PHONE TOOLS (Android only) | Find phone, camera, battery, alarms, reminders, hourly weather, contacts, QR and language association; see [Chronos](CHRONOS.md) |

The hub shows six 108x44-pixel cards in Android mode and five in iPhone mode,
in a two-column, three-row grid. Gaps between cards are
not tap targets. Coordinates scale for both the 240x280 LCD and 368x448 AMOLED.

## Games and D&D dice

Swipe **down again from Scrivania** to open **GAMES / GIOCHI**. It groups:

| Game | Content |
| --- | --- |
| POLYHEDRAL DICE / DADI POLIEDRICI | Roll 1–100 dice of one selected type: D4, D6, D8, D10, D12, D20 or D100 |
| PONG | Play against an automatic right paddle; tap the court at the desired height to move the left paddle, RESET starts a new game |
| TIC TAC TOE / TRIS | You play X against Mochi O; tap an empty cell, NEW GAME clears the board |
| FLAPPY BIRD | Tap the court to start, flap through the pipes, or retry after a collision |

Dice starts at **1D20**. Use the top `<` / `>` buttons to change die type.
The main view shows a large projection of the actual solid: tetrahedron,
cube, octahedron, pentagonal trapezohedron, dodecahedron or icosahedron.
Its number is the first die's result, not the pool total, which appears below.
D100 uses two D10s for tens (00–90) and units (0–9); **00 + 0 means 100**.
A single D20 shows Mochi's star eyes on a natural **20** and sad eyes on a
natural **1**, beside the solid. These badges do not navigate away, change the
selected face or palette, reroll, or write preferences. Other results and
multi-die pools have no critical badge.
Open **details / dettagli** to adjust the count by **−10, −1, +1 or +10**,
clamped to 1–100. This separate panel contains the sum and individual results;
**BACK / INDIETRO** returns to the solid without rerolling.
The four count actions are 48×44 design pixels. ROLL and DETAILS/BACK use
116-pixel-wide buttons from design row 224 to four physical pixels above the
panel bottom. This replaces the tiny lower strip and carousel dots on the dice
page with taller touch targets; the gutter between the two buttons stays inert.
Tap **ROLL / LANCIA** or shake for a new roll. Shake works only while the dice
page is visible, independently of the **Shake dizzy** face setting. It uses the
existing deliberate multi-reversal gesture and five-second cooldown; no IMU
means touch-only rolls. Shaking on that page does not trigger Dizzy.
Each die is drawn once using Arduino's
bounded random source; repainting, waiting or returning to the page never rerolls.
Details show the sum and twelve individual values per page. Tap the
left/right half of the result card to browse pages, indicated by `<1/9>` for
a 100-die pool. D100 results are 1–100, not 0–99. Changing count/type clears
the old result. The selection, last roll and page remain in RAM across navigation;
reboot resets to 1D20. No preferences are written.

Rolls are homogeneous `XdY` pools, not mixed expressions. Modifiers and automatic
advantage/disadvantage rules are not implemented. Games and dice also work in
iPhone mode and without a phone connection.

Pong runs at the display frame cadence, pauses off-page or behind a call overlay,
and caps physics catch-up at 100 ms. Scores wrap after 99. Game state stays in
RAM and never writes preferences. Taps move the paddle; holds and swipes keep
their normal navigation behavior.

Tris shows whose turn it is, a winning stroke, and win/loss/draw messages in
English or Italian. Mochi replies after a non-blocking 450 ms thinking pause:
it takes an immediate win, blocks your immediate win, then prefers the center
and corners. It is a simple tactical opponent, not an unbeatable solver.
Occupied cells and taps during Mochi's turn are ignored. A finished board stays
visible until NEW GAME. Leaving the tool preserves the RAM-only match; reboot
clears it. The `tic_tac_toe` Mood is a separate decorative demo.

Flappy Bird uses an original small bird drawing, two scrolling pipes and
10 ms physics steps with at most 100 ms catch-up. Tap to start/flap; a pipe,
ceiling or floor collision ends the game. Tap again to retry. The display shows
score / session best, kept only in RAM. Leaving the page or showing a call
overlay pauses physics without simulating time away on return.

Swipe up from a desk tool to return to Scrivania. From Dice, Pong, Tris
or Flappy Bird, swipe up to Games, then up to Scrivania, then up to Clock. Swiping down on Games
or Dice stays on the same page. A horizontal swipe from a tool goes to the previous/next primary page
relative to Clock, so the carousel still has only eight destinations. The Clock
position dot stays selected in these tools.

Time requires a Chronos sync after boot and then continues while disconnected.
Analog Clock uses 24-hour source time for its hands, regardless of the digital
12/24-hour preference. Missing or malformed time shows a sync prompt rather
than invented hands. The main Clock and BIG CLOCK keep their existing layouts.
Weather uses received phone data and has missing/stale states; it is not measured
by the board. Device status shows initialization, not vehicle telemetry. The
battery row is a best-effort LiPo estimate from an unverified ADC divider on
`BAT_ADC` (LCD) or the AXP2101 battery-voltage ADC (AMOLED V2); implausible
readings (no battery fitted) are reported as NOT MEASURED,
and the conversion is native-tested but physically unvalidated. A charge bolt
appears beside the row while the AMOLED PMU reports a stable charging state.

## Local timer

MODE cycles 5 minutes, 15 minutes, Pomodoro and Stopwatch, resetting paused.
Stopwatch counts up (ELAPSED / TRASCORSO), switches to H:MM:SS after an hour
and stops at 99:59:59 as DONE. START/PAUSE
controls the countdown; tapping the large time does the same, and a **hold**
anywhere on the timer resets it paused, like RESET. START/PAUSE is a
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
Dice fixtures exhaust every face of all seven types, 1/100-die bounds, the
10,000 maximum sum, count/type edits, pagination, inert touch gaps, navigation,
EN/IT labels and result persistence on LCD, taller LCD and AMOLED layouts.
They do not establish hardware random-source quality or physical touch usability.
Flappy Bird checks cover timing, collisions, scoring, restart, wrap and page/call
pause restoration. The real IMU-driver fixture covers shake routing with face
shake disabled, cooldown and off-page isolation. Both new features retain the
existing large hub tiles and the primary carousel.
