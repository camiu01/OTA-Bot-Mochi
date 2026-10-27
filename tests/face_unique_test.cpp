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
#include "../OTA-bot-Mochi/src/eyes/eye_reactions.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <assert.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

/** @brief Render a stationary face as an occupied-pixel mask, ignoring all color differences. @param gfx Canvas. @param expression Mood. @return Silhouette. */
static std::vector<bool> silhouette(Arduino_GFX &gfx, Expression expression) {
	eyes_state() = EyeState{};
	eyes_set_expression(expression);
	eyes_draw();
	std::vector<bool> mask;
	for (uint16_t pixel : gfx.pixels) { mask.push_back(pixel != 0); }
	return mask;
}

/** @brief Similar emotions must differ in at least ten percent of their combined silhouette. @param width Panel width. @param height Panel height. */
static void family_separation(int16_t width, int16_t height) {
	Arduino_GFX gfx(width, height);
	const std::vector<std::vector<Expression>> families = {
		{MILD_HAPPY, HAPPY, GLEE, GRIN, BEAMING},
		{SAD, SAD_DOWN, SAD_UP, GLUM, WORRIED},
		{ANGRY, FURIOUS, GRUMPY, ANNOYED, FOCUSED},
		{SLEEPY, TIRED, LAZY, DOZE, ZEN},
		{SURPRISED, AWE, SCARED, TERRIFIED, PANICKED, SHOCKED, EXCITED}
	};
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_init(&gfx);
		eyes_set_style(static_cast<EyeStyle>(style));
		eyes_set_eye_colors(0, 0xFFFF, 0, 0);
		for (const auto &family : families) {
			for (size_t a = 0; a < family.size(); ++a) {
				auto first = silhouette(gfx, family[a]);
				for (size_t b = a + 1; b < family.size(); ++b) {
					auto second = silhouette(gfx, family[b]);
					size_t changed = 0, occupied = 0;
					for (size_t i = 0; i < first.size(); ++i) {
						changed += first[i] != second[i];
						occupied += first[i] || second[i];
					}
					if (changed * 100 < occupied * 10) {
						std::cerr << eyes_expression_name(family[a]) << " too similar to "
							<< eyes_expression_name(family[b]) << " on style " << unsigned(style)
							<< " at " << width << "x" << height << "\n";
					}
					assert(occupied > 0 && changed * 100 >= occupied * 10);
				}
			}
		}
	}
}

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
		if (expression == PONG || expression == GAUGE || expression == GLOW || expression == HEADLIGHTS || expression >= DANCING) {
			state.lastMs += 3000;
			eyes_draw();
		}
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

/** @brief Accept only RGB565 shades between neutral lids and blue water. @param pixel Paint. @param water Water ink. @param neutral Lid ink. @return Valid highlight. */
static bool crying_shade(uint16_t pixel, uint16_t water, uint16_t neutral) {
	const uint8_t shifts[] = {11, 5, 0}, masks[] = {31, 63, 31};
	for (uint8_t i = 0; i < 3; ++i) {
		int p = (pixel >> shifts[i]) & masks[i];
		int a = (water >> shifts[i]) & masks[i], b = (neutral >> shifts[i]) & masks[i];
		if (p < std::min(a, b) || p > std::max(a, b)) { return false; }
	}
	return true;
}

/** @brief Reaction colors are frame-local; dirty bounds and solid-style exclusions remain valid. @param gfx Canvas. @param style Eye style. */
static void reaction_frame(Arduino_GFX &gfx, uint8_t style) {
	eyes_draw();
	auto dirty = gfx.pixels;
	const auto &st = eyes_state();
	uint16_t ink = eye_reaction_ink(eyes_config(), st.lastMs - st.effectStartMs);
	bool crying = eyes_config().expression == CRYING;
	uint16_t neutral = eyes_config().fgColor;
	assert(st.prevX >= 0 && st.prevY >= 0);
	assert(st.prevX + st.prevW <= gfx.width() && st.prevY + st.prevH <= gfx.height());
	bool visible = false;
	for (uint16_t pixel : dirty) {
		visible |= pixel == ink || (crying && pixel == neutral);
		assert(pixel == 0 || pixel == ink ||
			(crying && crying_shade(pixel, ink, neutral)) ||
			((style == NORMAL || style == BLOUB) && (pixel == 0xF800 || pixel == 0x07E0)));
	}
	assert(visible && eyes_config().fgColor == 0x07FF);
	eyes_state().dirtyInit = false;
	eyes_draw();
	assert(dirty == gfx.pixels);
}

