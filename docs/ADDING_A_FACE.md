# Editing and adding a face

Every existing expression has one file in `OTA-bot-Mochi/src/eyes/faces/`:
123 files, named after the canonical USB names. Each file owns an immutable
`FacePreset` and its expression-specific drawing and animation procedures.
For example, edit `faces/curious.h` for CURIOUS or `faces/crying.h` for CRYING.

`face_catalog.cpp` registers these definitions in public ID order. Generic panel
fit, dirty-region rendering, style bodies, interpolation and reusable drawing
primitives stay shared. Public names, translations, sound policy, duration,
priority and saved settings remain in their existing tables.

## Per-eye parameters

For the simple path, edit an existing preset's named dimensions/openness/gaze,
keep its name/ID and callbacks, then run the native tests. No new registration
or advanced callback knowledge is needed. Registration is only for a new face.

The firmware uses C++17. Use a constexpr lambda with named assignments, rather
than C++20 designated initializers. This compiled example is also available in
[`examples/simple_face.h`](examples/simple_face.h):

```cpp
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "my_curious";
	p.left.widthScale = 1.0f;
	p.left.heightScale = 1.15f;
	p.left.openness = 1.0f;
	p.right.widthScale = 0.9f;
	p.right.heightScale = 0.65f;
	p.right.openness = 0.7f;
	p.gazeX = 0.15f;
	p.gazeY = -0.10f;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid face parameters");
```

This is a custom example, not an additional registered expression. When editing
an existing face, keep its canonical `name` and existing callbacks unless you
intend to change those effects too.

| Field | Supported range | Default | Meaning |
|---|---|---|---|
| `left/right.widthScale` | 0.2..2.6 | 1 | Multiplier of the selected style's reference width |
| `left/right.heightScale` | 0.1..2.6 | 1 | Multiplier of its reference height |
| `left/right.openness` | 0..1 | 1 | Additional height multiplier |
| `gazeX`, `gazeY` | -1..1 | 0 | Gaze bias, added before existing gaze clamps |

Scale multipliers are rounded to integer percentages, then use integer division.
This retains the existing raster rounding. Width and height remain subject to
the renderer's clamps, so `openness = 0` produces the minimum resting height,
not a fully closed eye. Blink still overrides height with six reference pixels.
Gaze biases contribute up to 20 horizontal and 18 vertical reference pixels;
they do not replace saccades, IMU motion or pupil rules.

For exact existing geometry, each eye also supports `widthDelta` (-32..32),
`heightDelta` (-130..130), `offsetY` (-32..32), an optional `height` override
(0 means unused, otherwise 1..130), and `heightFromWidth`.
The nested `asym` adjustments run only with `cfg.asym && !st.blinking`.
The normal `left` and `right` parameters always apply; they can differ even
when that legacy asymmetry option is off.

The order is width/height scales and deltas, optional square/height override,
openness, nominal-width clamp, conditional asymmetry, final size clamps,
blink override, gaze/bank, then panel scaling. Keep custom combinations within
the panel budget and test them on the smallest supported panels.

`face_preset_valid()` rejects unsupported ranges, non-finite numbers and empty
names. Every face and the complete ID-ordered catalog have compile-time checks.
Native tests also check that preset names match the public name table.

## Advanced: effects in the same file

Face-specific procedures live beside the preset. Attach only the callbacks the
face needs: `lids`, `glyph`, `reactionMotion`, `reactionBody`, `referenceBody`,
`performanceMotion`, `performanceColor`, `performanceScene`, `scenery`,
`background`, `bounds`, `layout`, `drawing` or `scene`.

Use existing `face_primitives` helpers from `face_support.h` for reusable shapes,
lid cuts, colors and interpolation. A callback may draw or calculate frame-local
geometry, but must not block, log to Serial, persist settings, or allocate another
framebuffer. Preserve the existing clocks and transient-palette restoration.

For example, TIRED's exact one-third top and one-quarter bottom lid cuts remain
in `faces/tired.h`, instead of approximating them with percentage cuts.
CRYING owns its water/lid procedures; GAUGE and PONG own their scene procedures.
The old effect headers are compatibility includes, not places to add face code.

