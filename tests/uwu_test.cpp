/**
 * @file uwu_test.cpp
 * @brief UwU has white top-open U eyes and separate pink cheeks, without a mouth or backdrop.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/faces/uwu.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <cassert>
#include <iostream>
namespace uwu = face_defs::uwu_face;

/** @brief Check white U silhouettes, pink cheek ink, an empty mouth region and no backdrop. */
static void shape(EyeConfig cfg) {
	Arduino_GFX gfx(cfg.screenW, cfg.screenH);
	cfg.fgColor = 0x07FF;
	gfx.fillScreen(cfg.bgColor);
	EyeLayout original = eye_layout(cfg, EyeState{});
	assert(uwu::scene(&gfx, cfg, original, 2000, 1));
	assert(face_preset(UWU)->background == nullptr);
	assert(eye_performance_ink(cfg, true, 2000) == eye_rgb(255, 255, 255));
	for (bool left : {true, false}) {
		EyeBox b = uwu::target(cfg, left, 2000);
		assert(b.h > b.w && b.radius == b.w / 2);
		int16_t cx = b.x + b.w / 2;
		assert(gfx.pixels[(b.y + b.h / 4) * cfg.screenW + cx] == cfg.bgColor);
		assert(gfx.pixels[(b.y + b.h - 2) * cfg.screenW + cx] == 0xFFFF);
		int16_t cheekY = b.y + b.h + cfg.screenH / 25 + (cfg.screenH / 35 + 1) / 2;
		assert(gfx.pixels[cheekY * cfg.screenW + cx] == eye_rgb(255, 85, 139));
	}
	for (int16_t y = 0; y < cfg.screenH; ++y) {
		for (int16_t x = cfg.screenW / 2 - cfg.screenW / 20; x <= cfg.screenW / 2 + cfg.screenW / 20; ++x) {
			assert(gfx.pixels[y * cfg.screenW + x] == cfg.bgColor);
		}
	}
	for (uint16_t pixel : gfx.pixels) {
		assert(pixel == cfg.bgColor || pixel == 0xFFFF || pixel == eye_rgb(255, 85, 139));
	}
}
/** @brief All styles morph cleanly on both target panels and the small native fixtures. */
int main() {
	assert(settings_init());
	const int16_t panels[][2] = {{240,280}, {240,320}, {320,480}, {368,448}, {128,160}, {160,128}};
	for (const auto &p : panels) {
		Arduino_GFX gfx(p[0], p[1]); eyes_init(&gfx);
		for (uint8_t style = 0; style < 4; ++style) {
			eyes_set_style(EyeStyle(style)); eyes_life_reset();
			eyes_set_eye_colors(0x0841, 0xFFFF, 0xF800, 0x07E0); eyes_set_expression(UWU);
			shape(eyes_config());
			eyes_state() = EyeState{}; eyes_draw();
			eyes_state().lastMs = 6000; eyes_draw(); auto rest = gfx.pixels;
			for (uint32_t t = 125; t <= 6000; t += 125) {
				eyes_state().lastMs = t; eyes_draw(); auto frame = gfx.pixels;
				eyes_state().dirtyInit = false; eyes_draw(); assert(gfx.pixels == frame);
				if (t >= 5500) { assert(gfx.pixels == rest); }
			}
		}
	}
	std::cout << "White U eyes, pink cheeks, no mouth or scenery and clean UwU cycles passed on six panels\n";
}
