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

/** @brief Render eyelid masks shared by the original expression set. @param gfx Canvas. @param cfg Config. @param b Eye. */
static void lids_classic(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
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
		// WINK keeps a visible closed eye even when per-eye geometry is disabled.
		case WINK: strip(gfx, b, b.h * (b.left ? 0 : 85) / 100, false, cfg.bgColor); break;
		default: break;
	}
}

/** @brief Render eyelid masks for the extended expression set. @param gfx Canvas. @param cfg Config. @param b Eye. */
static void lids_extra(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	switch (cfg.expression) {
		case EXCITED: strip(gfx, b, b.h / 6, true, cfg.bgColor); break;
		case TERRIFIED: strip(gfx, b, b.h / 8, false, cfg.bgColor); break;
		case DISGUSTED: slope(gfx, cfg, b, true, b.h / 5);
			strip(gfx, b, b.h / 6, true, cfg.bgColor); break;
		case SHY: strip(gfx, b, b.h / 4, true, cfg.bgColor); break;
		case PROUD: strip(gfx, b, b.h * (b.left ? 30 : 40) / 100, false, cfg.bgColor); break;
		case TIRED: strip(gfx, b, b.h / 3, false, cfg.bgColor);
			strip(gfx, b, b.h / 4, true, cfg.bgColor); break;
		case GRIN: strip(gfx, b, b.h / 2, true, cfg.bgColor); break;
		case PANICKED: strip(gfx, b, b.h / 8, false, cfg.bgColor);
			strip(gfx, b, b.h / 8, true, cfg.bgColor); break;
		case GLUM: slope(gfx, cfg, b, false, b.h / 4);
			strip(gfx, b, b.h / 6, false, cfg.bgColor); break;
		case SMUG: strip(gfx, b, b.h * (b.left ? 20 : 45) / 100, false, cfg.bgColor); break;
		case ZOMBIE: strip(gfx, b, b.h * (b.left ? 40 : 10) / 100, false, cfg.bgColor); break;
		case CONFUSED: slope(gfx, cfg, b, false, b.h / 5); break;
		case CURIOUS: case WONDERING: strip(gfx, b, b.h / 6, false, cfg.bgColor); break;
		case CRYING: slope(gfx, cfg, b, false, b.h / 3); break;
		default: break;
	}
}

/** @brief Render eyelid masks for the newest expression set. @param gfx Canvas. @param cfg Config. @param b Eye. */
static void lids_new(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	switch (cfg.expression) {
		case MILD_HAPPY: strip(gfx, b, b.h / 5, true, cfg.bgColor); break;
		case SMIRK: strip(gfx, b, b.h * (b.left ? 45 : 25) / 100, false, cfg.bgColor); break;
		case POUT: strip(gfx, b, b.h / 8, false, cfg.bgColor);
			strip(gfx, b, b.h / 8, true, cfg.bgColor); break;
		case KISS: strip(gfx, b, b.h / 4, false, cfg.bgColor);
			strip(gfx, b, b.h / 4, true, cfg.bgColor); break;
		case NERVOUS: slope(gfx, cfg, b, false, b.h / 6);
			strip(gfx, b, b.h / 8, false, cfg.bgColor); break;
		case SHOCKED: strip(gfx, b, b.h / 12, false, cfg.bgColor); break;
		case MELTING: slope(gfx, cfg, b, false, b.h / 4); break;
		case LAZY: strip(gfx, b, b.h / 3, false, cfg.bgColor);
			strip(gfx, b, b.h / 6, true, cfg.bgColor); break;
		case DOZE: strip(gfx, b, b.h / 2, false, cfg.bgColor);
			strip(gfx, b, b.h / 4, true, cfg.bgColor); break;
		case GROGGY: strip(gfx, b, b.h * (b.left ? 15 : 45) / 100, false, cfg.bgColor); break;
		case DAYDREAM: strip(gfx, b, b.h / 6, false, cfg.bgColor); break;
		case GRUMPY: slope(gfx, cfg, b, true, b.h / 5);
			strip(gfx, b, b.h / 6, false, cfg.bgColor); break;
		case SASSY: strip(gfx, b, b.h * (b.left ? 35 : 12) / 100, false, cfg.bgColor); break;
		case CHEEKY: strip(gfx, b, b.h / 3, true, cfg.bgColor);
			slope(gfx, cfg, b, true, b.h / 6); break;
		case FLIRT: strip(gfx, b, b.h / 4, false, cfg.bgColor);
			strip(gfx, b, b.h / 6, true, cfg.bgColor); break;
		case ZEN: strip(gfx, b, b.h * 40 / 100, false, cfg.bgColor);
			strip(gfx, b, b.h * 40 / 100, true, cfg.bgColor); break;
		case MERCIFUL: slope(gfx, cfg, b, false, b.h / 4);
			strip(gfx, b, b.h / 8, false, cfg.bgColor); break;
		case MANIC: strip(gfx, b, b.h / 10, false, cfg.bgColor); break;
		case SIDE_EYE: strip(gfx, b, b.h / 6, false, cfg.bgColor); break;
		case BEAMING: strip(gfx, b, b.h * 45 / 100, true, cfg.bgColor);
			strip(gfx, b, b.h / 10, false, cfg.bgColor); break;
		default: break;
	}
}

/** @brief Render emotional eyelids, not pupils, for all eye styles. @param gfx Canvas. @param cfg Config. @param b Eye. */
void eye_lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	lids_classic(gfx, cfg, b);
	lids_extra(gfx, cfg, b);
	lids_new(gfx, cfg, b);
	strip(gfx, b, cfg.lidTop, false, cfg.bgColor);
}
