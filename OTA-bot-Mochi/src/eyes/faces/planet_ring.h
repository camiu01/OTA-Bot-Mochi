/**
 * @file planet_ring.h
 * @brief A small ringed planet orbits the face, passing behind and then in front of the eyes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_cinematics.h"
namespace face_defs { namespace planet_ring_face {
using face_cinematics::pair_body;
using face_cinematics::tint;
/** @brief Draw a small orange planet with a pale tilted ring. */
inline void planet(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t x, int16_t y, int16_t r, float a) {
	gfx->fillEllipse(x, y, r * 2, r / 3 + 1, face_cinematics::tint(cfg, eye_rgb(230, 200, 140), a));
	gfx->fillCircle(x, y, r, face_cinematics::tint(cfg, eye_rgb(240, 160, 90), a));
	gfx->drawFastHLine(x - r * 2, y, r * 4, face_cinematics::tint(cfg, eye_rgb(250, 230, 180), a));
}
/** @brief Depth order flips with the orbit; the pair glances along with it. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &pair, uint32_t ms, float a) {
	const int16_t W = cfg.screenW, H = cfg.screenH;
	float t = ms * 0.002f;
	int16_t r = W / 20 + 3, px = W / 2 + int16_t(cosf(t) * W * 0.38f), py = H / 2 + int16_t(sinf(t) * H * 0.3f);
	if (a > 0 && sinf(t) < 0) { planet(gfx, cfg, px, py, r, a); }
	pair_body(gfx, cfg, pair, a, int16_t(cosf(t) * a * W / 50), 0, 1, 1 - a * 0.15f, cfg.fgColor);
	if (a > 0 && sinf(t) >= 0) { planet(gfx, cfg, px, py, r, a); }
	return true;
}
inline constexpr FacePreset preset = face_cinematics::preset("planet_ring", scene);
static_assert(face_preset_valid(preset), "Invalid Planet Ring face");
} }
