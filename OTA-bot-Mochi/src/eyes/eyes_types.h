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
	TRAPEZ = 3	/**< Solid mirrored trapezoids, without pupils or highlights. */
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
	HALO = 43,	/**< A halo ring set into the upper eye. */
	DEVIL = 44,	/**< Paired horns in the upper eye. */
	FIRE = 45,	/**< Flame-shaped pupils. */
	MOON = 46,	/**< Crescent-shaped pupils. */
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
	CALCULATING = 86,	/**< Furrowed asymmetric lids over an hourglass. */
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
	ENRAGED = 122	/**< Narrow shaking red eyes with an anger burst. */
};

/** @brief Number of defined expressions, bounding menus, parsers, and lookups. */
constexpr uint8_t EXPRESSION_COUNT = uint8_t(ENRAGED) + 1;
static_assert(EXPRESSION_COUNT == 123,
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
		"mistake", "encouragement", "adore", "laughing", "enraged"
	};
	static_assert(sizeof(names) / sizeof(names[0]) == EXPRESSION_COUNT,
		"eyes_expression_name() entries must match Expression values one for one");
	return uint8_t(expr) < EXPRESSION_COUNT ? names[uint8_t(expr)] : "unknown";
}

/**
 * @brief Persistent configuration singleton.
 */
struct EyeConfig {
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

/**
 * @brief Mutable animation state singleton.
 */
struct EyeState {
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
