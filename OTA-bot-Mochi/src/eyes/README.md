# Eyes Module (src/eyes)

Cozmo-style rounded-rectangle robot eyes with asymmetric per-eye geometry, plus Mochi/Bloub legacy styles and a rounded mirrored TRAPEZ style.

## Styles

Preset values below come from `eyes_set_style()` in `eyes_anim.cpp`, which is the
authoritative source; `eye_box()` applies expression changes and clamps on top.

| Style | Value | Geometry |
|---|---|---|
| NORMAL | 0 | Flat cartoon, 56x84, r0, asym off, no slant |
| MOCHI | 1 | Tall solid capsule, 44x100, gap 42, r22, asym off |
| BLOUB | 2 | Wide squishy body, 76x60, r24, asym on |
| TRAPEZ | 3 | Mirrored slanted body, 68x76, r0, slant 10, asym off |

MOCHI and TRAPEZ are solid: they never draw a pupil or a highlight.

TRAPEZ is filled row by row from the slanted quadrilateral and both ends are pulled
inward along a circular profile (`trapezoid()` in `eye_shapes.cpp`), so its corners
read as curves instead of sharp vertices. The bottom taper stays `w / 10` and the
softening radius is `max(radius, min(w, h) / 6)` with a 4px floor, so the shape is
rounded even though the TRAPEZ preset sets `radius` to 0. The left eye is always the
canonical evaluation and the right eye is its exact horizontal mirror, which keeps
the reflected halves pixel-identical.

## Expressions and animated moods (123)

Each expression has its own `faces/<canonical_name>.h`, containing its per-eye
`FacePreset` and expression-specific effects. `face_catalog.cpp` registers all
123 definitions; reusable primitives and the render pipeline remain shared.
For parameters, a compiled example and registration, see
[Adding a face](../../../docs/ADDING_A_FACE.md).

