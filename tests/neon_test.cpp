/**
 * @file neon_test.cpp
 * @brief Neon glare extends outside the outline, fades outward, and leaves a hollow center.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/faces/neon.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <cassert>
#include <iostream>

/** @brief Sum RGB565 channel intensities for monotonic glow samples. */
static unsigned light(uint16_t ink) { return (ink >> 11) + ((ink >> 5) & 63) + (ink & 31); }
/** @brief Check a panel/style's halo, hollow body, dirty redraw and exact restoration. */
static void check(int16_t w, int16_t h, EyeStyle style) {
	Arduino_GFX gfx(w, h), bare(w, h);
	eyes_init(&gfx); eyes_set_style(style); eyes_life_reset();
	eyes_set_eye_colors(0x0841, 0xFFFF, 0xF800, 0x07E0); eyes_set_expression(NEON);
	eyes_state() = EyeState{}; eyes_draw();
	eyes_state().lastMs = 6000; eyes_draw(); auto rest = gfx.pixels;
	eyes_state().lastMs = 3000; eyes_draw(); auto lit = gfx.pixels;
	const EyeConfig &cfg = eyes_config();
	EyeLayout pair = eye_layout(cfg, EyeState{});
	int16_t step = w / 160 + 1;
	for (const EyeBox &b : {pair.left, pair.right}) {
		int16_t cy = b.y + b.h / 2;
		assert(gfx.pixels[cy * w + b.x + b.w / 2] == cfg.bgColor);
		bare.fillScreen(cfg.bgColor); eye_body(&bare, cfg, b);
		int16_t edge = b.x;
		while (edge < b.x + b.w && bare.pixels[cy * w + edge] == cfg.bgColor) { ++edge; }
		assert(edge < b.x + b.w && edge >= 2 * step);
		uint16_t near = gfx.pixels[cy * w + edge - step];
		uint16_t far = gfx.pixels[cy * w + edge - 2 * step];
		if (!(light(near) > light(far) && light(far) > light(cfg.bgColor))) {
			std::cerr << w << "x" << h << " style " << unsigned(style) << " left " << b.left
				<< " near/far/bg " << near << "/" << far << "/" << cfg.bgColor << "\n";
		}
		assert(light(near) > light(far) && light(far) > light(cfg.bgColor));
	}
	eyes_state().dirtyInit = false; eyes_draw(); assert(gfx.pixels == lit);
	eyes_state().lastMs = 6000; eyes_draw(); assert(gfx.pixels == rest);
	assert(eyes_config().fgColor == 0xFFFF && eyes_config().bgColor == 0x0841);
}
int main() {
	assert(settings_init());
	const int16_t panels[][2] = {{240,280}, {240,320}, {320,480}, {368,448}, {128,160}, {160,128}};
	for (const auto &p : panels) {
		for (uint8_t style = 0; style < 4; ++style) { check(p[0], p[1], EyeStyle(style)); }
	}
	std::cout << "Neon outward glare, falloff, hollow centers and palette restoration passed on six panels\n";
}