/** @brief GIF-informed reactions animate, recover exactly, survive rollover, and never leave trails. @param w Panel width. @param h Panel height. */
static void reaction_cycles(int16_t w, int16_t h) {
	Arduino_GFX gfx(w, h);
	const Expression reactions[] = {HAPPY, MILD_HAPPY, GRIN, GLEE, BEAMING, ANGRY,
		GRUMPY, FURIOUS, CRYING, SLEEPY, DOZE, SQUINT, SMIRK, SMUG, SASSY,
		SCARED, TERRIFIED, NERVOUS, SURPRISED, SHOCKED, AWE, LOVE, SWIRLY, SNEEZE};
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_init(&gfx);
		eyes_set_style(static_cast<EyeStyle>(style));
		eyes_set_eye_colors(0, 0x07FF, 0xF800, 0x07E0);
		for (Expression mood : reactions) {
			eyes_state() = EyeState{};
			eyes_set_expression(mood);
			uint32_t entry = UINT32_MAX - 500;
			eyes_state().lastMs = entry;
			reaction_frame(gfx, style);
			auto baseline = gfx.pixels;
			bool animated = false;
			for (uint32_t elapsed = 150; elapsed <= 3000; elapsed += 150) {
				eyes_state().lastMs = entry + elapsed;
				reaction_frame(gfx, style);
				animated |= !std::equal(gfx.pixels.begin(), gfx.pixels.end(), baseline.begin(),
					[](uint16_t a, uint16_t b) { return (a == 0) == (b == 0); });
			}
			assert(animated && gfx.pixels == baseline);
			eyes_state().lastMs = entry + 3001;
			eyes_state().blinking = true;
			reaction_frame(gfx, style);
			eyes_state().blinking = false;
			eyes_set_expression(NEUTRAL);
			reaction_frame(gfx, style);
		}
	}
}

/** @brief Neutral closed lids touch broad blue curtains; growth and drainage both travel downward. */
static void crying_streams() {
	EyeConfig cfg{};
	cfg.expression = CRYING; cfg.style = MOCHI; cfg.fgColor = 0x07FF;
	for (int16_t height : {int16_t(30), int16_t(85), int16_t(160)}) {
		Arduino_GFX gfx(160, 200);
		EyeBox box = {20, 10, 44, height, 6, true};
		int16_t lid = eye_limit(height / 9, 3, 9);
		int16_t lid_y = box.y + height / 6, center = box.x + box.w / 2;
		int16_t previous_bottom = 0;
		for (uint32_t time : {800u, 1000u, 1200u, 1400u}) {
			gfx.fillScreen(0);
			eye_reaction_tears(&gfx, cfg, EyeState{}, box, time, 0xFFFF);
			assert(gfx.pixels[(lid_y + lid / 2) * gfx.width() + center] == 0xFFFF);
			int16_t bottom = 0, width = 0;
			for (int16_t y = 0; y < gfx.height(); ++y) {
				for (int16_t x = 0; x < gfx.width(); ++x) {
					if (gfx.pixels[y * gfx.width() + x] == 0) { continue; }
					assert(x >= box.x && x < box.x + box.w && y >= box.y && y < box.y + box.h);
					bottom = y;
					if (y == lid_y + lid) { ++width; }
				}
			}
			assert(width >= box.w * 3 / 4);
			for (int16_t y = lid_y + lid; y <= bottom; ++y) {
				assert(gfx.pixels[y * gfx.width() + center] != 0);
			}
			assert(bottom > previous_bottom);
			previous_bottom = bottom;
		}
		assert(previous_bottom >= box.y + box.h - 4);
		int16_t top, begin, end;
		eye_crying_water_rows(box, 2400, top, begin, end);
		assert(begin > 0 && begin < end); // The tail falls off below the closed eye.
		eye_crying_water_rows(box, 2500, top, begin, end);
		assert(begin == end && eye_crying_closure(2500) == 1);
	}
	assert(eye_crying_closure(0) == 0 && eye_crying_closure(650) == 1);
	assert(eye_crying_closure(2850) == 0 && eye_crying_closure(3000) == 0);
	float sx = 1, sy = 1, dx = 0, dy = 0;
	eye_reaction_motion(CRYING, true, 1200, 1, sx, sy, dx, dy);
	assert(sx > 1 && sx <= 1.281f && sy == 1 && dx == 0 && dy != 0 && fabsf(dy) <= 3);
}

