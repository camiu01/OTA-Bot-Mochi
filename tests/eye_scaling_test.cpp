/**
 * @file eye_scaling_test.cpp
 * @brief Panel-adaptive eye geometry: centring, fit, and name uniqueness per panel.
 *
 * Style presets are tuned on the 240x280 reference panel, so other panels must
 * recenter the pair, scale it with the smaller axis plus the large-panel fill
 * bonus (capped so the widest expression keeps its full gaze range), and never
 * clip or collapse two expressions into the same picture. Rendering runs on a
 * native raster substitute; it does not prove electrical behaviour on a real
 * panel.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_geometry.h"
#include "../OTA-bot-Mochi/src/eyes/eye_palette.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <assert.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

/** @brief Render one expression with gaze, bobbing, and blink held still. @param expression Expression. */
static void render(Expression expression) {
	EyeState &state = eyes_state();
	state.saccadeX = state.saccadeY = state.tiltX = state.tiltY = state.floatY = 0;
	state.blinking = false;
	eyes_set_expression(expression);
	state.dirtyInit = false;
	eyes_draw();
	if (expression == PONG || expression == GAUGE || expression == GLOW || expression == HEADLIGHTS || expression >= DANCING) {
		state.lastMs = state.effectStartMs + 3000;
		eyes_draw();
	}
}

/** @brief Measure the painted bounding box of the current frame. @param gfx Canvas. @return True when something was painted. */
static bool painted_box(const Arduino_GFX &gfx, int16_t &x0, int16_t &y0, int16_t &x1, int16_t &y1) {
	x0 = y0 = 32767;
	x1 = y1 = -1;
	for (int y = 0; y < gfx.height(); ++y) {
		for (int x = 0; x < gfx.width(); ++x) {
			if (!gfx.pixels[size_t(y) * gfx.width() + x]) { continue; }
			if (x < x0) { x0 = int16_t(x); }
			if (y < y0) { y0 = int16_t(y); }
			if (x > x1) { x1 = int16_t(x); }
			if (y > y1) { y1 = int16_t(y); }
		}
	}
	return x1 >= 0;
}

/** @brief Every expression of one style must stay inside the panel. @param style Style. @param w Panel width. @param h Panel height. */
static void fit_checks(uint8_t style, int16_t w, int16_t h) {
	eyes_set_style(static_cast<EyeStyle>(style));
	for (uint8_t value = 0; value < EXPRESSION_COUNT; ++value) {
		eyes_set_expression(static_cast<Expression>(value));
		EyeLayout box = eye_layout(eyes_config(), eyes_state());
		assert(box.left.x >= 0 && box.left.y >= 0);
		assert(box.right.x >= 0 && box.right.y >= 0);
		assert(box.left.x + box.left.w <= w && box.left.y + box.left.h <= h);
		assert(box.right.x + box.right.w <= w && box.right.y + box.right.h <= h);
	}
}

/** @brief Scaling must not merge two names into one picture on this panel. @param gfx Canvas. @param style Style. @return True when the style stays unique. */
static bool unique_checks(Arduino_GFX &gfx, uint8_t style) {
	eyes_set_style(static_cast<EyeStyle>(style));
	std::map<std::vector<uint16_t>, uint8_t> seen;
	std::string collisions;
	for (uint8_t value = 0; value < EXPRESSION_COUNT; ++value) {
		render(static_cast<Expression>(value));
		auto placed = seen.emplace(gfx.pixels, value);
		if (placed.second) { continue; }
		collisions += settings_mood_name(placed.first->second);
		collisions += " == ";
		collisions += settings_mood_name(value);
		collisions += "; ";
	}
	if (!collisions.empty()) {
		std::cerr << "style " << unsigned(style) << " on " << gfx.width() << "x" << gfx.height()
			<< ": " << collisions << "\n";
	}
	return collisions.empty();
}

/** @brief Check one panel: fit percent, centered pair, in-bounds eyes, unique names. @param w Width. @param h Height. @param expect_scale Expected eye_scale percent. @return True when names stayed unique. */
static bool panel(int16_t w, int16_t h, int16_t expect_scale) {
	Arduino_GFX gfx(w, h);
	eyes_init(&gfx);
	eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
	eyes_set_style(MOCHI);
	assert(eye_scale(eyes_config()) == expect_scale);
	render(NEUTRAL);
	int16_t x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	assert(painted_box(gfx, x0, y0, x1, y1));
	assert((x0 + x1) / 2 - w / 2 >= -2 && (x0 + x1) / 2 - w / 2 <= 2);
	assert((y0 + y1) / 2 - h / 2 >= -2 && (y0 + y1) / 2 - h / 2 <= 2);
	for (uint8_t style = 0; style < 4; ++style) {
		fit_checks(style, w, h);
		for (Expression expression : {LOVE, STAR}) {
			render(expression);
			EyeBox eye = eye_layout(eyes_config(), eyes_state()).left;
			int16_t first = w, last = -1;
			for (int y = eye.y; y < eye.y + eye.h; ++y) {
				for (int x = eye.x; x < eye.x + eye.w; ++x) {
					if (gfx.pixels[size_t(y) * w + x] != eye_accent(expression)) { continue; }
					if (x < first) { first = int16_t(x); }
					if (x > last) { last = int16_t(x); }
				}
			}
			int16_t size = eye.w < eye.h ? eye.w : eye.h;
			assert(last - first + 1 >= (size - 4) * 8 / 10);
		}
	}
	bool unique = unique_checks(gfx, MOCHI);
	return unique_checks(gfx, BLOUB) && unique;
}

/** @brief Run the panel fit proof on six panel geometries. @return Success. */
int main() {
	assert(settings_init());
	// The fit uses the smaller axis, so a taller or narrower panel never
	// overflows the pair; larger panels then reinvest half of the extra ratio
	// into the face (133 -> 154, 153 -> 192, capped to 189 by the MOCHI full
	// gaze budget on 368x448). The last two sizes prove the 50 percent floor,
	// where the fill bonus does not apply.
	bool unique = panel(240, 280, 100);
	unique = panel(240, 320, 100) && unique;
	unique = panel(320, 480, 154) && unique;
	unique = panel(368, 448, 189) && unique;
	unique = panel(128, 160, 53) && unique;
	unique = panel(160, 128, 50) && unique;
	assert(unique);
	std::cout << "Panel-adaptive eye geometry passed on six panel sizes\n";
	return 0;
}