| Value | Name | Look |
|---|---|---|
| 0 | NEUTRAL | Relaxed default gaze |
| 1 | HAPPY | Lidded cheerful eyes |
| 2 | SAD | Drooping eyelids |
| 3 | ANGRY | Angled eyelids |
| 4 | SURPRISED | Enlarged wide eyes |
| 5 | SLEEPY | Narrowed tired slits |
| 6 | LOVE | Heart-shaped eyes |
| 7 | DIZZY | Cross-shaped wobbling eyes |
| 8 | GLEE | Joyful wide double arcs |
| 9 | WORRIED | Raised uneasy inner lids |
| 10 | FOCUSED | Low intense bars |
| 11 | ANNOYED | Half flat asymmetric lids |
| 12 | SKEPTIC | One big eye, one small eye |
| 13 | FRUSTRATED | Angled asymmetric lids |
| 14 | UNIMPRESSED | Flat half-mast lids |
| 15 | SUSPICIOUS | One high eye, one low eye |
| 16 | SQUINT | Narrowed slits |
| 17 | FURIOUS | Thick angled glare |
| 18 | SCARED | Small narrow tall eyes |
| 19 | AWE | Huge round wonder eyes |
| 20 | SLEEPY_EYES | Heavy asymmetric droop |
| 21 | SAD_DOWN | Sad gaze looking down |
| 22 | SAD_UP | Sad gaze looking up |
| 23 | EXCITED | Wide eyes with raised lower lids |
| 24 | TERRIFIED | Very tall eyes under a thin upper lid |
| 25 | DISGUSTED | Angled lid over a raised lower lid |
| 26 | SHY | Small lowered eyes |
| 27 | PROUD | Smug half-lids, mildly asymmetric |
| 28 | TIRED | Heavy droop on both lids |
| 29 | GRIN | Wide eyes behind a deep lower lid |
| 30 | PANICKED | Tall eyes pinched by two thin lids |
| 31 | GLUM | Small eyes under a double sad slope |
| 32 | SMUG | One eye far more closed than the other |
| 33 | DEADPAN | Flat centered line |
| 34 | ZOMBIE | One sagging eye, reduced height |
| 35 | CONFUSED | One wide eye beside one squinting eye |
| 36 | CURIOUS | One raised eye beside one lowered eye |
| 37 | WINK | One eye closed, the other open |
| 38 | WONDERING | One raised brow over an open gaze |
| 39 | STAR | Star-shaped eyes |
| 40 | CRYING | Sad lids above a falling tear |
| 41 | SWIRLY | Spiral eyes, a dizzy-drunk look |
| 42 | CAT | Vertical slit eyes |
| 43 | HALO | Halo ring set into the upper eye |
| 44 | DEVIL | Paired horns in the upper eye |
| 45 | FIRE | Flame-shaped eyes |
| 46 | MOON | Crescent-shaped eyes |
| 47 | SPARKLE | Four-point sparkle eyes |
| 48 | BOLT | Lightning bolt eyes |
| 49 | DIAMOND | Rhombus eyes |
| 50 | RING | Hollow ring eyes |
| 51 | TARGET | Crosshair eyes |
| 52 | MONEY | Currency-mark eyes |
| 53 | MILD_HAPPY | Gentle lidded smile |
| 54 | SMIRK | One raised corner over a flat lid |
| 55 | POUT | Puffed, low, and deliberately wide |
| 56 | KISS | Narrow centered pucker |
| 57 | NERVOUS | Small uneasy eyes, raised inner lid |
| 58 | SHOCKED | Wider and taller than SURPRISED |
| 59 | MELTING | Heavy downward droop on one side |
| 60 | LAZY | Symmetric half-mast with a low lift |
| 61 | DOZE | Almost fully closed resting slit |
| 62 | GROGGY | One eye far more open than the other |
| 63 | DAYDREAM | Wide, soft, and unfocused |
| 64 | GRUMPY | Frowning slope under a half-lid |
| 65 | SASSY | One raised brow over a flat lid |
| 66 | CHEEKY | A wink paired with a wide grin |
| 67 | FLIRT | Soft half-lids with a light lower lift |
| 68 | ZEN | Calm, narrow, and deeply relaxed |
| 69 | MERCIFUL | A kind, uneasy sadness |
| 70 | MANIC | Very wide eyes with tight pupils |
| 71 | SIDE_EYE | Both pupils pressed to one side |
| 72 | BEAMING | Wide joyful arcs, brighter than GLEE |
| 73 | MUSIC | Note-mark pupils on a calm gaze |
| 74 | LOW_BATT | Battery mark under drooping lids |
| 75 | POWER | Standby symbol in half-closed eyes |
| 76 | LOADING | Hourglass pupils while waiting |
| 77 | IDEA | Exclamation pupils, a sudden spark |
| 78 | HYPER | Very wide eyes under a thin raised lid |
| 79 | SQUISH | Wide eyes pressed down into a squish |
| 80 | DERP | Mismatched lids and a wandering gaze |
| 81 | GLITCH | Displaced horizontal glitch bars |
| 82 | TWITCH | One eye squinting, one squeezed shut |
| 83 | SNEEZE | Both eyes scrunched before the sneeze |
| 84 | JUDGE | Heavy half-lids over a raised lower lid |
| 85 | QUESTION | Question-mark pupils over an open gaze |
| 86 | CALCULATING | Furrowed asymmetric lids over an hourglass |
| 87 | CYBORG | One eye behind a visor slope, one plain |
| 88 | ARC_HAPPY | Curved smiling slits |
| 89 | ARC_SAD | Curved drooping slits |
| 90 | SQUEEZED | Mirrored closed-eye chevrons |
| 91 | TILTED | Opposing playful slants |
| 92 | PONG | Eyes morph into autonomous Pong paddles, then back; independent of desk Pong |
| 93 | CLOCK_EYES | Animated decorative dials driven by uptime, not local time |
| 94 | GAUGE | Eyes merge into a decorative tachometer and return, never speed/RPM data |
| 95 | GLOW | Selected-color-to-hues-to-selected-color cycle with soft halos around the current style |
| 96 | HEADLIGHTS | Slow-pulsing outward light cones behind the current style |

The first 118 expressions keep their numeric values. IDs 97–122 provide twenty-six
[original colored performances](../../../docs/ANIMATIONS.md), including their
names, timing and reference correspondence. The additions
omit mouths, cheeks and eyebrows. Existing LOVE, CRYING and angry faces remain
available. The appended `adore`, `laughing`, and `enraged` performances add
distinct scenery rather than replacing those ordinary moods.

