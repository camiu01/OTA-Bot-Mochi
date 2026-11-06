/**
 * @file face_variation.h
 * @brief Bounded, monotonic rhythm variation without changing complete loop lengths.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "face_policy.h"
#include "face_presets.h"
#include "face_random.h"

/** @brief Vary the active beat, keeping the first/last 800 ms and cycle boundaries exact. */
inline uint32_t face_variation_elapsed(const EyeConfig &cfg, uint32_t elapsed, float amount) {
	if (!cfg.variationSeed || amount <= 0) { return elapsed; }
	const FacePreset *preset = face_preset(cfg.expression);
	uint32_t duration = preset && preset->normalCycle ? 6000 : FacePolicy::cycle_duration(cfg.expression);
	uint32_t phase = elapsed % duration;
	if (phase <= 800 || phase >= duration - 800) { return elapsed; }
	float t = (phase - 800) / float(duration - 1600);
	float bump = 16 * t * t * (1 - t) * (1 - t);
	float strength = (face_random(elapsed / duration, 823, cfg.variationSeed) - 0.5f) * 0.16f;
	float intensity = amount > 1.2f ? 1.2f : amount;
	int64_t varied = int64_t(elapsed) + int32_t(strength * intensity * (duration - 1600) * bump);
	return varied > UINT32_MAX ? UINT32_MAX : uint32_t(varied);
}
