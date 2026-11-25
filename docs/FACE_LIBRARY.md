# Face library

[← README](../README.md) · [Docs index](README.md)

The allow-list applies to manual selection, automatic playback, taps, double taps,
petting, startup greetings, IMU reactions and shake-to-dizzy. At least one face
must remain enabled. Removing the selected face selects the first enabled ID.
An empty reaction pool leaves the current allowed face unchanged; a one-face
library never hangs trying to find a different expression.

## On the display

Open **Settings → FACES → Face library** (scroll down one row).
Tap a face to enable/exclude it; checked cells are enabled. Swipe vertically
through the list. SAVE persists the changes; BACK returns without undoing them.
The usual LOOK → Mood grid only selects enabled faces.

### Favorites

Scroll further in **FACES → Edit favorites** and tap a cell to toggle its `*`
star. **FACES → Favorites** opens a compact picker of starred, enabled faces.
Selecting one changes the configured mood just like LOOK → Mood. Empty favorites
show a message, not an implicit fallback. Excluding a face hides it from this
picker without deleting its star; enabling it again restores the entry.
Stars do not restrict automatic playback, tap reactions or the ordinary mood picker.

Stars change RAM immediately. BACK does not undo them; only explicit SAVE
persists them. The independent `mochi/favorites` record uses version/count,
bitmap and XOR checksum (35 bytes for 254 faces); appended IDs start unstarred.
Missing/corrupt records start empty without changing exclusions or the profile.
General defaults clears stars in RAM. The browser editor still edits exclusions
only, and SAVE FACES does not save pending stars or other settings.

## Refined everyday animations

The catalog now includes eight additional six-second performances, available in
the same display/browser library and USB mood selector:

| Canonical name | Performance |
| --- | --- |
| `hacker` | Eye terminals scroll code, scan, then show access-granted checks. |
| `berserk` | Eyes tighten and tremble beneath jagged energy discharges. |
| `portal` | Oppositely spinning apertures swallow the eyes and release them. |
| `origami` | Eyes fold into shaded paper kites with visible creases, then unfold. |
| `ghost` | Spectral eyes fade, drift and rematerialize with trailing wisps. |
| `eclipse` | Lunar discs cross two suns, leaving luminous coronas. |
| `detective` | A magnifying glass searches the right eye, then locks onto a detail. |
| `pixel_crash` | Eye tiles scatter on individual paths, then rebuild without Glitch's color tears. |

## Anime-inspired performances

Twelve original genre/reaction scenes were added after the first cinematic batch:

| Canonical name | Performance |
| --- | --- |
| `power_up` | Determined eyes charge a rising golden shonen aura. |
| `ninja` | A tied indigo headband, narrowed eyes and short dash afterimages. |
| `alchemist` | Double transmutation circles and rotating triangular sigils. |
| `magical_girl` | Pink transformation ribbons and a diamond tiara. |
| `mecha_lock` | Cyan tracking brackets converge around scanning amber eyes. |
| `spirit_seal` | Floating, marked paper charms above watchful violet eyes. |
| `sweat_drop` | An awkward asymmetric squint with an oversized sliding temple droplet. |
| `anime_vein` | A pulsing four-pronged anger mark above compressed eyes. |
| `chibi_panic` | Tiny trembling eyes beneath blue gloom strokes. |
| `tsundere` | A stubborn sidelong squint softens into a striped-blush wink. |
| `manga_shock` | Tall monochrome eyes inside radial manga impact lines. |
| `chibi_joy` | Bouncing smiling chevrons and sparse pastel confetti. |

Existing expression IDs remain unchanged. Older 146-, 154- and 166-face allow-lists
migrate to the 172-face catalog with old exclusions preserved and only appended
faces enabled. No settings-profile version or partition change is needed.

## Comic performances

Six more six-second gags bring the catalog to 172 faces:

