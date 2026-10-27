/**
 * @file twitch.h
 * @brief twitch definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace twitch_face {
using namespace face_primitives;

/** @brief twitch lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	strip(gfx, b, b.h * (b.left ? 8 : 35) / 100, false, cfg.bgColor);
			if (!b.left) { strip(gfx, b, b.h / 4, true, cfg.bgColor); }
}

/** @brief Immutable twitch face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "twitch";
	p.lids = lids;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid twitch face parameters");
} }
