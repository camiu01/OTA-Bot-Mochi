/**
 * @file face_policy.h
 * @brief Non-blocking temporary moods, tap/pet reactions, and sustained IMU qualification.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"
#include <math.h>

class FacePolicy {
	Expression base = NEUTRAL, face = NEUTRAL, previous = NEUTRAL, candidate = NEUTRAL;
	bool active = false, motion = false, qualifying = false, cooling = false, manual = false;
	bool affection = false;
	uint32_t started = 0, duration = 0, last_random = 0, random_delay = 12000;
	uint32_t motion_since = 0, last_motion = 0;
	uint32_t last_sample = 0;
	bool sampled = false;
	int8_t direction = 0;

	/**
	 * @brief Start a bounded reaction from an explicit pool, avoiding the base and repeats.
	 * @param now Time.
	 * @param choice Random input.
	 * @param pool Candidate expressions.
	 * @param pool_size Pool length.
	 * @param dur Reaction duration in milliseconds.
	 */
	void react(uint32_t now, uint32_t choice, const Expression *pool, uint8_t pool_size, uint32_t dur) {
		uint8_t index = uint8_t(choice % pool_size);
		while (pool[index] == base || pool[index] == previous || pool[index] == face) {
			index = uint8_t((index + 1) % pool_size);
		}
		face = previous = pool[index]; started = last_random = now;
		duration = dur; random_delay = 12000 + choice % 18001;
		active = true; motion = false; manual = true; qualifying = false;
		affection = false;
	}

	/** @brief Start a bounded random reaction. @param now Time. @param choice Random input. @param tapped Explicit request. */
	void random_reaction(uint32_t now, uint32_t choice, bool tapped) {
		static const Expression pool[] = {
			HAPPY, GLEE, FOCUSED, SKEPTIC, LOVE, AWE, SLEEPY_EYES,
			EXCITED, GRIN, CURIOUS, WONDERING, STAR, SMUG, CONFUSED, SHY,
			SPARKLE, BEAMING, MILD_HAPPY, SMIRK, DAYDREAM, CHEEKY, ZEN
		};
		react(now, choice, pool, uint8_t(sizeof(pool) / sizeof(pool[0])), 3000);
		manual = tapped;
	}

public:
	/** @brief Distinguish an automatic reaction from base/manual restoration. @return Automatic reaction. */
	bool automatic() const { return active && !manual; }
	/** @brief Trigger a temporary face even with automation off. @param now Time. @param choice Random input. */
	void tap(uint32_t now, uint32_t choice) { random_reaction(now, choice, true); }
	/** @brief Trigger a playful double-tap face even with automation off. @param now Time. @param choice Random input. */
	void play(uint32_t now, uint32_t choice) {
		static const Expression pool[] = {
			AWE, EXCITED, STAR, SPARKLE, GLEE, BEAMING, MANIC
		};
		react(now, choice, pool, uint8_t(sizeof(pool) / sizeof(pool[0])), 3500);
	}
	/**
	 * @brief Trigger an affectionate face; continued stroking extends it up to six seconds.
	 * @param now Time.
	 * @param choice Random input.
	 */
	void pet(uint32_t now, uint32_t choice) {
		static const Expression pool[] = {
			HAPPY, LOVE, GLEE, BEAMING, MILD_HAPPY, EXCITED
		};
		if (active && affection) {
			uint32_t end = started + duration;
			uint32_t wanted = now + 3000;
			if (wanted > end) { end = wanted; }
			if (end > started + 6000) { end = started + 6000; }
			duration = end - started;
			last_random = now;
			return;
		}
		react(now, choice, pool, uint8_t(sizeof(pool) / sizeof(pool[0])), 3000);
		affection = true;
	}
	/** @brief Select a base mood and restart automatic timers. @param value Mood. @param now Timestamp. */
	void reset(Expression value, uint32_t now) {
		*this = FacePolicy{};
		base = face = previous = value;
		last_random = now;
	}

	/**
	 * @brief Qualify a sustained axis deflection, not a single road impulse.
	 * @param x Mount-relative g.
	 * @param y Mount-relative g.
	 * @param z Mount-relative g.
	 * @param now Timestamp.
	 * @param enabled IMU mood opt-in.
	 */
	void sample(float x, float y, float z, uint32_t now, bool enabled) {
		if (active && manual && uint32_t(now - started) < duration) { qualifying = false; return; }
		if (sampled && uint32_t(now - last_sample) > 150) { qualifying = false; }
		last_sample = now; sampled = true;
		if (!enabled || !isfinite(x) || !isfinite(y) || !isfinite(z)) { qualifying = false; return; }
		if (cooling && uint32_t(now - last_motion) < 5000) { qualifying = false; return; }
		Expression next = fabsf(z) >= 0.8f ? SURPRISED :
			(fabsf(y) >= 0.25f ? FOCUSED : (fabsf(x) >= 0.25f ? SKEPTIC : NEUTRAL));
		if (next == NEUTRAL) { qualifying = false; return; }
		float axis = next == SURPRISED ? z : (next == FOCUSED ? y : x);
		int8_t sign = axis >= 0 ? 1 : -1;
		if (!qualifying || next != candidate || sign != direction) {
			candidate = next; motion_since = now; qualifying = true;
			direction = sign;
		}
		uint32_t required = next == SURPRISED ? 100 : 350;
		if (uint32_t(now - motion_since) < required) { return; }
		face = next; started = now; duration = 2200; active = motion = true; manual = false;
		last_motion = now; cooling = true; qualifying = false;
		last_random = now;
	}

	/**
	 * @brief Return a temporary mood or the selected base, without changing style/color.
	 * @param now Timestamp.
	 * @param random_enabled Random mood opt-in.
	 * @param imu_enabled IMU mood opt-in.
	 * @param choice Random input.
	 * @return Active mood, with IMU taking priority over random.
	 */
	Expression update(uint32_t now, bool random_enabled, bool imu_enabled, uint32_t choice) {
		if (active && (uint32_t(now - started) >= duration || (motion ? !imu_enabled : (!random_enabled && !manual)))) {
			active = false;
		}
		if (!random_enabled) { last_random = now; }
		if (!active && random_enabled && uint32_t(now - last_random) >= random_delay) {
			random_reaction(now, choice, false);
		}
		return active ? face : base;
	}
};
