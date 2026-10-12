# Eyes Module (src/eyes)

Cozmo-style rounded-rectangle robot eyes with asymmetric per-eye geometry, plus Mochi/Bloub legacy styles and a corner-lifted TRAPEZ style.

## Styles

| Style | Value | Geometry |
|---|---|---|
| NORMAL | 0 | Flat cartoon, 56x84, r10, small pupils |
| MOCHI | 1 | Rounded, 62x92, r46, pupil + highlight |
| BLOUB | 2 | Default large body, 64x96, r28, asym on |
| TRAPEZ | 3 | Corner-lifted parallelogram, 66x88, r14, slant 14 |

TRAPEZ pulls the high-left corner up by `slant` px (fillTriangle lift over a fillRoundRect base), then draws the standard pupil + highlight.

## Expressions (23)

| Value | Name | Look |
|---|---|---|
| 0 | NEUTRAL | Relaxed default gaze |
| 1 | HAPPY | Lidded cheerful eyes |
| 2 | SAD | Drooping eyelids |
| 3 | ANGRY | Angled eyelids |
| 4 | SURPRISED | Enlarged wide eyes |
| 5 | SLEEPY | Narrowed tired slits |
| 6 | LOVE | Heart-shaped pupils |
| 7 | DIZZY | Cross wobbling pupils |
| 8 | GLEE | Joyful wide double arcs |
| 9 | WORRIED | Raised uneasy inner lids |
| 10 | FOCUSED | Low intense bars |
| 11 | ANNOYED | Half flat asymmetric lids |
| 12 | SKEPTIC | One big one small |
| 13 | FRUSTRATED | Angled asymmetric lids |
| 14 | UNIMPRESSED | Flat half-mast lids |
| 15 | SUSPICIOUS | One high one low |
| 16 | SQUINT | Narrowed slits 45pct |
| 17 | FURIOUS | Thick 18px angled glare |
| 18 | SCARED | Small narrow tall eyes |
| 19 | AWE | Huge round wonder eyes |
| 20 | SLEEPY_EYES | Heavy asymmetric droop |
| 21 | SAD_DOWN | Sad gaze looking down |
| 22 | SAD_UP | Sad gaze looking up |

Blink renders as a thin horizontal line; asymmetric expressions offset left vs right eye (SKEPTIC, SUSPICIOUS, ANNOYED, FRUSTRATED, WORRIED, SAD_DOWN, SAD_UP, SLEEPY_EYES).

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

Name parser accepts: neutral, happy, glee, worried, focused, annoyed, surprised, skeptic, frustrated, unimpressed, sleepy, sleepy_eyes, suspicious, squint, angry, furious, scared, awe, love, dizzy, sad, sad_down/saddown, sad_up/sadup.

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