| Canonical name | Performance |
| --- | --- |
| `rubber_band` | Eyes pull apart, flatten and rebound like elastic, retaining the selected ink. |
| `peekaboo` | Two miniature blue roller blinds hide the intact eyes, then lift for a peek. |
| `popcorn` | Squashed eyes jolt as a few buttery three-lobed kernels pop upward. |
| `spring_eyes` | Eyes hop on visible accordion coils with fixed lower anchors. |
| `comic_glasses` | Oversized hollow purple glasses slip unevenly, then wobble back. |
| `balloon_eyes` | Two pastel eye balloons inflate and float above curved strings before deflating. |

All six retain the configured style/palette at rest and support existing seeded
motion. Older exclusions remain intact; only newly appended IDs become enabled.

## Car performances

Nine six-second dashboard and road scenes bring the catalog to 181 faces:

| Canonical name | Performance |
| --- | --- |
| `wipers` | Two wiper arms sweep falling raindrops while the eyes follow the blades. |
| `turn_signal` | The eyes glance right, then left, beside one to three sequential amber arrows. |
| `fuel_low` | Drooping eyes under a blinking amber fuel pump; a gauge drains and turns red. |
| `drift` | Counter-steering eyes slide sideways over grey tyre smoke and skid strokes. |
| `car_wash` | A red/blue roller brush crosses the panel; each eye squeezes shut as it passes, among foam bubbles. |
| `parking_sensor` | Four distance bars light from green to a flashing red while the eyes widen and tremble. |
| `tunnel` | Relaxed eyes warm briefly as amber lamps and lane dashes stream past. |
| `gear_shift` | A red knob works through a five-speed gate; the eyes nod at each shift. |
| `check_engine` | Worried uneven eyes glance up at an amber engine lamp that blinks, then stays lit. |

These are decorative animations: they do not read vehicle data, fuel level,
distance sensors or diagnostic codes. A saved 172-face allow-list or favorites
record migrates with its exclusions and stars preserved; the nine appended faces
start enabled and unstarred. The records grow by one byte; no settings-profile
version or partition change is needed.

## More anime performances

Five more six-second manga reactions bring the catalog to 186 faces:

| Canonical name | Performance |
| --- | --- |
| `yandere` | A sweet raised squint suddenly snaps wide open and trembles under violet shadow lines. |
| `nap_bubble` | Closed eyes nod slowly while a hollow blue snot bubble swells and shrinks below one eye. |
| `gloom` | Lowered half-lidded eyes sit under a drifting grey raincloud with short blue drizzle. |
| `dot_eyes` | Both eyes collapse to tiny dots while a three-dot ellipsis types out and resets. |
| `steam_blush` | Squeezed, shivering eyes over wide pink cheeks, with steam puffs rising from both temples. |

All five keep the configured style and palette at rest, never add pupils to
MOCHI or TRAPEZ, and differ by silhouette and motion as well as color. A saved
181-face allow-list or favorites record keeps its exclusions and stars; the
five appended faces start enabled and unstarred. The records grow by one byte;
no settings-profile version or partition change is needed.

## Themed prop scenes

Sixty-eight six-second prop scenes bring the catalog to its full 254 faces
(IDs 186-253). Each keeps the configured style and palette at rest, adds no
pupils to MOCHI or TRAPEZ, and moves its props throughout the hold.

### Weather

| Canonical name | Performance |
| --- | --- |
| `snowfall` | Calm, slightly lowered eyes under drifting six-armed snowflakes. |
| `heatwave` | Wilting, wobbling eyes under a rotating sun with shimmering heat lines. |
| `rainbow` | Smiling raised eyes while a five-band rainbow arc draws itself behind them. |
| `thunderstorm` | Eyes flinch shut whenever a zigzag bolt drops from a drifting storm cloud. |
| `foggy` | Squinting eyes peer side to side through slowly drifting fog bands. |
| `windy` | Eyes squint and lean into gusts while wind curls and a leaf tumble past. |
| `hail` | Eyes wince under a red umbrella while icy hail pellets rain down on it. |
| `sunrise` | Closed eyes slowly open as an orange sun rises over a pink horizon. |
| `tornado` | Wobbling eyes are tugged sideways toward a swaying grey tornado funnel. |
| `frosty` | Shivering, ice-tinted eyes grow icicles that hang from their lower edges. |

