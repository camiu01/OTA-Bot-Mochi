/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * @file eyes_types.h
 * @brief Shared types for bloub/Mochi robot eyes.
 *
 * @purpose Define EyeStyle, Expression, EyeConfig and EyeState singletons.
 */

#ifndef EYES_TYPES_H
#define EYES_TYPES_H

#include <stdint.h>

/**
 * @brief Rendering style of the eyes.
 */
enum EyeStyle : uint8_t {
	NORMAL = 0,	/**< Flat cartoon style with small pupils. */
	MOCHI = 1,	/**< Tall solid Dasai-like capsules, without pupils or highlights. */
	BLOUB = 2,	/**< Wide soft blobs with pupils and highlights. */
	TRAPEZ = 3,	/**< Solid mirrored trapezoids, without pupils or highlights. */
	ANIME = 4,	/**< Tall ovals with large reflective irises. */
	KITSUNE = 5,	/**< Normal-size ovals with feline slit pupils. */
	DRL = 6,	/**< Filled angular headlights with a white rim, without pupils. */
	EYE_STYLE_COUNT = 7	/**< Number of persistent styles; existing IDs never change. */
};

/**
 * @brief Emotional expression shown by the eyes.
 */
enum Expression : uint8_t {
	NEUTRAL = 0,	/**< Relaxed default gaze. */
	HAPPY = 1,	/**< Lidded cheerful eyes. */
	SAD = 2,	/**< Drooping eyelids. */
	ANGRY = 3,	/**< Angled eyelids. */
	SURPRISED = 4,	/**< Enlarged eyes. */
	SLEEPY = 5,	/**< Narrowed eyes. */
	LOVE = 6,	/**< Heart-shaped pupils. */
	DIZZY = 7,	/**< Cross-shaped wobbling pupils. */
	GLEE = 8,	/**< Joyful wide eyes with double arcs. */
	WORRIED = 9,	/**< Raised inner lids, uneasy stare. */
	FOCUSED = 10,	/**< Low bars, intense stare. */
	ANNOYED = 11,	/**< Half lids, asymmetric flat look. */
	SKEPTIC = 12,	/**< One big eye, one small eye. */
	FRUSTRATED = 13,	/**< Angled asymmetric lids. */
	UNIMPRESSED = 14,	/**< Flat half-mast lids. */
	SUSPICIOUS = 15,	/**< One high eye, one low eye. */
	SQUINT = 16,	/**< Narrowed suspicious slits. */
	FURIOUS = 17,	/**< Thick angled lids, intense glare. */
	SCARED = 18,	/**< Small narrow tall eyes. */
	AWE = 19,	/**< Huge round wonder eyes. */
	SLEEPY_EYES = 20,	/**< Heavy asymmetric drooping lids. */
	SAD_DOWN = 21,	/**< Sad gaze looking downward. */
	SAD_UP = 22,	/**< Sad gaze looking upward. */
	EXCITED = 23,	/**< Wide eyes with raised lower lids. */
	TERRIFIED = 24,	/**< Very tall eyes under a thin upper lid. */
	DISGUSTED = 25,	/**< Angled upper lid over a raised lower lid. */
	SHY = 26,	/**< Small lowered eyes, downward gaze. */
	PROUD = 27,	/**< Smug half-lids, mildly asymmetric. */
	TIRED = 28,	/**< Heavy droop on both lids. */
	GRIN = 29,	/**< Wide eyes behind a deep lower lid. */
	PANICKED = 30,	/**< Tall eyes pinched by two thin lids. */
	GLUM = 31,	/**< Small eyes under a double sad slope. */
	SMUG = 32,	/**< One eye far more closed than the other. */
	DEADPAN = 33,	/**< Flat centered line, a permanent deadpan. */
	ZOMBIE = 34,	/**< One sagging eye, reduced overall height. */
	CONFUSED = 35,	/**< One wide eye beside one squinting eye. */
	CURIOUS = 36,	/**< One raised eye beside one lowered eye. */
	WINK = 37,	/**< One eye closed, the other open. */
	WONDERING = 38,	/**< One raised brow over an open gaze. */
	STAR = 39,	/**< Star-shaped pupils. */
	CRYING = 40,	/**< Sad lids above a falling tear. */
	SWIRLY = 41,	/**< Spiral pupils, a dizzy-drunk look. */
	CAT = 42,	/**< Vertical slit pupils. */
	HALO = 43,	/**< One external golden aureole above the eye pair, without wings. */
	DEVIL = 44,	/**< Two external red horns above mischievous eyes. */
	FIRE = 45,	/**< Continuously flickering orange/yellow flame eyes. */
	MOON = 46,	/**< Continuously cycling lunar-phase eyes. */
	SPARKLE = 47,	/**< Four-point sparkle pupils. */
	BOLT = 48,	/**< Lightning bolt pupils. */
	DIAMOND = 49,	/**< Rhombus pupils. */
	RING = 50,	/**< Hollow ring pupils. */
	TARGET = 51,	/**< Crosshair pupils. */
	MONEY = 52,	/**< Currency-mark pupils. */
	MILD_HAPPY = 53,	/**< Gentle lidded smile, softer than HAPPY. */
	SMIRK = 54,	/**< One raised corner over a flat lid. */
	POUT = 55,	/**< Puffed, low, and deliberately wide. */
	KISS = 56,	/**< Narrow centered pucker. */
	NERVOUS = 57,	/**< Small uneasy eyes with a raised inner lid. */
	SHOCKED = 58,	/**< Wider and taller than SURPRISED. */
	MELTING = 59,	/**< Heavy downward droop on one side. */
	LAZY = 60,	/**< Symmetric half-mast with a low lift. */
	DOZE = 61,	/**< Almost fully closed, a thin resting slit. */
	GROGGY = 62,	/**< One eye far more open than the other. */
	DAYDREAM = 63,	/**< Wide, soft, and unfocused. */
	GRUMPY = 64,	/**< Frowning slope under a half-lid. */
	SASSY = 65,	/**< One raised brow over a flat lid. */
	CHEEKY = 66,	/**< A wink paired with a wide grin. */
	FLIRT = 67,	/**< Soft half-lids with a light lower lift. */
	ZEN = 68,	/**< Calm, narrow, and deeply relaxed. */
	MERCIFUL = 69,	/**< A kind, uneasy sadness. */
	MANIC = 70,	/**< Very wide eyes with tight pupils. */
	SIDE_EYE = 71,	/**< Both pupils pressed to one side. */
	BEAMING = 72,	/**< Wide joyful arcs, brighter than GLEE. */
	MUSIC = 73,	/**< Note-mark pupils on a calm gaze. */
	LOW_BATT = 74,	/**< Battery mark under drooping lids. */
	POWER = 75,	/**< Standby symbol in half-closed eyes. */
	LOADING = 76,	/**< Hourglass pupils while waiting. */
	IDEA = 77,	/**< Exclamation pupils, a sudden spark. */
	HYPER = 78,	/**< Very wide eyes under a thin raised lid. */
	SQUISH = 79,	/**< Wide eyes pressed down into a squish. */
	DERP = 80,	/**< Mismatched lids and a wandering gaze. */
	GLITCH = 81,	/**< Displaced horizontal glitch bars. */
	TWITCH = 82,	/**< One eye squinting, one squeezed shut. */
	SNEEZE = 83,	/**< Both eyes scrunched just before the sneeze. */
	JUDGE = 84,	/**< Heavy half-lids over a raised lower lid. */
	QUESTION = 85,	/**< Question-mark pupils over an open gaze. */
	CALCULATING = 86,	/**< Concentrated asymmetric lids over plus and equals signs. */
	CYBORG = 87,	/**< One eye behind a visor slope, one plain. */
	ARC_HAPPY = 88,	/**< Curved smiling eye slits. */
	ARC_SAD = 89,	/**< Curved drooping eye slits. */
	SQUEEZED = 90,	/**< Mirrored tightly closed chevrons. */
	TILTED = 91,	/**< Playful opposing slants. */
	PONG = 92,	/**< Automatic miniature Pong court. */
	CLOCK_EYES = 93,	/**< Decorative animated clock dials, not local time. */
	GAUGE = 94,	/**< Decorative tachometer sweep; never real speed or RPM. */
	GLOW = 95,	/**< Smooth color-cycling bodies with soft halos. */
	HEADLIGHTS = 96,	/**< Pulsing light cones outside the eyes. */
	DANCING = 97,	/**< Colored equalizer morph. */
	POLICE = 98,	/**< Slow red/blue eye alternation, not a real warning lamp. */
	RAIN = 99,	/**< Falling blue rain around drooping eyes. */
	GROWING = 100,	/**< Expanding breathing eyes. */
	SHRINK = 101,	/**< Eyes contract and recover. */
	SWINGING = 102,	/**< A suspended pendulum eye pair. */
	BLINDING = 103,	/**< Amber squint with soft rays. */
	BUZZING = 104,	/**< Yellow vibrating eyes. */
	SICK = 105,	/**< Green queasy sway and bubbles. */
	SPLASH = 106,	/**< Blue wave and falling droplets. */
	SPRAYING = 107,	/**< Cyan outward droplet jets. */
	SUSHI = 108,	/**< Eyes merge into one salmon nigiri. */
	ENERGETIC = 109,	/**< Automotive turbochargers with rotating compressors. */
	SMOKE = 110,	/**< Teal eyes with drifting soft puffs. */
	MENACING = 111,	/**< Slow red orbital eye rings. */
	HELLO = 112,	/**< Bouncing greeting and wink, without text or mouth. */
	DISTRACTED = 113,	/**< Alternating wandering eyes. */
	LOOK_LEFT = 114,	/**< Deliberate left glance and return. */
	LOOK_RIGHT = 115,	/**< Deliberate right glance and return. */
	LOOK_DOWN = 116,	/**< Deliberate downward glance and return. */
	STARGAZING = 117,	/**< Eyes look up at a twinkling starry sky, then return. */
	MISTAKE = 118,	/**< Flattened eyes beneath glitching "OTA BOT!" text. */
	ENCOURAGEMENT = 119,	/**< Hopping eyes above a "GO OTA BOT!" banner. */
	ADORE = 120,	/**< Pink swelling eyes with rising hearts. */
	LAUGHING = 121,	/**< Squeezed bouncing eyes with tear droplets. */
	ENRAGED = 122,	/**< Narrow shaking red eyes with an anger burst. */
	TRAFFIC = 123,	/**< Decorative red/amber/green lamp eyes. */
	NEON = 124,	/**< Breathing hollow eye outlines. */
	WHEELS = 125,	/**< Counter-rotating five-spoke wheels. */
	SAKURA = 126,	/**< Pink eyes beneath falling petals. */
	SAMURAI = 127,	/**< Asymmetric inclined eyes above a diagonal steel blade. */
	MECHA = 128,	/**< Full helmet assembles over the original eyes and retracts. */
	BAMBOO = 129,	/**< Pink eyes in a swaying bamboo grove. */
	TITAN = 130,	/**< Expanding amber eyes with red energy ribs. */
	UPSIDE_DOWN = 131,	/**< Reversible orbital half-turn. */
	UWU = 132,	/**< Tall U-shaped eyes and tilted cheeks, without a mouth. */
	YOKAI = 133,	/**< Ember eyes with horns and drifting wisps. */
	TIC_TAC_TOE = 134,	/**< Decorative miniature tic-tac-toe match. */
	MECHA_MASK = 135,	/**< Moving lower mask below the active original eyes. */
	MECHA_BREAK = 136,	/**< Mecha armor breaks away in separate pieces. */
	HOLLOW_ANGER = 137,	/**< Hollow angry eyes expand toward the viewer. */
	SNICKERING = 138,	/**< Uneven closed eyes bounce beneath comic streaks. */
	HICCUP = 139,	/**< Squash, upward jolt and recovery with bubbles. */
	RETRO_TV = 140,	/**< Eyes settle inside an original miniature CRT. */
	MECHA_PILOT = 141,	/**< Rounded pilot helmet with a green eye display. */
	COMET = 142,	/**< Lowered eyes beneath shooting stars. */
	YAKURA = 143,	/**< Round ember eyes beneath swept pale smoke brows. */
	CHINESE_DRAGON = 144,	/**< Red Chinese dragon with gold horns, whiskers and a coiling body. */
	ANGEL = 145,	/**< Gentle eyes beneath an aureole, with two external feathered wings. */
	HACKER = 146,	/**< Scrolling eye terminals and access-granted scan. */
	BERSERK = 147,	/**< Trembling narrowed eyes with a jagged energy discharge. */
	PORTAL = 148,	/**< Twin apertures swallow and release the eyes. */
	ORIGAMI = 149,	/**< Eyes fold into creased paper kites and unfold. */
	GHOST = 150,	/**< Spectral eyes fade, drift and rematerialize. */
	ECLIPSE = 151,	/**< Moving lunar discs leave two solar coronas. */
	DETECTIVE = 152,	/**< A magnifying glass searches and locks onto a detail. */
	PIXEL_CRASH = 153,	/**< Individual eye tiles scatter and rebuild. */
	POWER_UP = 154,	/**< Determined eyes charge a rising golden aura. */
	NINJA = 155,	/**< Headband, narrowed eyes and dash afterimages. */
	ALCHEMIST = 156,	/**< Rotating transmutation circles around cyan eyes. */
	MAGICAL_GIRL = 157,	/**< Pink transformation ribbons and a tiara. */
	MECHA_LOCK = 158,	/**< Tracking brackets converge around scanning amber eyes. */
	SPIRIT_SEAL = 159,	/**< Floating paper charms above violet eyes. */
	SWEAT_DROP = 160,	/**< Awkward squint with an oversized temple droplet. */
	ANIME_VEIN = 161,	/**< Pulsing four-pronged anger mark over tense eyes. */
	CHIBI_PANIC = 162,	/**< Small trembling eyes under a blue gloom comb. */
	TSUNDERE = 163,	/**< Stubborn sidelong squint softens into a blushing wink. */
	MANGA_SHOCK = 164,	/**< Tall monochrome eyes inside radial impact lines. */
	CHIBI_JOY = 165,	/**< Bouncing smiling chevrons with pastel confetti. */
	RUBBER_BAND = 166,	/**< Elastic eyes pull apart and rebound. */
	PEEKABOO = 167,	/**< Tiny roller blinds lower, then lift for a peek. */
	POPCORN = 168,	/**< Buttery kernels pop above squashed eyes. */
	SPRING_EYES = 169,	/**< Eyes hop on visible accordion springs. */
	COMIC_GLASSES = 170,	/**< Oversized purple glasses slip and wobble. */
	BALLOON_EYES = 171,	/**< Pastel eye balloons inflate above curved strings. */
	WIPERS = 172,	/**< Wiper arms sweep rain while the eyes follow. */
	TURN_SIGNAL = 173,	/**< Sideways glance beside sequential amber arrows. */
	FUEL_LOW = 174,	/**< Drooping eyes, blinking fuel pump and draining gauge. */
	DRIFT = 175,	/**< Counter-steering eyes over tyre smoke and skid marks. */
	CAR_WASH = 176,	/**< Striped roller brush crosses squeezed eyes among foam. */
	PARKING_SENSOR = 177,	/**< Distance bars close in to flashing red as eyes tense. */
	TUNNEL = 178,	/**< Relaxed eyes warmed by passing amber tunnel lamps. */
	GEAR_SHIFT = 179,	/**< Knob works a five-speed gate while the eyes nod. */
	CHECK_ENGINE = 180,	/**< Worried eyes glance up at a blinking engine lamp. */
	YANDERE = 181,	/**< Sweet squint snaps wide open under violet shadow lines. */
	NAP_BUBBLE = 182,	/**< Closed nodding eyes with a swelling anime snot bubble. */
	GLOOM = 183,	/**< Drooping eyes under a drifting personal raincloud. */
	DOT_EYES = 184,	/**< Speechless dot eyes while an ellipsis types out. */
	STEAM_BLUSH = 185,	/**< Squeezed flustered eyes, full blush and rising steam. */
	SNOWFALL = 186,	/**< Calm eyes under drifting crossed snowflakes. */
	HEATWAVE = 187,	/**< Wilting eyes under a rotating sun and heat shimmer. */
	RAINBOW = 188,	/**< Smiling eyes while a five-band rainbow draws itself. */
	THUNDERSTORM = 189,	/**< Eyes flinch at a lightning bolt under a drifting cloud. */
	FOGGY = 190,	/**< Squinting eyes peer through drifting fog bands. */
	WINDY = 191,	/**< Eyes squint into gusts while a leaf tumbles past. */
	HAIL = 192,	/**< Eyes wince under a red umbrella as hail pellets fall. */
	SUNRISE = 193,	/**< Sleepy eyes open as the sun rises over the horizon. */
	TORNADO = 194,	/**< Wobbling eyes are tugged toward a swaying funnel. */
	FROSTY = 195,	/**< Shivering icy eyes grow hanging icicles. */
	RAMEN = 196,	/**< Happy eyes over a steaming bowl and lifted noodles. */
	ICE_CREAM = 197,	/**< Eyes follow a melting strawberry cone. */
	COFFEE = 198,	/**< Drowsy eyes snap awake after a steaming mug. */
	PIZZA = 199,	/**< Hopping eyes over a slice with stretching cheese. */
	BUBBLE_TEA = 200,	/**< Sipping eyes over a cup of bouncing tapioca pearls. */
	DONUT = 201,	/**< Eyes track a sprinkled donut gliding back and forth. */
	ONIGIRI = 202,	/**< Bouncing happy squint beside a hopping rice ball. */
	COOKIE = 203,	/**< Munching eyes as bites vanish from a chocolate-chip cookie. */
	BUNNY = 204,	/**< Twitching rabbit ears rise above soft eyes. */
	FROG = 205,	/**< Bobbing eyes on green mounds with a flicking tongue. */
	OWL = 206,	/**< Feathered rings, tufts and a beak; one eye slow-blinks. */
	PENGUIN = 207,	/**< Waddling eyes with flapping flippers and orange feet. */
	BEE = 208,	/**< Eyes track a striped bee flying a figure eight. */
	FISH_BOWL = 209,	/**< Eyes inside a glass bowl with a swimming goldfish. */
	PUPPY = 210,	/**< Wide eyes with floppy ears and a panting tongue. */
	BUTTERFLY = 211,	/**< A butterfly flutters in and lands on one eye. */
	SHOUJO_STARS = 212,	/**< Enlarged dreamy eyes among twinkling shoujo stars. */
	CHIBI_CRY = 213,	/**< Squeezed eyes spray fountain arcs of tears. */
	DETERMINED = 214,	/**< Narrowed eyes with set brows over streaming speed lines. */
	FAINT = 215,	/**< Eyes droop shut as a little soul floats away. */
	KAWAII_WINK = 216,	/**< A blushing wink launches a bouncing star. */
	SNEAKY = 217,	/**< Low narrow eyes creep sideways leaving tiptoe prints. */
	HYPNOTIZED = 218,	/**< Eyes sway with a pendulum inside spreading rings. */
	BRAIN_FREEZE = 219,	/**< Icy squeezed eyes shudder under pulsing ice crystals. */
	DJ = 220,	/**< Eyes nod to the beat under headphones and equalizer bars. */
	KARAOKE = 221,	/**< Closed singing eyes sway behind a microphone and notes. */
	DISCO = 222,	/**< Bouncing eyes under a mirror ball and roaming lights. */
	DRUM_ROLL = 223,	/**< Jittering eyes above a snare and alternating sticks. */
	PARTY_POPPER = 224,	/**< Wide eyes as a popper sprays curling streamers. */
	BIRTHDAY = 225,	/**< Smiling eyes over a cake with three flickering candles. */
	FIREWORKS = 226,	/**< Upturned eyes watch staggered radial firework bursts. */
	MARACAS = 227,	/**< Eyes bounce alternately as two maracas shake. */
	WIFI_SEARCH = 228,	/**< Searching eyes under Wi-Fi arcs that fill in turn. */
	DOWNLOAD = 229,	/**< Eyes watch a progress bar fill, then smile. */
	PAIRING = 230,	/**< Eyes take turns dimming while dots link two devices. */
	CURSOR = 231,	/**< A pointer glides in and clicks one eye. */
	ERROR_SIGN = 232,	/**< Shaky wide eyes under a pulsing warning triangle. */
	SYNC_SPIN = 233,	/**< Eyes circle with two chasing sync arrows. */
	CHARGING = 234,	/**< Eyes open wider as a battery fills segment by segment. */
	TYPING = 235,	/**< Lowered eyes watch keys light up on a keyboard. */
	SOCCER = 236,	/**< Eyes track a patched ball bouncing across the bottom. */
	BASKETBALL = 237,	/**< Eyes follow a ball arcing into a netted hoop. */
	BOWLING = 238,	/**< Eyes follow a ball rolling into three toppling pins. */
	DICE_ROLL = 239,	/**< Bouncing eyes watch a tumbling die change faces. */
	FISHING = 240,	/**< Staring eyes widen when the bobber dips on a bite. */
	YOYO = 241,	/**< Eyes ride up and down with a spinning yo-yo. */
	JUGGLING = 242,	/**< Eyes sway under three juggled balls in a cascade. */
	TENNIS = 243,	/**< Eyes snap left and right after a ball over a net. */
	UFO = 244,	/**< A hovering saucer's beam lifts the eyes. */
	ROCKET = 245,	/**< Eyes look up as a rocket launches on its flame. */
	PLANET_RING = 246,	/**< A ringed planet orbits in front of and behind the eyes. */
	BLACK_HOLE = 247,	/**< Stretched eyes are pulled toward a spinning accretion swirl. */
	CRYSTAL_BALL = 248,	/**< Violet-lit eyes gaze into a swirling crystal ball. */
	WIZARD = 249,	/**< Eyes under a starry pointed hat beside a sparkling wand. */
	DRAGON_EGG = 250,	/**< Curious eyes watch a speckled egg wobble and crack. */
	VAMPIRE = 251,	/**< Red-tinted narrowed eyes, high collar and a circling bat. */
	PUMPKIN = 252,	/**< Candle-lit eyes inside a ribbed pumpkin outline. */
	ASTRONAUT = 253	/**< Floating eyes inside a helmet ring among drifting stars. */
};

