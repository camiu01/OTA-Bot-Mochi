/**
 * @file face_policy.h
 * @brief Non-blocking temporary moods, tap/pet reactions, and sustained IMU qualification.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"
#include "face_flow.h"
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
	uint8_t chain_count = 0, sequence_index = 0;

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
		chain_count = 0;
	}

	/**
	 * @brief Map a dense index to every visible expression except the neutral default and dizzy.
	 * @param index Index within EXPRESSION_COUNT - 2.
	 * @return Selected expression.
	 */
	static Expression automatic_at(uint8_t index) {
		return Expression(index < uint8_t(DIZZY) - 1 ? index + 1 : index + 2);
	}

	/**
	 * @brief Start a bounded automatic reaction covering the whole expression set.
	 * @param now Time.
	 * @param choice Random input.
	 * @param mode Playback mode.
	 */
	void automatic_face(uint32_t now, uint32_t choice, uint8_t mode) {
		static constexpr uint8_t pool_size = uint8_t(EXPRESSION_COUNT - 2);
		uint8_t index = mode == FACE_SEQUENCE ? sequence_index : uint8_t(choice % pool_size);
		Expression value = automatic_at(index);
		while (value == base || value == previous || value == face) {
			index = uint8_t((index + 1) % pool_size);
			value = automatic_at(index);
		}
		face = previous = value; started = last_random = now;
		sequence_index = uint8_t((index + 1) % pool_size);
		if (chain_count < 5) { ++chain_count; }
		duration = cycle_duration(value); random_delay = 12000 + choice % 18001;
		active = true; motion = false; manual = false; qualifying = false;
		affection = false;
	}

	/** @brief Start a bounded random reaction from the curated tap pool. @param now Time. @param choice Random input. */
	void random_reaction(uint32_t now, uint32_t choice) {
		static const Expression pool[] = {
			HAPPY, GLEE, FOCUSED, SKEPTIC, LOVE, AWE, SLEEPY_EYES,
			EXCITED, GRIN, CURIOUS, WONDERING, STAR, SMUG, CONFUSED, SHY,
			SPARKLE, BEAMING, MILD_HAPPY, SMIRK, DAYDREAM, CHEEKY, ZEN,
			MUSIC, IDEA, SQUISH, DERP, HYPER
		};
		react(now, choice, pool, uint8_t(sizeof(pool) / sizeof(pool[0])), 3000);
	}

public:
	/** @brief Complete visible loop length. @param value Mood. @return Milliseconds. */
	static uint32_t cycle_duration(Expression value) {
		if (value >= DANCING && value <= ENRAGED) { return 6000; }
		if (value == GAUGE || value == PONG) { return 12000; }
		if (value == GLOW) { return 9600; }
		return value == HEADLIGHTS ? 3600 : 3000;
	}
	/** @brief Distinguish an automatic reaction from base/manual restoration. @return Automatic reaction. */
	bool automatic() const { return active && !manual; }
	/** @brief Trigger a temporary face even with automation off. @param now Time. @param choice Random input. */
	void tap(uint32_t now, uint32_t choice) { random_reaction(now, choice); }
	/** @brief Trigger a playful double-tap face even with automation off. @param now Time. @param choice Random input. */
	void play(uint32_t now, uint32_t choice) {
		static const Expression pool[] = {
			AWE, EXCITED, STAR, SPARKLE, GLEE, BEAMING, MANIC,
			HYPER, SQUISH, DERP
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
		if (active && (manual || (!motion && duration > 3000)) &&
			uint32_t(now - started) < duration) { qualifying = false; return; }
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
	 * @param ready Selected mood has finished its visible cycle; does not cancel an active reaction.
	 * @param mode Return, group of five, ordered sequence, or continuous random playback.
	 */
	Expression update(uint32_t now, bool random_enabled, bool imu_enabled, uint32_t choice,
			bool ready = true, uint8_t mode = FACE_RETURN) {
		bool expired = active && uint32_t(now - started) >= duration;
		if (expired && !manual && !motion && random_enabled && mode != FACE_RETURN &&
			(mode != FACE_GROUP5 || chain_count < 5)) {
			automatic_face(now, choice, mode);
			return face;
		}
		if (active && (uint32_t(now - started) >= duration || (motion ? !imu_enabled : (!random_enabled && !manual)))) {
			active = false;
			chain_count = 0;
			if (mode == FACE_GROUP5) { last_random = now; }
		}
		if (!random_enabled) { last_random = now; }
		if (ready && !active && random_enabled && uint32_t(now - last_random) >= random_delay) {
			automatic_face(now, choice, mode);
		}
		return active ? face : base;
	}
};