### Food

| Canonical name | Performance |
| --- | --- |
| `ramen` | Happy eyes over a steaming red ramen bowl as chopsticks lift the noodles. |
| `ice_cream` | Eyes follow a strawberry ice-cream cone whose scoop melts and drips. |
| `coffee` | Drowsy eyes snap wide awake once the steaming coffee mug kicks in. |
| `pizza` | Hopping eyes over a pepperoni slice whose cheese string stretches upward. |
| `bubble_tea` | Eyes squeeze with each sip from a milk-tea cup of bouncing tapioca pearls. |
| `donut` | Eyes track a pink-frosted donut that glides back and forth with spinning sprinkles. |
| `onigiri` | Happy squinting eyes bounce with a hopping onigiri wrapped in nori. |
| `cookie` | Munching eyes as bites disappear one by one from a chocolate-chip cookie. |

### Animals

| Canonical name | Performance |
| --- | --- |
| `bunny` | Soft eyes under tall rabbit ears whose pink-lined tips twitch. |
| `frog` | Bobbing eyes perched on green frog mounds while a tongue flicks out. |
| `owl` | Owl eyes in brown feather rings with ear tufts and a beak; one slowly blinks. |
| `penguin` | Waddling eyes between flapping navy flippers above stepping orange feet. |
| `bee` | Eyes track a striped bee that buzzes along a figure-eight path. |
| `fish_bowl` | Slightly shrunken eyes inside a round glass bowl as a goldfish swims past. |
| `puppy` | Eager wide eyes between floppy brown ears, with a panting pink tongue. |
| `butterfly` | An orange butterfly flutters in and lands on the right eye, which narrows. |

### Anime reactions

| Canonical name | Performance |
| --- | --- |
| `shoujo_stars` | Enlarged dreamy eyes surrounded by twinkling gold and pink shoujo stars. |
| `chibi_cry` | Squeezed, trembling eyes spray two comic fountain arcs of tears. |
| `determined` | Narrowed eyes under set diagonal brows, with speed lines streaming behind. |
| `faint` | Eyes sag and close unevenly while a little ghostly soul floats up and away. |
| `kawaii_wink` | A blushing wink: the right eye closes and a gold star bounces away from it. |
| `sneaky` | Low, narrow eyes creep from side to side while tiptoe footprints appear. |
| `hypnotized` | Eyes sway after a swinging gold pendulum inside spreading violet rings. |
| `brain_freeze` | Icy-tinted, squeezed eyes shudder under three pulsing ice crystals. |

### Party and music

| Canonical name | Performance |
| --- | --- |
| `dj` | Eyes nod on the beat under a headphone band, with bouncing equalizer bars. |
| `karaoke` | Closed, happily singing eyes sway behind a microphone as notes float up. |
| `disco` | Bouncing eyes under a turning mirror ball while colored light spots roam. |
| `drum_roll` | Jittering, tense eyes above a red snare drum struck by alternating sticks. |
| `party_popper` | Delighted wide eyes as a gold party popper sprays curling streamers. |
| `birthday` | Smiling eyes over a pink birthday cake with three flickering candles. |
| `fireworks` | Wide upturned eyes watch three staggered radial firework bursts. |
| `maracas` | Eyes bounce in alternation while two dotted maracas shake at the sides. |

### Tech

| Canonical name | Performance |
| --- | --- |
| `wifi_search` | Searching eyes glance around under Wi-Fi arcs that light up one by one. |
| `download` | Eyes watch a green progress bar fill under a bouncing arrow, then smile. |
| `pairing` | Eyes take turns dimming while dots travel along a link between two rings. |
| `cursor` | A mouse pointer glides in from the corner and clicks the right eye. |
| `error_sign` | Shaky, wide eyes under a red warning triangle that pulses brighter and dimmer. |
| `sync_spin` | Eyes trace a small circle while two green sync arrows chase each other. |
| `charging` | Eyes open wider as a battery with a lightning bolt fills segment by segment. |
| `typing` | Lowered, focused eyes watch pseudo-random keys light up on a small keyboard. |