Select these moods in Settings → LOOK → Mood, or use USB commands such as
`mood arc_happy`, `mood squeezed`, `mood pong`, and `mood clock_eyes`.
The remaining video effects use `mood gauge`, `mood glow`, and `mood headlights`.
Only SAVE persists the choice; the thirteen-byte version-five profile is unchanged.
Pong ignores idle gaze and blinks so its court stays readable. Clock eyes blink
with the other eyes and are decorative, not synchronized watches. For playable
Pong and a real clock, use the [desk tools](../../../docs/DESK.md).

GAUGE and PONG run entry-relative twelve-second loops: eyes for 0.8 seconds,
a 1.2-second smooth morph, the scene for eight seconds, the reverse morph for
1.2 seconds, then eyes again. Gauge eyes merge and hollow into the dial.
The needle accelerates, kicks back four times at the redline and returns to
idle, repeating twice while the dial is visible.
Pong eyes narrow and move into the paddles. Their dirty bounds include both
the original eyes and the scene throughout the transition.
GAUGE draws a 270-degree dial with a red zone and an autonomous limiter animation.
It ignores gaze/blinks and does not read phone speed, GPS, RPM
or vehicle data. GLOW starts with the selected eye color, holds it for 1.2 seconds,
then blends through six colors and back to the selected color in a 9.6-second loop. It paints
three halo layers using the current style's body shape; this overrides body
color for the frame only, not the user's saved palette. HEADLIGHTS adds
clipped, shaded outward cones joined beneath the rounded eyes. On selection
they expand from the eyes to the panel edges over 1.2 seconds, then pulse
on a 2.4-second brightness cycle. Their
dirty bounds include the entire dial, halos or cones, including during
blinks, gaze movement and return to another mood. No extra framebuffer,
delay or task is introduced.

### How an expression renders

Every expression must work on all four styles, which the native suite enforces:

- **Silhouette expressions** LOVE and STAR replace the whole eye on every style
  with a large coloured glyph. DIZZY and SWIRLY replace the whole eye on solid styles.
- **Engraved expressions** (CRYING, the ten symbol faces 43-52, and the glyph
  faces 73-77, 81, 85-86) draw the normal body first and then cut the glyph back
  out in the background colour at rest, so the silhouette survives and no pupil or
  highlight pixel is ever produced.
- **Lid expressions** only change eyelids and box dimensions, so they already work
  everywhere; WINK also carries an explicit lid so it stays visible when per-eye
  geometry is switched off.
- Non-solid styles (NORMAL, BLOUB) additionally shift the pupil, using either a
  plain circle or the same glyph as a coloured pupil, except for LOVE and STAR.

`glyph()` in `eye_shapes.cpp` dispatches to the selected face's glyph callback. Every primitive
stays inside a circle of radius `s` around its centre, which is what guarantees that
whole-eye marks and pupil-sized marks both stay inside the dirty region.

### Configured colours and distinct silhouettes

Resting expressions do not override the selected palette. LOVE and STAR use
the body color as whole-eye shapes; other glyphs use the configured pupil ink
on NORMAL/BLOUB and background-color engravings on MOCHI/TRAPEZ. The petting
heart marker also uses the rendered body color. `eye_palette.h` packs RGB565
values for temporary reaction and scene colors.

Related emotions differ in geometry rather than tint: smiling lids have
different curved profiles, sad eyes vary in opening and vertical position,
angry eyes narrow into deeper slopes, and startled eyes range from thin tall
capsules to round wonder eyes. Native tests compare occupied-pixel masks,
without relying on color differences.

Face-owned callbacks add three-second, entry-relative reactions to 24 ordinary
moods, informed by the user-supplied local GIF references. Closed chevrons,
sleepy arcs, flowing tear curtains, asymmetric eye openings, startled stretching,
and sneeze recovery remain bounded to the current eye geometry. LOVE,
ANGRY, GRUMPY, and FURIOUS also fade to themed body colors during the reaction,
then restore the selected ink. MOCHI/TRAPEZ spirals replace the entire eye.
`faces/crying.h` keeps clean neutral eyes and rounded closed lids above attached
blue water with soft downward highlights. The eyes widen as they close; remaining
water drains downward before reopening. Lids do not change to the water color.
No GIF assets, mouths, extra framebuffers, or saved settings are added.

