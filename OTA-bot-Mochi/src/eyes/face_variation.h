/**
 * @file face_variation.h
 * @brief Bounded, monotonic rhythm variation without changing complete loop lengths.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "face_policy.h"
#include "face_presets.h"
#include "face_random.h"
#include "eye_geometry.h"

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

/** @brief Give self-contained scenes gentle seeded breathing and drift, with exact entry/rest windows. */
inline void face_variation_layout(const EyeConfig &cfg, EyeLayout &layout, uint32_t elapsed, float amount,
		bool tracking = false) {
	const auto *preset = face_preset(cfg.expression);
	if (!cfg.variationSeed || amount <= 0 || !preset || !preset->fullScene || cfg.expression == DIZZY) { return; }
	uint32_t duration = preset->normalCycle ? 6000 : FacePolicy::cycle_duration(cfg.expression);
	uint32_t phase = elapsed % duration;
	if (phase <= 800 || phase >= duration - 800) { return; }
	float t = (phase - 800) / float(duration - 1600);
	float envelope = 16 * t * t * (1 - t) * (1 - t) * (amount > 1.2f ? 1.2f : amount);
	uint32_t salt = uint32_t(cfg.expression) * 1597u + elapsed / duration * 977u;
	auto sample = [&](uint32_t key) {
		return (face_smooth_random(phase, 1300, salt + key, cfg.variationSeed) - 0.5f) * envelope;
	};
	float sx = 1 + sample(173) * 0.04f, sy = 1 + sample(511) * 0.04f;
	int16_t dx = tracking ? 0 : int16_t(lroundf(sample(733) * cfg.screenW / 60));
	int16_t dy = tracking ? 0 : int16_t(lroundf(sample(947) * cfg.screenH / 70));
	for (auto *box : {&layout.left, &layout.right}) {
		int16_t w = eye_limit(int16_t(lroundf(box->w * sx)), 1, cfg.screenW);
		int16_t h = eye_limit(int16_t(lroundf(box->h * sy)), 1, cfg.screenH);
		box->x = eye_limit(box->x + (box->w - w) / 2 + dx, 0, cfg.screenW - w);
		box->y = eye_limit(box->y + (box->h - h) / 2 + dy, 0, cfg.screenH - h);
		box->w = w; box->h = h;
		box->radius = eye_limit(box->radius, 0, (w < h ? w : h) / 2);
	}
}