### Sports and games

| Canonical name | Performance |
| --- | --- |
| `soccer` | Eyes track a black-and-white patched ball bouncing across the bottom. |
| `basketball` | Eyes follow an orange ball that arcs into a netted hoop and drops through. |
| `bowling` | Eyes follow a purple ball rolling into three pins, which topple outward. |
| `dice_roll` | Bouncing eyes watch a white die tumble across, changing its face as it rolls. |
| `fishing` | Patient eyes stare at a floating bobber, then widen when it dips on a bite. |
| `yoyo` | Eyes ride up and down with a green yo-yo spinning on its string. |
| `juggling` | Eyes sway beneath three colored balls juggled in a cascade pattern. |
| `tennis` | Eyes snap left and right after a yellow ball bouncing over a low net. |

### Space and fantasy

| Canonical name | Performance |
| --- | --- |
| `ufo` | A hovering saucer shines a green tractor beam that gently lifts the eyes. |
| `rocket` | Eyes look up as a red-and-white rocket rumbles, then launches on its flame. |
| `planet_ring` | A small ringed planet orbits the face, passing behind and then in front of the eyes. |
| `black_hole` | Stretched eyes are pulled inward toward a spinning violet-and-orange accretion swirl. |
| `crystal_ball` | Violet-lit eyes gaze down into a crystal ball where pale mist swirls. |
| `wizard` | Focused eyes under a starry pointed hat, beside a wand trailing sparkles. |
| `dragon_egg` | Curious eyes watch a speckled green egg wobble harder, crack and puff a spark. |
| `vampire` | Red-tinted, narrowed eyes above a high crimson collar while a bat circles overhead. |
| `pumpkin` | Warm candle-lit eyes inside a ribbed orange pumpkin outline with a leafy stem. |
| `astronaut` | Weightless eyes float inside a round helmet visor while stars drift past. |

These are decorative animations; they read no weather, network, battery or
game data. A saved 186-face allow-list or favorites record keeps its exclusions
and stars; the 68 appended faces start enabled and unstarred. The records grow
by eight bytes; no settings-profile version or partition change is needed.

## Silhouette and legibility pass

Similar classic moods now differ by lid shape, not height alone: Squint pinches
into a lens, Tired droops at the outer corners, Glum sags low, Lazy and Focused
are deliberately uneven, Annoyed presses flat and tilted, Unimpressed rolls its
gaze up, Zen closes into calm downward arcs, Merciful adds smiling lower edges,
Sassy and Skeptic pair a lowered lid with a raised eye, Wondering lifts curved
lids toward an upward gaze, and Deadpan is a wide flat bar. Cat has almond lids,
Kiss closes happily with a small heart, Shy's blush and Sick's bubbles are
stronger, Fire/Moon/Sparkle/Clock Eyes are larger, the Look and Distracted
glances travel further and Ghost fades less.

Existing IDs and saved choices remain unchanged. Sleepy slowly nods closed,
briefly wakes, then settles; Curious lifts and leans toward a detail; Suspicious
narrows while checking both sides. Shy averts its gaze and briefly shows blush
hatches inside the lower lids. Love has a double heartbeat followed by a pause.
Glitch interrupts intact eyes with brief bounded chromatic tears, then fully
recovers between bursts. These use bounded frame-local drawing and never write
preferences. MOCHI and TRAPEZ remain pupil-free.
Shared micro-motion also varies smoothly across the rest of the catalog:
per-entry and per-face rhythms alternate quiet holds with livelier beats.
Repeated drawing of one frame is deterministic, finger tracking suppresses
random gaze drift, and Calm disables this extra variation. Randomness never
changes the selected expression, stored colors, controls or dice outcomes.
Self-contained scenes now also breathe and drift gently, using smooth seeded
samples rather than frame-by-frame random jumps. Both eyes share the movement;
scale varies by at most 2.4%, with roughly two logical pixels of drift on the
small panel. The first/last 800 ms stay exact, finger tracking suppresses drift,
and Calm disables the added movement. Complete loop lengths remain unchanged.

