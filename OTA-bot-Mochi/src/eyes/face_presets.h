/**
 * @file face_presets.h
 * @brief Per-eye face definitions and render hooks; public IDs remain stable.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"

class Arduino_GFX;
struct EyeBox;
struct EyeLayout;

/** @brief Additional geometry enabled only by cfg.asym outside a blink. */
struct FaceAsymmetry {
	float widthScale = 1.0f, heightScale = 1.0f;
	int16_t widthDelta = 0, heightDelta = 0, offsetY = 0, height = 0;
};

/** @brief Style-relative dimensions, with exact reference-unit adjustments. */
struct FaceEye {
	float widthScale = 1.0f, heightScale = 1.0f, openness = 1.0f;
	int16_t widthDelta = 0, heightDelta = 0, offsetY = 0, height = 0;
	bool heightFromWidth = false;
	FaceAsymmetry asym;
};

/** @brief Optional procedural motion shared by reaction/performance clocks. */
using FaceMotion = void (*)(bool, uint32_t, float, float &, float &, float &, float &);
using FaceLids = void (*)(Arduino_GFX *, const EyeConfig &, const EyeBox &);
using FaceBody = bool (*)(Arduino_GFX *, const EyeConfig &, const EyeState &, const EyeBox &, uint32_t, uint16_t);
using FaceGlyph = bool (*)(Arduino_GFX *, const EyeConfig &, const EyeBox &, int16_t, int16_t, int16_t, uint16_t, uint16_t);
using FaceScene = bool (*)(Arduino_GFX *, const EyeConfig &, const EyeLayout &, uint32_t, float);
using FaceScenery = void (*)(Arduino_GFX *, const EyeConfig &, const EyeLayout &, uint32_t, float);

/** @brief One face owns its definition and expression-specific procedures. */
struct FacePreset {
	const char *name = nullptr;
	FaceEye left, right;
	float gazeX = 0.0f, gazeY = 0.0f;
	FaceLids lids = nullptr;
	FaceGlyph glyph = nullptr;
	FaceMotion reactionMotion = nullptr, performanceMotion = nullptr;
	FaceBody reactionBody = nullptr, referenceBody = nullptr;
	FaceScene scene = nullptr, performanceScene = nullptr;
	FaceScenery scenery = nullptr;
	void (*background)(Arduino_GFX *, EyeConfig &, const EyeState &, const EyeLayout &) = nullptr;
	EyeLayout (*bounds)(const EyeConfig &, EyeLayout) = nullptr;
	void (*layout)(const EyeConfig &, EyeBox &, uint32_t, float) = nullptr;
	void (*drawing)(EyeState &, const EyeLayout &, uint32_t) = nullptr;
	uint16_t (*performanceColor)(const EyeConfig &, bool, uint32_t) = nullptr;
	uint16_t reactionInk = 0, performanceInk = 0;
	bool hasReactionInk = false, performance = false, fullScene = false;
	bool wholeEye = false, replacesSolid = false, cross = false, sideGaze = false;
};

/** @brief An explicit public ID and its immutable definition. */
struct FacePresetEntry { Expression id; FacePreset preset; };
extern const FacePresetEntry FACE_PRESETS[EXPRESSION_COUNT];

/** @brief Resolve one face without allocating or searching. */
inline const FacePreset *face_preset(Expression id) {
	return uint8_t(id) < EXPRESSION_COUNT ? &FACE_PRESETS[uint8_t(id)].preset : nullptr;
}

/** @brief Percent arithmetic preserves legacy integer division rather than float rounding. */
constexpr int16_t face_percent(float scale) { return int16_t(scale * 100.0f + 0.5f); }

/** @brief Validate finite supported multipliers, openness and reference geometry. */
constexpr bool face_eye_valid(const FaceEye &eye) {
	return eye.widthScale >= 0.2f && eye.widthScale <= 2.6f &&
		eye.heightScale >= 0.1f && eye.heightScale <= 2.6f &&
		eye.openness >= 0 && eye.openness <= 1 &&
		eye.widthDelta >= -32 && eye.widthDelta <= 32 &&
		eye.heightDelta >= -130 && eye.heightDelta <= 130 &&
		eye.offsetY >= -32 && eye.offsetY <= 32 &&
		eye.height >= 0 && eye.height <= 130 &&
		eye.asym.widthScale >= 0.2f && eye.asym.widthScale <= 2.6f &&
		eye.asym.heightScale >= 0.1f && eye.asym.heightScale <= 2.6f &&
		eye.asym.widthDelta >= -32 && eye.asym.widthDelta <= 32 &&
		eye.asym.heightDelta >= -130 && eye.asym.heightDelta <= 130 &&
		eye.asym.offsetY >= -32 && eye.asym.offsetY <= 32 &&
		eye.asym.height >= 0 && eye.asym.height <= 130;
}

/** @brief Validate a definition before adding it to the immutable catalog. */
constexpr bool face_preset_valid(const FacePreset &p) {
	return p.name && p.name[0] && face_eye_valid(p.left) && face_eye_valid(p.right) &&
		p.gazeX >= -1 && p.gazeX <= 1 && p.gazeY >= -1 && p.gazeY <= 1;
}

/** @brief Check every catalog slot, including its explicit unchanged ID. */
constexpr bool face_entries_valid(const FacePresetEntry *entries) {
	for (unsigned i = 0; i < EXPRESSION_COUNT; ++i) {
		if (uint8_t(entries[i].id) != i || !face_preset_valid(entries[i].preset)) { return false; }
	}
	return true;
}

/** @brief Validate all registered face IDs and parameter ranges. */
bool face_catalog_valid();