/** @brief Number of defined expressions, bounding menus, parsers, and lookups. */
constexpr uint8_t EXPRESSION_COUNT = uint8_t(ASTRONAUT) + 1;
static_assert(EXPRESSION_COUNT == 254,
	"Expression enum, names, i18n labels and documentation must be extended together");

/**
 * @brief Canonical lowercase name of an expression, shared by USB, menus, and parsing.
 *
 * Single source of truth: the index of each entry equals its Expression value, so a
 * mismatch between the table and the enum cannot go unnoticed.
 *
 * @param expr Expression value.
 * @return Stable ASCII name, or "unknown" when out of range.
 */
inline const char *eyes_expression_name(Expression expr) {
	static const char *const names[EXPRESSION_COUNT] = {
		"neutral", "happy", "sad", "angry", "surprised", "sleepy", "love", "dizzy",
		"glee", "worried", "focused", "annoyed", "skeptic", "frustrated", "unimpressed",
		"suspicious", "squint", "furious", "scared", "awe", "sleepy_eyes", "sad_down", "sad_up",
		"excited", "terrified", "disgusted", "shy", "proud", "tired", "grin", "panicked",
		"glum", "smug", "deadpan", "zombie", "confused", "curious", "wink", "wondering",
		"star", "crying", "swirly", "cat", "halo", "devil", "fire", "moon", "sparkle",
		"bolt", "diamond", "ring", "target", "money", "mild_happy", "smirk", "pout",
		"kiss", "nervous", "shocked", "melting", "lazy", "doze", "groggy", "daydream",
		"grumpy", "sassy", "cheeky", "flirt", "zen", "merciful", "manic", "side_eye",
		"beaming", "music", "low_batt", "power", "loading", "idea", "hyper", "squish",
		"derp", "glitch", "twitch", "sneeze", "judge", "question", "calculating",
		"cyborg", "arc_happy", "arc_sad", "squeezed", "tilted", "pong", "clock_eyes",
		"gauge", "glow", "headlights", "dancing", "police", "rain", "growing",
		"shrink", "swinging", "blinding", "buzzing", "sick", "splash", "spraying",
		"sushi", "energetic", "smoke", "menacing", "hello", "distracted",
		"look_left", "look_right", "look_down", "stargazing",
		"mistake", "encouragement", "adore", "laughing", "enraged",
		"traffic", "neon", "wheels", "sakura", "samurai", "mecha",
		"bamboo", "titan", "upside_down", "uwu", "yokai", "tic_tac_toe", "mecha_mask", "mecha_break",
		"hollow_anger", "snickering", "hiccup", "retro_tv", "mecha_pilot", "comet", "yakura", "chinese_dragon", "angel",
		"hacker", "berserk", "portal", "origami", "ghost", "eclipse", "detective", "pixel_crash",
		"power_up", "ninja", "alchemist", "magical_girl", "mecha_lock", "spirit_seal",
		"sweat_drop", "anime_vein", "chibi_panic", "tsundere", "manga_shock", "chibi_joy",
		"rubber_band", "peekaboo", "popcorn", "spring_eyes", "comic_glasses", "balloon_eyes",
		"wipers", "turn_signal", "fuel_low", "drift", "car_wash", "parking_sensor", "tunnel",
		"gear_shift", "check_engine", "yandere", "nap_bubble", "gloom", "dot_eyes", "steam_blush",
		"snowfall", "heatwave", "rainbow", "thunderstorm", "foggy", "windy", "hail", "sunrise",
		"tornado", "frosty", "ramen", "ice_cream", "coffee", "pizza", "bubble_tea", "donut",
		"onigiri", "cookie", "bunny", "frog", "owl", "penguin", "bee", "fish_bowl", "puppy",
		"butterfly", "shoujo_stars", "chibi_cry", "determined", "faint", "kawaii_wink", "sneaky",
		"hypnotized", "brain_freeze", "dj", "karaoke", "disco", "drum_roll", "party_popper",
		"birthday", "fireworks", "maracas", "wifi_search", "download", "pairing", "cursor",
		"error_sign", "sync_spin", "charging", "typing", "soccer", "basketball", "bowling",
		"dice_roll", "fishing", "yoyo", "juggling", "tennis", "ufo", "rocket", "planet_ring",
		"black_hole", "crystal_ball", "wizard", "dragon_egg", "vampire", "pumpkin", "astronaut"
	};
	static_assert(sizeof(names) / sizeof(names[0]) == EXPRESSION_COUNT,
		"eyes_expression_name() entries must match Expression values one for one");
	return uint8_t(expr) < EXPRESSION_COUNT ? names[uint8_t(expr)] : "unknown";
}

