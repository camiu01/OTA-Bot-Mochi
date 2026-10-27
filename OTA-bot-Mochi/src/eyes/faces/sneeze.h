/**
 * @file sneeze.h
 * @brief sneeze definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace sneeze_face {
using namespace face_primitives;

/** @brief sneeze lids procedure. */
inline void lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	slope(gfx, cfg, b, true, b.h / 3);
			strip(gfx, b, b.h / 3, true, cfg.bgColor);
}

/** @brief sneeze reactionMotion procedure. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)phase; (void)a; (void)sx; (void)sy; (void)dx; (void)dy;
	constexpr Expression expr = SNEEZE; (void)expr;
	float wave = sinf(phase * 0.0062831853f); (void)wave;
	if (phase < 1100) { sx -= a * 0.1f; sy += a * 0.18f; dy = -a * 4; }
			else if (phase < 2000) { sx += a * 0.15f; sy -= a * 0.4f; dy = a * 7; }
			else { dy = -a * 3; }
}

/** @brief sneeze reactionBody procedure. */
inline bool reactionBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	(void)st; (void)neutral;
	if (eye_reaction_amount(elapsed) < 0.8f || elapsed % 3000 < 1100 || elapsed % 3000 >= 2000) { return false; }
	eye_reaction_chevron(gfx, cfg, b); return true;
}

/** @brief Immutable sneeze face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "sneeze";
	p.lids = lids;
	p.reactionMotion = reactionMotion;
	p.reactionBody = reactionBody;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid sneeze face parameters");
} }
