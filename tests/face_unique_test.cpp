/**
 * @file face_unique_test.cpp
 * @brief Pairwise pixel uniqueness of every expression on every style.
 *
 * The NEUTRAL distinctness proof only says a face is visible; it says nothing
 * about two different names that silently draw the same pixels. A collision
 * here means one of the faces is unreachable in the UI, so the test prints the
 * offending name pair per style and fails.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_geometry.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <assert.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

/** @brief Render every expression deterministically and report name collisions. @param gfx Canvas. @param style Style under test. @return True when the style is collision-free. */
static bool style_uniqueness(Arduino_GFX &gfx, uint8_t style) {
	eyes_set_style(static_cast<EyeStyle>(style));
	std::map<std::vector<uint16_t>, uint8_t> seen;
	std::string collisions;
	for (uint8_t expression = 0; expression < EXPRESSION_COUNT; ++expression) {
		EyeState &state = eyes_state();
		state.saccadeX = state.saccadeY = state.tiltX = state.tiltY = 0;
		state.blinking = false;
		eyes_set_expression(static_cast<Expression>(expression));
		state.dirtyInit = false;
		eyes_draw();
		auto placed = seen.emplace(gfx.pixels, expression);
		if (placed.second) { continue; }
		collisions += settings_mood_name(placed.first->second);
		collisions += " == ";
		collisions += settings_mood_name(expression);
		collisions += "; ";
	}
	if (!collisions.empty()) {
		std::cerr << "duplicate faces on style " << unsigned(style) << ": " << collisions << "\n";
	}
	return collisions.empty();
}

/** @brief Run the pairwise proof across all four styles. @return Success. */
int main() {
	assert(settings_init());
	Arduino_GFX gfx;
	eyes_init(&gfx);
	eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
	bool unique = true;
	for (uint8_t style = 0; style < 4; ++style) { unique = style_uniqueness(gfx, style) && unique; }
	assert(unique);
	std::cout << "Pairwise expression uniqueness passed on all four styles\n";
	return 0;
}
