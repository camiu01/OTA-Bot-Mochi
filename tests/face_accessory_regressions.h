/**
 * @file face_accessory_regressions.h
 * @brief External Devil/Halo/Angel accessories, canonical registration and clean live redraw.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

/** @brief Halo has only an aureole, Angel adds two wings, Devil has exactly two external horns. */
static void accessory_shapes() {
	Arduino_GFX gfx(240, 280);
	eyes_init(&gfx); eyes_set_style(MOCHI);
	for (Expression face : {DEVIL, HALO, ANGEL}) {
		EyeConfig cfg = eyes_config(); cfg.expression = face;
		const auto *p = face_preset(face);
		assert(p && p->background && !p->glyph && p->bounds);
		EyeLayout layout = eye_layout(cfg, EyeState{});
		gfx.fillScreen(0); p->background(&gfx, cfg, EyeState{}, layout);
		int16_t top = std::min(layout.left.y, layout.right.y);
		unsigned left = 0, right = 0, upper = 0, lateral = 0;
		for (int y = 0; y < gfx.height(); ++y) {
			for (int x = 0; x < gfx.width(); ++x) {
				uint16_t pixel = gfx.pixels[y * gfx.width() + x];
				if (!pixel) { continue; }
				assert(!(x >= layout.left.x && x < layout.left.x + layout.left.w &&
					y >= layout.left.y && y < layout.left.y + layout.left.h));
				assert(!(x >= layout.right.x && x < layout.right.x + layout.right.w &&
					y >= layout.right.y && y < layout.right.y + layout.right.h));
				if (y < top) { ++upper; }
				if (x < layout.left.x || x >= layout.right.x + layout.right.w) { ++lateral; }
				if (pixel == eye_rgb(187, 221, 247)) { x < 120 ? ++left : ++right; }
			}
		}
		assert(upper > 20);
		if (face == ANGEL) { assert(left > 20 && right > 20 && lateral > 40); }
		else { assert(!left && !right && !lateral); }
	}
}

/** @brief Accessories remain in bounds through gaze/blink and clear completely when leaving. */
static void accessory_redraw() {
	for (const auto &panel : {std::pair<int, int>{240, 280}, {368, 448}, {128, 160}, {160, 128}}) {
		Arduino_GFX gfx(panel.first, panel.second);
		for (uint8_t style = 0; style < EYE_STYLE_COUNT; ++style) {
			eyes_init(&gfx); eyes_set_style(EyeStyle(style)); eyes_life_reset();
			for (Expression face : {DEVIL, HALO, ANGEL}) {
				for (bool blink : {false, true}) {
					for (int16_t gaze : {int16_t(-20), int16_t(20)}) {
						eyes_state() = EyeState{}; eyes_state().blinking = blink;
						eyes_state().saccadeX = gaze; eyes_state().saccadeY = gaze;
						eyes_set_expression(face); eyes_draw();
						auto dirty = gfx.pixels; eyes_state().dirtyInit = false; eyes_draw();
						assert(dirty == gfx.pixels);
						eyes_set_expression(NEUTRAL); eyes_draw(); dirty = gfx.pixels;
						eyes_state().dirtyInit = false; eyes_draw(); assert(dirty == gfx.pixels);
					}
				}
			}
		}
	}
}
