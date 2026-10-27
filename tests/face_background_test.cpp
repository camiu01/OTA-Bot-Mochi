/**
 * @file face_background_test.cpp
 * @brief Themed backdrops stay outside the eye ink, claim full dirty bounds and never animate.
 *
 * @details A backdrop is a permanent part of the mood, so it must be visible
 *          outside the pair, must not hide the eye body, and must return the
 *          same pixels after the performance clock has run out. The catalog
 *          rule also keeps the frozen raster fixtures honest: only the two
 *          legacy effect faces and this batch may paint behind the eyes.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_geometry.h"
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <algorithm>
#include <assert.h>
#include <iostream>

/** @brief Moods owning a persistent backdrop; GLOW and HEADLIGHTS predate the batch. */
static const Expression BACKDROPPED[] = {MECHA, SAMURAI, TITAN, YOKAI, NEON, WHEELS, UWU};

/** @brief Check one pixel against both eye boxes, including their rounding margin. */
static bool outside_pair(const EyeLayout &pair, int16_t x, int16_t y) {
	for (const EyeBox &b : {pair.left, pair.right}) {
		if (x >= b.x - 2 && x < b.x + b.w + 2 && y >= b.y - 2 && y < b.y + b.h + 2) { return false; }
	}
	return true;
}

/** @brief Only the two legacy effect faces and the themed batch may paint a backdrop. */
static void catalog_rule() {
	for (uint8_t id = 0; id < EXPRESSION_COUNT; ++id) {
		const FacePreset *p = face_preset(Expression(id));
		assert(p != nullptr);
		bool expected = id == GLOW || id == HEADLIGHTS;
		for (Expression mood : BACKDROPPED) { expected = expected || id == mood; }
		assert((p->background != nullptr) == expected);
		if (expected) { assert(p->bounds != nullptr); }
	}
}

/** @brief One panel/style: visible backdrop, visible eyes, full dirty bounds and a static frame. */
static void check(int16_t w, int16_t h, EyeStyle style) {
	Arduino_GFX gfx(w, h);
	eyes_init(&gfx);
	eyes_set_style(style);
	eyes_life_reset();
	eyes_set_eye_colors(0x0841, 0xFFFF, 0xF800, 0x07E0);
	for (Expression mood : BACKDROPPED) {
		eyes_state() = EyeState{};
		eyes_set_expression(mood);
		eyes_draw();
		const EyeState &st = eyes_state();
		assert(st.prevX == 0 && st.prevY == 0 && st.prevW == w && st.prevH == h);
		auto first = gfx.pixels;
		EyeLayout pair = eye_layout(eyes_config(), EyeState{});
		size_t painted = 0, ink = 0;
		for (int16_t y = 0; y < h; ++y) {
			for (int16_t x = 0; x < w; ++x) {
				uint16_t pixel = first[size_t(y) * w + x];
				if (outside_pair(pair, x, y)) { painted += pixel != eyes_config().bgColor; }
				else { ink += pixel == eyes_config().fgColor; }
			}
		}
		if (painted == 0 || ink == 0) {
			std::cerr << "backdrop " << eyes_expression_name(mood) << " at " << w << "x" << h
				<< " style " << unsigned(style) << " painted " << painted << " ink " << ink << "\n";
		}
		assert(painted > 0 && ink > 0);
		// A backdrop is time-independent, so a finished performance repaints the same frame.
		eyes_state().lastMs = 5900;
		eyes_draw();
		assert(gfx.pixels == first);
		eyes_state().dirtyInit = false;
		eyes_draw();
		assert(gfx.pixels == first);
		if (style == MOCHI || style == TRAPEZ) {
			assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), uint16_t(0xF800)) == 0);
			assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), uint16_t(0x07E0)) == 0);
		}
	}
}

/** @brief Run the catalog rule and the backdrop checks on every panel and style. */
int main() {
	assert(settings_init());
	catalog_rule();
	const int16_t panels[][2] = {{240, 280}, {240, 320}, {320, 480}, {368, 448}, {128, 160}, {160, 128}};
	for (const auto &panel : panels) {
		for (uint8_t style = 0; style < 4; ++style) { check(panel[0], panel[1], EyeStyle(style)); }
	}
	std::cout << "Themed backdrops, full dirty bounds, visible eyes and static frames passed\n";
	return 0;
}