/** @brief Closure, water growth, and reopening cannot replace most of an eye in one 33 ms frame. */
static void crying_continuity() {
	for (uint8_t style = 0; style < 4; ++style) {
		Arduino_GFX gfx(240, 280);
		eyes_init(&gfx);
		eyes_set_style(static_cast<EyeStyle>(style));
		eyes_set_eye_colors(0, 0x07FF, 0, 0);
		eyes_set_expression(CRYING);
		eyes_draw();
		auto previous = gfx.pixels;
		EyeBox box = eye_layout(eyes_config(), eyes_state()).left;
		for (uint32_t now = 33; now <= 3036; now += 33) {
			eyes_state().lastMs = now;
			eyes_draw();
			size_t changed = 0;
			for (size_t i = 0; i < previous.size(); ++i) {
				changed += (previous[i] == 0) != (gfx.pixels[i] == 0);
			}
			assert(changed * 100 <= size_t(2 * box.w * box.h) * 20);
			previous = gfx.pixels;
		}
	}
}

/** @brief Reaction colors match their themes, blend smoothly, and restore the selected ink. */
static void reaction_colors() {
	EyeConfig cfg{};
	cfg.fgColor = 0xFFFF;
	const Expression moods[] = {LOVE, CRYING, ANGRY, GRUMPY, FURIOUS};
	const uint16_t colors[] = {eye_rgb(255, 92, 150), eye_rgb(75, 205, 255),
		eye_rgb(255, 135, 65), eye_rgb(255, 135, 65), eye_rgb(245, 70, 85)};
	for (uint8_t i = 0; i < 5; ++i) {
		cfg.expression = moods[i];
		assert(eye_reaction_ink(cfg, 0) == cfg.fgColor);
		assert(eye_reaction_ink(cfg, 550) == eye_mix(colors[i], cfg.fgColor, 50));
		assert(eye_reaction_ink(cfg, 1200) == colors[i]);
		assert(eye_reaction_ink(cfg, 3000) == cfg.fgColor);
		assert(cfg.fgColor == 0xFFFF);
	}
	cfg.expression = NEUTRAL;
	assert(eye_reaction_ink(cfg, 1200) == cfg.fgColor);
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
	family_separation(240, 280);
	family_separation(368, 448);
	family_separation(128, 160);
	assert(eye_reaction_amount(0) == 0 && eye_reaction_amount(3000) == 0);
	assert(eye_reaction_amount(800) == 1 && eye_reaction_amount(2200) == 1);
	reaction_colors();
	crying_streams();
	crying_continuity();
	reaction_cycles(240, 280);
	reaction_cycles(368, 448);
	reaction_cycles(128, 160);
	reaction_cycles(160, 128);
	std::cout << "Expression uniqueness, silhouette separation, and colored reaction cycles passed\n";
	return 0;
}
