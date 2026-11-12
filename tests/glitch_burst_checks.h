/**
 * @file glitch_burst_checks.h
 * @brief Sample a real brief corruption, rather than mistaking intact recovery for a missing face.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../OTA-bot-Mochi/src/eyes/face_support.h"
#include <algorithm>
#include <cassert>

/** @brief A Glitch cycle must visibly corrupt the eye during its active window on every style. */
inline void glitch_visible_burst(Arduino_GFX &gfx) {
	auto &state = eyes_state();
	for (uint32_t t = 2000; t <= 2600; t += 33) {
		state.lastMs = state.effectStartMs + t; eyes_draw();
		if (std::any_of(gfx.pixels.begin(), gfx.pixels.end(), [](uint16_t ink) {
			return ink == eye_rgb(64, 212, 255) || ink == eye_rgb(248, 64, 184);
		})) { return; }
	}
	assert(false && "Glitch must have a visible brief burst, not only intact eyes");
}