/**
 * @brief Persistent configuration singleton.
 */
struct EyeConfig {
	uint32_t variationSeed = 0; /**< Render-only event seed; zero keeps the canonical drawing, never persisted. */
	EyeStyle style;	/**< Active rendering style. */
	Expression expression;	/**< Active emotional expression. */
	uint16_t bgColor;	/**< Background clear color (RGB565). */
	uint16_t fgColor;	/**< Eye body color (RGB565). */
	uint16_t pupilColor;	/**< Pupil/overlay color (RGB565). */
	uint16_t highlightColor;	/**< Specular highlight color (RGB565). */
	int16_t eyeW;	/**< Single eye width in pixels. */
	int16_t eyeH;	/**< Single eye height in pixels. */
	int16_t eyeGap;	/**< Horizontal gap between eyes in pixels. */
	int16_t radius;	/**< Corner radius used by fillRoundRect. */
	int8_t slant;	/**< Mirrored TRAPEZ top slope, clamped within the eye body. */
	uint8_t asym;	/**< 0 symmetric, 1 per-eye geometry may vary. */
	int16_t lidTop;	/**< Eyelid line offset in px, negative raises lid. */
	uint16_t screenW;	/**< Canvas width in pixels, read from the display. */
	uint16_t screenH;	/**< Canvas height in pixels, read from the display. */
};

