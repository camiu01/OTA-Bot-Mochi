/**
 * @file face_variety.h
 * @brief Visual families and bounded recent history for continuous random playback.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"

/** @brief Group related silhouettes; ungrouped performances retain their own identity. */
inline Expression face_family(Expression face) {
	switch (face) {
		case HAPPY: case MILD_HAPPY: case GLEE: case GRIN: case BEAMING:
		case SQUEEZED: case LAUGHING: case ARC_HAPPY: return HAPPY;
		case ANGRY: case FURIOUS: case GRUMPY: case ANNOYED: case FRUSTRATED:
		case ENRAGED: return ANGRY;
		case SLEEPY: case SLEEPY_EYES: case TIRED: case LAZY: case DOZE:
		case GROGGY: case ZEN: case DEADPAN: return SLEEPY;
		case LOVE: case ADORE: case KISS: case FLIRT: case SHY: return LOVE;
		case SAD: case SAD_DOWN: case SAD_UP: case GLUM: case WORRIED:
		case CRYING: case ARC_SAD: return SAD;
		case SURPRISED: case SHOCKED: case AWE: case EXCITED: case PANICKED:
		case TERRIFIED: case SCARED: case HYPER: case MANIC: return SURPRISED;
		case SMUG: case SMIRK: case SASSY: case PROUD: case JUDGE:
		case UNIMPRESSED: return SMUG;
		case FOCUSED: case SQUINT: case SKEPTIC: case SUSPICIOUS:
		case SIDE_EYE: return FOCUSED;
		case CONFUSED: case CURIOUS: case WONDERING: case QUESTION:
		case CALCULATING: return CONFUSED;
		case LOOK_LEFT: case LOOK_RIGHT: case LOOK_DOWN: case DISTRACTED: return DISTRACTED;
		case STAR: case SPARKLE: return STAR;
		case MUSIC: case DANCING: return DANCING;
		case GLOW: case NEON: return GLOW;
		case DEVIL: case YOKAI: return DEVIL;
		default: return face;
	}
}

/** @brief Avoid the last eight IDs and the two most recent visual families. */
class FaceVariety {
	static constexpr uint8_t CAPACITY = 8;
	Expression recent[CAPACITY] = {};
	uint8_t count = 0;
public:
	/** @brief Test one candidate without consuming randomness or modifying history. */
	bool excludes(Expression value, Expression current, Expression base) const {
		Expression family = face_family(value);
		if (family == face_family(current) || family == face_family(base)) { return true; }
		for (uint8_t i = 0; i < count; ++i) {
			if (value == recent[i] || (i < 2 && family == face_family(recent[i]))) { return true; }
		}
		return false;
	}
	/** @brief Remember only an actual automatic selection, not a rendered frame. */
	void remember(Expression value) {
		if (count < CAPACITY) { ++count; }
		for (uint8_t i = count - 1; i > 0; --i) { recent[i] = recent[i - 1]; }
		recent[0] = value;
	}
};
