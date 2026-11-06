/**
 * @file halo.h
 * @brief One external golden aureole above the original eyes, without wings.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_accessories.h"
namespace face_defs { namespace halo_face {
/** @brief Draw only the aureole; preserve the selected eyes and their normal pupils. */
inline void background(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &, const EyeLayout &layout) {
	face_accessories::aureole(gfx, cfg, layout);
}
/** @brief Preserve Halo's public ID independently of the new Angel face. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "halo";
	p.background = background; p.bounds = face_accessories::bounds;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid halo face");
} }