/** @brief Transient scenery timing, independent of the eye expression clock. */
struct EyeBackdropState {
	EyeConfig config{};
	uint32_t since = 0;
	uint32_t frozenElapsed = 0;
	bool initialized = false;
	bool leaving = false;
};

/**
 * @brief Mutable animation state singleton.
 */
struct EyeState {
	EyeBackdropState backdrop; /**< Outgoing scenery, never persisted. */
	float backdropOpacity = 1; /**< Frame-local backdrop fade; never applied to eye ink. */
	Expression effectExpression = NEUTRAL; /**< Last rendered mood for effect timing. */
	uint32_t effectStartMs = 0; /**< Effect entry time, never persisted. */
	bool effectClockInit = false; /**< First render initializes the effect clock. */
	int16_t saccadeX;	/**< Random gaze offset on X axis. */
	int16_t saccadeY;	/**< Random gaze offset on Y axis. */
	int16_t tiltX;	/**< IMU-driven pupil offset on X axis. */
	int16_t tiltY;	/**< IMU-driven pupil offset on Y axis. */
	int16_t floatY;	/**< Sinusoidal idle bobbing offset. */
	float floatPhase;	/**< Phase accumulator for idle bobbing. */
	uint32_t lastMs;	/**< Timestamp of last update call. */
	uint32_t nextSaccadeMs;	/**< Scheduled time for next saccade. */
	uint32_t nextBlinkMs;	/**< Scheduled time for next blink. */
	uint32_t blinkStartMs;	/**< Timestamp when current blink started. */
	uint32_t blinkDurMs;	/**< Duration of current blink. */
	uint32_t dizzyUntilMs;	/**< Expiry time of DIZZY override, 0 when inactive. */
	uint32_t glanceUntilMs;	/**< Expiry of the tap-directed gaze hold, 0 when idle. */
	bool tracking = false; /**< A live finger owns gaze, never persisted. */
	uint32_t petUntilMs = 0; /**< Last recognized stroke's visual relaxation, RAM-only. */
	uint32_t wakeUntilMs = 0; /**< Brief pickup wake gesture, RAM-only. */
	uint32_t heartUntilMs;	/**< Expiry of the petting heart marker, 0 when idle. */
	int16_t heartX;		/**< Center X of the petting heart marker. */
	int16_t heartY;		/**< Top Y of the petting heart marker. */
	bool blinking;	/**< True while eyelids are closed. */
	Expression savedExpr;	/**< Expression to restore after DIZZY. */
	int16_t prevX;	/**< X of dirty region union from last frame. */
	int16_t prevY;	/**< Y of dirty region union from last frame. */
	int16_t prevW;	/**< Width of dirty region union from last frame. */
	int16_t prevH;	/**< Height of dirty region union from last frame. */
	bool dirtyInit;	/**< False until first full-screen clear is done. */
};

#endif
