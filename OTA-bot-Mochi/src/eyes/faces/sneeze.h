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

/** @brief Ease the scrunch in and out without changing shape in one frame. */
inline float scrunch(uint32_t phase) {
	float t = phase < 1200 ? (float(phase) - 900) / 300 :
		(phase < 1800 ? 1 : (2200 - float(phase)) / 400);
	t = t < 0 ? 0 : (t > 1 ? 1 : t);
	return t * t * (3 - 2 * t);
}

/** @brief Blend inhalation, compression and recovery rather than switching poses at fixed timestamps. */
inline void reactionMotion(bool left, uint32_t phase, float a, float &sx, float &sy, float &dx, float &dy) {
	(void)left; (void)dx;
	float pulse = scrunch(phase);
	sx += a * (-0.1f + 0.25f * pulse);
	sy += a * (0.18f - 0.58f * pulse);
	dy = a * (-4 + 11 * pulse);
}

/** @brief Close the ordinary eye before revealing its sneeze chevron, then reopen along the same path. */
inline bool reactionBody(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral) {
	(void)neutral;
	float pulse = scrunch(elapsed % 3000) * eye_reaction_amount(elapsed);
	if (pulse == 0) { return false; }
	float opening = pulse < 0.5f ? 1 - 2 * pulse : 2 * pulse - 1;
	EyeBox visible = b;
	visible.h = eye_limit(int16_t(6 + (b.h - 6) * opening), 1, b.h);
	visible.y += (b.h - visible.h) / 2;
	visible.radius = eye_limit(b.radius, 0, visible.h / 2);
	if (pulse >= 0.5f) { eye_reaction_chevron(gfx, cfg, visible); return true; }
	eye_body(gfx, cfg, visible);
	eye_pupils(gfx, cfg, st, visible);
	lids(gfx, cfg, visible);
	return true;
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