## Style behavior

| Style | Reference body | Default asymmetry | Pupil rule |
|---|---|---|---|
| NORMAL | 56x84 | off | Existing cartoon pupils |
| MOCHI | 44x100 | off | Solid, no pupils or highlights |
| BLOUB | 76x60 | on | Existing pupils and highlights |
| TRAPEZ | 68x76 | off | Solid mirrored trapezoids, no pupils or highlights |

`eyes_set_asym()` still overrides the style default. Existing conditional
asymmetry is unchanged. Presets do not replace style-specific bodies or the
configured palette. `eye_geometry.h` is authoritative for fit and clamps.

## Registering a genuinely new face

1. Add `faces/<canonical_name>.h`, with a unique namespace, named constexpr
   preset, range assertion and any expression-specific procedures.
2. Append an explicit numeric `Expression` value in `eyes_types.h`; never
   renumber existing IDs. Update `EXPRESSION_COUNT` and its assertion.
   Preserve the existing `uint8_t` representation and reserved bounds.
3. Include the new header in `face_catalog.cpp` and append its explicit ID and
   preset in `FACE_PRESETS`, at the matching index.
4. Append the canonical lowercase name to `eyes_expression_name()` in ID order.
   Existing C++ aliases `saddown` and `sadup` remain unchanged. USB accepts
   canonical names, not those aliases.
5. Add EN/IT labels to `UI_TRANSLATIONS` in `src/ui/i18n.h` and append the
   matching entry to `expressions` in `i18n.cpp`; update its range assertion.
6. Check face policy, sound, timing and range-dependent code before deciding
   whether the new face needs registration there. Catalog membership alone
   does not add sounds, duration or automatic selection.
7. Add native geometry/raster coverage and update the public expression list.
   Verify USB selection and settings encode/decode. Adding a face does not
   itself require changing the persisted settings format.

Editing an existing face requires no new ID, translation or settings version.
Adding a new implementation `.cpp` requires updating both native runners and
both emulator runners; Arduino/PlatformIO discover `src/` modules recursively.

## Regression baselines

Both raster fixtures were captured from the preserved, uncommitted workspace,
not `HEAD`. Never regenerate them from a refactored renderer to hide a mismatch.

- `tests/fixtures/face_raster.h`: initial three-face migration baseline,
  2,952 fingerprints covering 6,912 sampled frames.
- `tests/fixtures/all_faces_raster.h`: full migration baseline, 2,952
  fingerprints covering 47,232 sampled frames. All 123 faces have 16 samples.

Both cover six panels (240x280, 240x320, 320x480, 368x448, 128x160, 160x128)
and all four styles. The expanded fixture exercises asymmetry, blink, two
gaze/tilt states, two palettes including a non-black background, and sampled
timestamps from 0 through 15 seconds.

The tests fingerprint every RGB565 pixel and dirty bounds. Each dirty frame
is also compared pixel-for-pixel with a full redraw at the same state. Palette
changes explicitly invalidate the full canvas, matching the renderer contract.
These are regression samples, not proof for every timestamp, custom geometry,
toolchain, or physical panel. Test new custom dimensions separately.

`RECORD_FACE_BASELINE` exists only for deliberate captures from an independently
preserved pre-change renderer. Normal test runners compare, never rewrite fixtures.

## Commands

USB vocabulary is English; settings stay in RAM until explicit SAVE:

```text
mood names
mood tired
set mood focused
mood sleepy_eyes
save
```

From the repository root:

```powershell
.\scripts\test-host.ps1
git -c core.whitespace=cr-at-eol diff --check
```

On Linux: `bash scripts/test-host.sh` and `git diff --check`.
Both runners compile the guide example and check parameters, catalog IDs,
names, aliases, USB selection, settings round-trips and both raster fixtures.

Firmware compilation, without upload:

```text
pio run -e esp32s3-lcd -e esp32s3-amoled
```

Run only one PlatformIO build per build directory at a time. Keep OPI PSRAM
disabled on LCD 1.69 OLD and enabled on AMOLED V2. Native tests do not validate
physical display, IMU, touch or BLE. The preset dispatch adds no task,
framebuffer, dependency or dynamic allocation.
