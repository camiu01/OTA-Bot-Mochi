/**
 * @file face_background_test.cpp
 * @brief Themed backdrops stay outside the eye ink, claim full dirty bounds and fade gently into a stable frame.
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
#include "../OTA-bot-Mochi/src/eyes/eye_backdrop.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <algorithm>
#include <assert.h>
#include <iostream>

/** @brief Moods owning a persistent backdrop; GLOW and HEADLIGHTS predate the batch. */
static const Expression BACKDROPPED[] = {MECHA, SAMURAI, TITAN, YOKAI, NEON, WHEELS, SAKURA, BAMBOO, TRAFFIC, MOON, ZEN, DAYDREAM};

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

/** @brief Entry starts empty, advances toward the final ink and never restarts. */
static void entry(Arduino_GFX &gfx) {
	const EyeConfig cfg = eyes_config();
	const FacePreset *preset = face_preset(cfg.expression);
	EyeLayout pair = eye_layout(cfg, EyeState{});
	auto draw = [&](uint32_t elapsed, float opacity = 1) {
		gfx.fillScreen(cfg.bgColor);
		EyeConfig local = cfg;
		EyeState state{};
		state.lastMs = elapsed;
		state.backdropOpacity = opacity;
		preset->background(&gfx, local, state, pair);
		return gfx.pixels;
	};
	auto initial = draw(0);
	assert(std::count(initial.begin(), initial.end(), cfg.bgColor) == int(initial.size()));
	assert(draw(1200, 0) == initial);
	auto final = draw(1200);
	assert(final != initial);
	auto middle = draw(600);
	assert(middle != initial && middle != final);
	auto previous = initial;
	for (uint32_t elapsed : {200u, 600u, 1000u, 1200u}) {
		auto current = draw(elapsed);
		for (size_t i = 0; i < current.size(); ++i) {
			for (unsigned shift : {0u, 5u, 11u}) {
				int mask = shift == 5 ? 63 : 31;
				int target = (final[i] >> shift) & mask;
				int before = (previous[i] >> shift) & mask;
				int after = (current[i] >> shift) & mask;
				assert(std::abs(target - after) <= std::abs(target - before));
			}
		}
		previous = current;
	}
	assert(draw(6000) == final && draw(12000) == final);
	if (cfg.expression == SAKURA || cfg.expression == DAYDREAM) {
		auto drifting = draw(3000);
		assert(drifting != final && draw(3000) == drifting);
		assert(draw(4500) == final);
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
		entry(gfx);
		eyes_draw();
		eyes_state().lastMs = 6000; eyes_draw();
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
		// Once entry is complete, the backdrop stays unchanged across cycles.
		eyes_state().lastMs = 5900;
		eyes_draw();
		assert(gfx.pixels == first);
		eyes_state().dirtyInit = false;
		eyes_draw();
		assert(gfx.pixels == first);
		for (uint32_t elapsed : {1500u, 2500u, 3000u, 4500u, 6000u}) {
			eyes_state().lastMs = elapsed; eyes_draw();
			auto dirty = gfx.pixels;
			eyes_state().dirtyInit = false; eyes_draw();
			assert(gfx.pixels == dirty);
		}
		if (style == MOCHI || style == TRAPEZ) {
			assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), uint16_t(0xF800)) == 0);
			assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), uint16_t(0x07E0)) == 0);
		}
	}
}

/** @brief Keep scenery after the eye switch, including rapid retargets and a plain destination. */
static void handoff() {
	for (uint32_t start : {10000u, UINT32_MAX - 100u}) {
		for (bool retarget : {false, true}) {
			Arduino_GFX gfx;
			eyes_init(&gfx); eyes_set_style(MOCHI);
			eyes_set_eye_colors(0x0841, 0xFFFF, 0xF800, 0x07E0);
			eyes_state() = EyeState{};
			eyes_set_expression(MECHA);
			eyes_state().lastMs = start - 6000; eyes_draw();
			eyes_state().lastMs = start; eyes_life_step(start, 33); eyes_draw();
			size_t probe = size_t(gfx.height() / 2) * gfx.width();
			uint16_t initial = gfx.pixels[probe];
			assert(initial != 0x0841);
			eyes_set_expression(NEUTRAL); eyes_life_step(start, 0);
			for (uint32_t elapsed : {0u, 112u, 320u, 600u, 1000u, 1312u, 1912u, 2512u}) {
				eyes_state().lastMs = start + elapsed;
				eyes_life_step(start + elapsed, 33); eyes_draw();
				assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), uint16_t(0xFFFF)) > 0);
				if (elapsed < 1312 && elapsed >= 112) {
					assert(eyes_state().backdrop.leaving);
					assert(eyes_state().backdrop.since == start + 112);
					assert(eyes_state().prevW == gfx.width() && eyes_state().prevH == gfx.height());
				}
				if (elapsed == 320 || elapsed == 600) {
					assert(gfx.pixels[probe] != initial && gfx.pixels[probe] != 0x0841);
				}
				if (elapsed >= 1312) {
					assert(!eyes_state().backdrop.leaving && gfx.pixels[probe] == 0x0841);
				}
				auto dirty = gfx.pixels;
				eyes_state().dirtyInit = false; eyes_draw();
				assert(gfx.pixels == dirty);
				if (retarget && elapsed == 320) { eyes_set_expression(ZEN); eyes_life_step(start + elapsed, 0); }
			}
			if (retarget) { assert(eyes_state().backdrop.config.expression == ZEN); }
			assert(eyes_config().fgColor == 0xFFFF);
		}
	}
}

/** @brief Run the catalog rule and the backdrop checks on every panel and style. */
int main() {
	assert(settings_init());
	catalog_rule();
	handoff();
	const int16_t panels[][2] = {{240, 280}, {240, 320}, {320, 480}, {368, 448}, {128, 160}, {160, 128}};
	for (const auto &panel : panels) {
		for (uint8_t style = 0; style < 4; ++style) { check(panel[0], panel[1], EyeStyle(style)); }
	}
	std::cout << "Themed backdrops, soft entry, restrained motion, full dirty bounds and visible eyes passed\n";
	return 0;
}
