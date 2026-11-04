/**
 * @file face_random.h
 * @brief Stateless samples for repeatable particle paths and bounded expressive variation.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

/** @brief Hash an event and a stable independent salt into the closed unit interval. */
inline float face_random(uint32_t event, uint32_t salt, uint32_t seed = 0) {
	uint32_t value = event + salt + seed;
	value ^= value >> 16; value *= 0x7feb352du;
	value ^= value >> 15; value *= 0x846ca68bu;
	value ^= value >> 16;
	return (value & 65535u) / 65535.0f;
}

/** @brief Ease between event samples; callers supply a nonzero interval in milliseconds. */
inline float face_smooth_random(uint32_t elapsed, uint32_t interval, uint32_t salt, uint32_t seed = 0) {
	uint32_t event = elapsed / interval;
	float t = (elapsed % interval) / float(interval);
	t = t * t * (3 - 2 * t);
	return face_random(event, salt, seed) * (1 - t) + face_random(event + 1, salt, seed) * t;
}
