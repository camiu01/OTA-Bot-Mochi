# Face library

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
bitmap and XOR checksum (25 bytes for 172 faces); appended IDs start unstarred.
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
digits each, in ascending byte order. For 172 faces the body has 44 characters;
the twenty-two-byte mask uses only bits 0 through 3 of its final byte. Its four reserved
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
