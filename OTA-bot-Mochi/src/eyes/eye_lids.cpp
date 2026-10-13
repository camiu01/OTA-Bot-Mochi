/**
 * @file eye_lids.cpp
 * @brief Expression masks clipped to each eye's actual asymmetric geometry.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "eye_shapes.h"

/** @brief Remove a bounded strip from an eye. @param gfx Canvas. @param b Eye. @param height Pixels. @param bottom Side. @param color Background. */
static void strip(Arduino_GFX *gfx, const EyeBox &b, int16_t height, bool bottom, uint16_t color) {
	height = eye_limit(height, 0, b.h - 2);
	if (!height) { return; }
	gfx->fillRect(b.x, bottom ? b.y + b.h - height : b.y, b.w, height, color);
}

/** @brief Cut a mirrored sloped lid, entirely inside the eye bounds. @param gfx Canvas. @param cfg Config. @param b Eye. @param angry Slope direction. @param depth Depth. */
static void slope(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, bool angry, int16_t depth) {
	int16_t x = (b.left == angry) ? b.x + b.w - 1 : b.x;
	gfx->fillTriangle(b.x, b.y, b.x + b.w - 1, b.y, x, b.y + depth, cfg.bgColor);
}

/** @brief Render emotional eyelids, not pupils, for all eye styles. @param gfx Canvas. @param cfg Config. @param b Eye. */
void eye_lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	switch (cfg.expression) {
		case ANGRY: slope(gfx, cfg, b, true, b.h / 3); break;
		case FURIOUS: slope(gfx, cfg, b, true, b.h / 2); break;
		case SAD: case SAD_DOWN: slope(gfx, cfg, b, false, b.h / 3); break;
		case WORRIED: slope(gfx, cfg, b, false, b.h / 5); break;
		case HAPPY: strip(gfx, b, b.h / 3, true, cfg.bgColor); break;
		case GLEE: strip(gfx, b, b.h / 2, true, cfg.bgColor); break;
		case FOCUSED: strip(gfx, b, b.h / 5, false, cfg.bgColor);
			strip(gfx, b, b.h / 5, true, cfg.bgColor); break;
		case UNIMPRESSED: strip(gfx, b, b.h / 2, false, cfg.bgColor); break;
		case ANNOYED: strip(gfx, b, b.h * (b.left ? 25 : 35) / 100, false, cfg.bgColor); break;
		case FRUSTRATED: slope(gfx, cfg, b, false, b.h * (b.left ? 20 : 30) / 100); break;
		case SUSPICIOUS: strip(gfx, b, b.h * (b.left ? 15 : 35) / 100, false, cfg.bgColor); break;
		case SKEPTIC: strip(gfx, b, b.h * (b.left ? 10 : 35) / 100, false, cfg.bgColor); break;
		case SQUINT: strip(gfx, b, b.h / 4, false, cfg.bgColor); break;
		case SLEEPY_EYES: strip(gfx, b, b.h / 3, false, cfg.bgColor); break;
		case SAD_UP: strip(gfx, b, b.h / 4, true, cfg.bgColor); break;
		default: break;
	}
	strip(gfx, b, cfg.lidTop, false, cfg.bgColor);
}
