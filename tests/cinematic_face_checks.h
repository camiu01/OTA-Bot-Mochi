/**
 * @file cinematic_face_checks.h
 * @brief Independent intent and complete-cycle raster checks for cinematic and anime additions.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../OTA-bot-Mochi/src/eyes/face_cinematics.h"
#include <set>

/** @brief Comedy props stay visible and each new gag moves even during its full-strength hold. */
inline void comedy_intent(Arduino_GFX &gfx, const EyeConfig &cfg, const EyeLayout &pair,
		FaceScene scene, const std::vector<uint16_t> &active) {
	if (cfg.expression < RUBBER_BAND) { return; }
	uint16_t marker = cfg.expression == PEEKABOO ? eye_rgb(72,135,205) :
		(cfg.expression == POPCORN ? eye_rgb(255,225,142) :
		(cfg.expression == SPRING_EYES ? eye_rgb(145,190,205) : eye_rgb(195,115,235)));
	if (cfg.expression >= PEEKABOO && cfg.expression <= COMIC_GLASSES) {
		assert(std::count(active.begin(), active.end(), marker) > 0);
	}
	if (cfg.expression == RUBBER_BAND) {
		for (auto pixel : active) {
			assert(pixel == cfg.bgColor || pixel == cfg.fgColor || pixel == eye_mix(cfg.fgColor, cfg.bgColor, 70));
		}
	}
	if (cfg.expression == BALLOON_EYES) {
		for (uint16_t ink : {eye_rgb(255,145,190), eye_rgb(115,220,245)}) {
			int left = cfg.screenW, right = -1, top = cfg.screenH, bottom = -1;
			for (int y = 0; y < cfg.screenH; ++y) {
				for (int x = 0; x < cfg.screenW; ++x) {
					if (active[y * cfg.screenW + x] != ink) { continue; }
					left = std::min(left, x); right = std::max(right, x);
					top = std::min(top, y); bottom = std::max(bottom, y);
				}
			}
			assert(right > left && bottom > top);
			assert(active[top * cfg.screenW + left] != ink);
			assert(active[(top + bottom) / 2 * cfg.screenW + (left + right) / 2] == ink);
		}
	}
	gfx.fillScreen(cfg.bgColor); assert(scene(&gfx, cfg, pair, 2200, 1));
	assert(gfx.pixels != active);
}

/** @brief All appended scenes are distinct, animated and reversible on every supported style and six panels. */
inline void cinematic_face_checks() {
	const int16_t panels[][2] = {{240,280}, {240,320}, {320,480}, {368,448}, {128,160}, {160,128}};
	for (const auto &panel : panels) {
		Arduino_GFX gfx(panel[0], panel[1]); eyes_init(&gfx);
		for (uint8_t style = 0; style < EYE_STYLE_COUNT; ++style) {
			eyes_set_style(EyeStyle(style)); eyes_life_reset();
			eyes_set_eye_colors(0x0841, 0xFFFF, 0xF800, 0x07E0);
			std::set<std::vector<uint16_t>> seen;
			for (uint8_t id = HACKER; id < EXPRESSION_COUNT; ++id) {
				eyes_set_expression(Expression(id)); eyes_state() = EyeState{};
				auto cfg = eyes_config(); auto pair = eye_layout(cfg, EyeState{});
				auto scene = face_preset(Expression(id))->performanceScene; assert(scene);
				gfx.fillScreen(cfg.bgColor); assert(scene(&gfx, cfg, pair, 0, 0));
				auto rest = gfx.pixels;
				gfx.fillScreen(cfg.bgColor); assert(scene(&gfx, cfg, pair, 3000, 1));
				auto active = gfx.pixels;
				assert(active != rest && seen.insert(active).second);
				comedy_intent(gfx, cfg, pair, scene, active);
				if (id == PORTAL || id == ORIGAMI || id == ECLIPSE) {
					assert(std::count(active.begin(), active.end(), cfg.fgColor) == 0);
				}
				if (id == PIXEL_CRASH) {
					for (auto pixel : active) { assert(pixel == cfg.fgColor || pixel == cfg.bgColor); }
				}
				if (id == MANGA_SHOCK) {
					assert(std::count(active.begin(), active.end(), cfg.fgColor) > 0);
					for (auto pixel : active) {
						assert(pixel == cfg.fgColor || pixel == cfg.bgColor || pixel == eye_mix(cfg.fgColor, cfg.bgColor, 85));
					}
				}
				if (id == SWEAT_DROP || id == ANIME_VEIN || id == SPIRIT_SEAL) {
					uint16_t marker = id == SWEAT_DROP ? eye_rgb(75,185,255) :
						(id == ANIME_VEIN ? eye_rgb(255,65,90) : eye_rgb(255,237,193));
					assert(std::count(active.begin(), active.end(), marker) > 0);
				}
				gfx.fillScreen(cfg.bgColor); assert(scene(&gfx, cfg, pair, 6000, 0));
				assert(gfx.pixels == rest);
				bool animated = false;
				for (uint32_t time : {0u, 600u, 1050u, 1500u, 2200u, 2800u, 3000u, 3300u, 4200u, 5000u, 5400u, 6000u}) {
					eyes_state().lastMs = time; eyes_draw(); auto dirty = gfx.pixels;
					eyes_state().dirtyInit = false; eyes_draw(); assert(dirty == gfx.pixels);
					animated |= dirty != rest;
					assert(eyes_config().fgColor == cfg.fgColor && eyes_config().bgColor == cfg.bgColor);
				}
				assert(animated);
			}
		}
	}
}