The dice page independently shows star/sad mini-eyes for a single D20's natural
20/1, without replacing the configured expression or hiding the die.

Mecha Break assembles the familiar alloy mask, cracks it with a localized impact,
then scatters rotating forehead, copper-optic, cheek and chin fragments. A short
shock ring and sparks accompany the explosion; there is no full-screen flash.
The configured eyes remain beneath the fragments and return cleanly.
Samurai's inclined eyes paint only their visible silhouette, preserving the
red sun behind the slanted upper edge instead of cutting a black triangle into it.

## From a browser

Tap **Wi-Fi** in the library. With saved Wi-Fi connected, scan the web QR from
the same home network. Otherwise join the password-protected network shown
on Mochi, then open `http://192.168.4.1` and choose **Face library** from the
initial settings page. The browser list supports search and restoring all
faces. SAVE FACES applies and saves the list, without saving unrelated RAM settings.
No account, internet access or external resources are needed.
The **Settings, Wi-Fi and weather** link opens display preferences, saved Wi-Fi,
automatic time and city selection; see [Web settings](WEB_SETTINGS.md).

This is a manually enabled, five-minute maintenance session, not a permanent web
server. Normal eyes, BLE servicing and utilities pause until BACK or timeout.
The existing USB settings terminal remains available in face/settings mode,
including manual network setup; it stays paused during firmware uploads.
Firmware uploads are rejected in face-library mode unless explicitly switching
to firmware mode from the settings page. Configuration-AP names and passwords
are generated once and persisted per device; request tokens stay random per
session and are never saved. The separately saved home-network
credentials and station connection survive session close. Use only while parked.

The separate version-one `mochi/faces` NVS record contains a catalog length,
bitmap and checksum. The separate seventeen-byte version-ten profile and its
migrations are unchanged. Missing/invalid records enable all faces; newly appended IDs are
enabled when loading an older valid bitmap. Defaults restores all faces in RAM.

## Local HTTP contract

Only available while the face-library Wi-Fi session is active, at Host
the displayed local address (AP or LAN). Responses are not cached. No CORS
permission is provided. AP HTML supplies a per-session `X-Mochi-Token` only
on its protected receiving interface; LAN JavaScript reads it from the display's
QR URL fragment. A LAN client cannot obtain it by forging the AP Host.

| Request | Contract |
| --- | --- |
| `GET /` | Self-contained HTML manager; rejects a different Host with 403 |
| `GET /faces` | Requires token; returns `names` and localized `labels` in stable ID order, lowercase hexadecimal `enabled`, numeric `revision`, and boolean `saved` |
| `POST /faces` | Requires token, `Content-Type: text/plain` and `X-Mochi-Faces-Revision` matching the current decimal revision; body is exactly the fixed-width lowercase hexadecimal bitmap |

Bitmap bit `id % 8` of byte `id / 8` means enabled. Bytes are encoded as two hex
digits each, in ascending byte order. For 254 faces the body has 64 characters;
the thirty-two-byte mask uses only bits 0 through 5 of its final byte. Its two reserved
high bits must stay zero. At least one bit must be set. Loading an older
record preserves its exclusions and enables only the subsequently appended faces.

POST returns the same state on success (200) or failed storage (507, changes
remain live in RAM and can be retried). Invalid masks/body/type return 400 without
changing state, stale revisions return 409, and wrong/expired credentials, Host,
or session mode return 403. A successful save does not restart the device.
The revision detects another browser's edits; reload before overwriting them.

Native callback tests cover authorization, masks, revisions, persistence failures,
session timeout and upload rejection. They do not verify the ESP32 HTTP parser,
radio, phone Wi-Fi behavior or physical touch calibration.
