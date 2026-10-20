# Original colored animations

The local `sd_content` collection from
[watcher-mochi](https://github.com/pham-tuan-binh/watcher-mochi) was reviewed as a
visual reference. Its README separates the Apache-2.0 firmware from proprietary
Dasai GIF artwork. None of those GIFs, extracted frames, textures, or traced
shapes are included in this firmware.

The additions here are original procedural drawings. They preserve broad themes
such as rain, a visualizer, or shrinking eyes, not the source choreography.
Mouths, complete cartoon faces, source text and logos are omitted. Similar
expressions reuse existing moods instead of adding duplicate menu entries.
The correspondence below is thematic, not a claim of frame-for-frame playback.

## New moods

Select Settings > LOOK > Mood, or type `mood dancing` in the USB/emulator console.
The same renderer supports LCD and AMOLED and all four eye styles.

[LCD still gallery](images/colored-performances-240.png) |
[AMOLED still gallery](images/colored-performances-368.png).
These are native raster previews, not device photographs.

| ID | USB name | Original drawing |
| --- | --- | --- |
| 97 | `dancing` | Twelve moving equalizer columns; the two eye targets become ordinary columns, without overlays |
| 98 | `police` | Slowly blended red/blue eye colors, opposite on each side |
| 99 | `rain` | Drooping blue eyes beneath rain that enters at the top edge and falls across the screen |
| 100 | `growing` | Green eyes expand and breathe |
| 101 | `shrink` | Violet eyes contract and recover |
| 102 | `swinging` | Amber eyes swing from thin pendulum strings |
| 103 | `blinding` | Amber squint with rotating soft rays, no full-screen flash |
| 104 | `buzzing` | Yellow eyes vibrate beside short motion rays |
| 105 | `sick` | Green eyes sway beneath rising bubbles |
| 106 | `splash` | Blue eyes, a small wave and droplets |
| 107 | `spraying` | Cyan eyes with outward droplet jets |
| 108 | `sushi` | Both eyes merge into one centered nigiri with striped salmon, then separate |
| 109 | `energetic` | Two large car turbochargers with inward-facing intakes, rotating compressors and outward smoke bursts |
| 110 | `smoke` | Teal eyes beneath fading puffs |
| 111 | `menacing` | Red eye rings surrounded by orbiting dots |
| 112 | `hello` | Peach-colored bouncing eyes and a wink, no text |
| 113 | `distracted` | Mint eyes glance independently |
| 114 | `look_left` | Blue eyes glance left, then return |
| 115 | `look_right` | Pink eyes glance right, then return |
| 116 | `look_down` | Violet eyes glance down, then return |
| 117 | `stargazing` | Small blue eyes settle together at the bottom, then look up at slowly twinkling stars |

Each addition uses an entry-relative six-second loop: selected-color eyes for
0.6 seconds, a 0.9-second eased transformation, three seconds of performance,
a 0.9-second return, then 0.6 seconds of the initial eyes. The body colors and
accents are frame-local; they never change the Color preference or write NVS.
The `police` color blend is decorative, not an emergency warning signal.
In `stargazing`, the eyes shrink to about 35% and settle close together along the bottom edge.
They then lift slightly and open toward the sky; NORMAL/BLOUB pupils also move
upward, while MOCHI/TRAPEZ remain solid. Stars fill the upper three quarters,
fade in as the gaze lifts, then fade out before the eyes return to their initial
size and position.
Neither the turbochargers nor the sky represent sensor or astronomical data.
Turbo housings grow to roughly one third of the panel width each. Their intake
faces tilt toward the center; smoke puffs rise from the outer exhausts and fade.
Dancing draws its morphing eyes after the background columns, so intermediate
frames cannot acquire black vertical cuts.

Automatic Random/IMU/shake reactions wait for the complete visible cycle.
Deliberate touch reactions can still interrupt it. Returning from a reaction
starts a fresh complete cycle. The original IDs 0–96 and version-five
thirteen-byte profile remain unchanged.

The effects allocate no additional framebuffers, tasks, particle arrays or GIF
decoder. Their dirty region covers the canvas while active because drops,
strings and morphs leave the ordinary eye rectangle. Device frame time and
appearance still need physical checks on both panels.

## Reference correspondence

Names in the left column are reference filenames without `.gif`. Names on the
right are accepted firmware moods. Multiple references may share one mood.
Existing mood behavior is retained; these are not 63 newly recreated sequences.

| Reference names | Firmware mood |
| --- | --- |
| adore, love | `love` |
| angry, fierce | `angry` |
| blank | `neutral` |
| blinding | `blinding` |
| brave | `proud` |
| buzzing | `buzzing` |
| contempt | `judge` |
| crying, sobbing, weeping | `crying` |
| dancing | `dancing` |
| devil, evil | `devil` |
| distracted | `distracted` |
| dizzy | `swirly` |
| down | `look_down` |
| drowsy | `groggy` |
| encouragement, hello | `hello` |
| energetic | `energetic` |
| enraged, furious | `furious` |
| fast, rush, speed | `gauge` |
| giggle, laughing | `glee` |
| glowing | `glow` |
| growing | `growing` |
| handsome | `smug` |
| happy | `squeezed` |
| irritated | `annoyed` |
| left | `look_left` |
| menacing | `menacing` |
| mistake | `glitch` |
| playful | `pong` |
| police | `police` |
| rain | `rain` |
| relaxed, serene | `zen` |
| right | `look_right` |
| scared | `scared` |
| shrink | `shrink` |
| shy | `shy` |
| sick | `sick` |
| sleepy | `sleepy` |
| smile | `mild_happy` |
| smirk | `smirk` |
| smoke | `smoke` |
| sneeze | `sneeze` |
| sparkle | `sparkle` |
| splash | `splash` |
| spraying | `spraying` |
| squint | `squint` |
| surprised | `surprised` |
| sushi | `sushi` |
| swinging | `swinging` |
| teasing | `cheeky` |
| tough | `focused` |
| wink | `wink` |
| yawn | `doze` |
