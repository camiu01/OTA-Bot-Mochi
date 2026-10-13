/**
 * @file motion_filter.h
 * @brief Mount-relative, low-jitter gaze and deliberate multi-reversal shake detection.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include <math.h>

struct MotionSample {
	int16_t x = 0, y = 0;
	bool shake = false;
	float delta_x = 0, delta_y = 0, delta_z = 0;
};

class MotionFilter {
	bool ready = false;
	bool strong = false;
	bool cooling = false;
	uint8_t reversals = 0;
	float baseline[3] = {};
	float previous[3] = {};
	float gaze_x = 0, gaze_y = 0;
	uint32_t last_strong = 0, last_shake = 0;

	/** @brief Bound gaze excursion. @param value Pixels. @return Clamped integer. */
	static int16_t bounded(float value) {
		if (value > 8) { return 8; }
		if (value < -8) { return -8; }
		return int16_t(value);
	}

	/**
	 * @brief Require three opposing impulses within 800ms and a five-second cooldown.
	 * @param delta Gravity-relative acceleration vector.
	 * @param now Timestamp.
	 * @param enabled Explicit shake opt-in.
	 * @return Deliberate shake event.
	 */
	bool detect(const float delta[3], uint32_t now, bool enabled) {
		if (!enabled) { strong = false; reversals = 0; return false; }
		if (cooling && uint32_t(now - last_shake) < 5000) { return false; }
		float energy = delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2];
		if (energy < 1.44f) { return false; }
		if (!strong || uint32_t(now - last_strong) > 800) {
			reversals = 0;
		} else {
			float dot = delta[0] * previous[0] + delta[1] * previous[1] + delta[2] * previous[2];
			if (dot < -0.5f) { ++reversals; }
		}
		for (uint8_t i = 0; i < 3; ++i) { previous[i] = delta[i]; }
		last_strong = now; strong = true;
		if (reversals < 3) { return false; }
		last_shake = now; cooling = true; strong = false; reversals = 0;
		return true;
	}

public:
	/**
	 * @brief Filter a nominal 50ms sample; the first sample is the parked mounting reference.
	 * @param x Acceleration in g.
	 * @param y Acceleration in g.
	 * @param z Acceleration in g.
	 * @param now Timestamp.
	 * @param shake_enabled Opt-in shake reactions.
	 * @return Damped gaze and deliberate shake event.
	 */
	MotionSample update(float x, float y, float z, uint32_t now, bool shake_enabled) {
		MotionSample sample;
		if (!isfinite(x) || !isfinite(y) || !isfinite(z)) { return sample; }
		float axes[] = {x, y, z}, delta[3];
		if (!ready) {
			for (uint8_t i = 0; i < 3; ++i) { baseline[i] = axes[i]; }
			ready = true;
		}
		float energy = 0;
		for (uint8_t i = 0; i < 3; ++i) {
			delta[i] = axes[i] - baseline[i];
			energy += delta[i] * delta[i];
		}
		if (energy < 0.0225f) {
			for (uint8_t i = 0; i < 3; ++i) { baseline[i] += delta[i] * 0.01f; }
		}
		float target_x = fabsf(delta[0]) < 0.035f ? 0 : delta[0] * 18;
		float target_y = fabsf(delta[1]) < 0.035f ? 0 : -delta[1] * 12;
		gaze_x += (target_x - gaze_x) * 0.12f;
		gaze_y += (target_y - gaze_y) * 0.12f;
		sample.x = bounded(gaze_x); sample.y = bounded(gaze_y);
		sample.delta_x = delta[0]; sample.delta_y = delta[1]; sample.delta_z = delta[2];
		sample.shake = detect(delta, now, shake_enabled);
		return sample;
	}
};