Blink renders as a thin horizontal line. Legacy conditional asymmetry applies only when
`cfg.asym` is set, which `eyes_set_style()` enables for BLOUB alone: SKEPTIC,
SUSPICIOUS, SLEEPY_EYES, PROUD, SMUG, CONFUSED, CURIOUS, WONDERING, ZOMBIE, WINK,
SMIRK, MELTING, GROGGY, SASSY, CHEEKY and FLIRT. On the other three styles those
faces fall back to their lid masks, which is why each one carries a distinct lid.

## Panel adaptation

Style presets are tuned on the 240x280 reference panel (`EYE_REF_W`/`EYE_REF_H`).
`eyes_init()` records the canvas size into `cfg.screenW`/`cfg.screenH`, and
`eye_box()` recenters the pair on that canvas and scales every measure by the
smaller axis ratio, bounded to 50..260 percent. Panels larger than the
reference then reinvest `EYE_FILL_EXCESS` percent (default 50, `-D` overridable)
of the extra ratio into the face so it does not shrink against the black
gutters. The bonus is capped per style by `eye_scale()`: the nominal width plus
the widest expression plus the minimum gap plus the full +/-20 px gaze range
must fit the panel width, so a wide style such as BLOUB fills less than a
narrow one instead of clipping its pupils during a gaze shift. Eyes too short
for pupils still draw their lid mask, which keeps half-closed names distinct on
small panels.

## Gaze and IMU

`motion_filter.h` turns the mount-relative accelerometer delta into a damped pixel
offset:

- `bounded()` clamps the gaze to +/-14 px.
- Gains are `delta_x * 22` and `-delta_y * 16` with a 0.035 g dead zone, damped by a
  0.15 factor per 50 ms sample.
- The baseline recenters only below 0.01 g of combined energy, so a sustained
  cornering load above roughly 0.1 g keeps driving the gaze instead of averaging
  itself back to centre over a few seconds.

The offset reaches the eyes two ways: `eye_box()` shifts the whole pair by `tiltX`
and rolls them by `tiltX / 3` (bounded to 4 px) so a corner reads as a lean, and
`eye_pupils()` shifts the pupil on the two non-solid styles.

The sign of the roll is a mount-orientation detail that still needs verification on
real hardware; see `TODO.md`.

## API

| Function | Purpose |
|---|---|
| eyes_init(gfx) | Bind display, BLOUB/NEUTRAL/B/W defaults |
| eyes_set_style(s) | NORMAL/MOCHI/BLOUB/TRAPEZ + geometry preset |
| eyes_set_expression(e) | Set Expression enum directly |
| eyes_set_expression_by_name(n) | Case-insensitive name parser, bool match |
| eyes_set_colors(bg, fg) | Background + body shortcut |
| eyes_set_eye_colors(bg, fg, pupil, hl) | Full palette in one call |
| eyes_set_bg / _fg / _pupil / _highlight | Single channel setters |
| eyes_get_bg / _fg / _pupil / _highlight | Single channel getters |
| eyes_get_colors(...) | Full palette getter, nullable outs |
| eyes_set_slant(s) | TRAPEZ corner-lift px |
| eyes_set_lid(l) | Eyelid line offset px |
| eyes_set_asym(a) | 0 symmetric, 1 per-eye vary |
| eyes_update(ms) | Timers + IMU + redraw |
| eyes_draw() | Redraw from config/state |
| eyes_imu_init / _poll / _set_tilt / _trigger_shake | Tilt + shake-to-dizzy |

The name parser accepts every canonical `eyes_expression_name()` value, for example
`sleepy_eyes`, `mild_happy`, `side_eye`, plus the two aliases `saddown` and `sadup`.
USB exposes the same table through `mood names` and `set mood <name>`.

## Examples

```cpp
#include "src/eyes/eyes_api.h"

// Cozmo cyan look: black bg, cyan rounded bodies, black pupils.
eyes_init(gfx);
eyes_set_style(BLOUB);
eyes_set_eye_colors(BLACK, CYAN, BLACK, CYAN);
eyes_set_expression(SKEPTIC);

// Mochi white look.
eyes_set_style(MOCHI);
eyes_set_eye_colors(BLACK, WHITE, BLACK, WHITE);
eyes_set_expression_by_name("glee");

// Trapez corner-lifted custom eye.
eyes_set_style(TRAPEZ);
eyes_set_slant(14);
eyes_set_expression(AWE);
eyes_update(millis());
```
