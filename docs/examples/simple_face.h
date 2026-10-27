/**
 * @file simple_face.h
 * @brief C++17 equivalent of a named, per-eye custom face definition.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../../OTA-bot-Mochi/src/eyes/face_presets.h"

inline constexpr FacePreset GUIDE_SIMPLE_FACE = [] {
	FacePreset p;
	p.name = "my_curious";
	p.left.widthScale = 1.0f;
	p.left.heightScale = 1.15f;
	p.left.openness = 1.0f;
	p.right.widthScale = 0.9f;
	p.right.heightScale = 0.65f;
	p.right.openness = 0.7f;
	p.gazeX = 0.15f;
	p.gazeY = -0.10f;
	return p;
}();
static_assert(face_preset_valid(GUIDE_SIMPLE_FACE), "Guide example must remain valid");
