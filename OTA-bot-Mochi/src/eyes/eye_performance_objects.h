#pragma once
#include "eye_performances.h"
#include "eye_scenes.h"

/** @brief One centered nigiri target for both eyes. @param cfg Config. @return Rice silhouette. */
inline EyeBox eye_sushi_target(const EyeConfig &cfg) {
	int16_t w = cfg.screenW * 3 / 5;
	if (w > cfg.screenH) { w = cfg.screenH; }
	int16_t h = w / 2;
	return {int16_t((cfg.screenW - w) / 2), int16_t((cfg.screenH - h) / 2), w, h, int16_t(h / 3), true};
}

/** @brief Stripe the salmon after both rice bodies have merged, avoiding an erased seam. @param gfx Canvas. @param cfg Config. @param rice Current silhouette. @param a Strength. */
inline void eye_sushi_topping(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &rice, float a) {
	uint16_t salmon = eye_mix(eye_rgb(245, 135, 110), cfg.fgColor, uint8_t(a * 100));
	int16_t h = int16_t(rice.h * 0.52f * a), pad = rice.w / 12;
	if (h < 2) { return; }
	gfx->fillRoundRect(rice.x + pad, rice.y, rice.w - 2 * pad, h, h / 3, salmon);
	uint16_t stripe = eye_mix(eye_rgb(255, 210, 175), salmon, uint8_t(a * 100));
	for (int16_t i = 1; i < 6; ++i) {
		int16_t x = rice.x + pad + (rice.w - 2 * pad) * i / 7;
		gfx->drawLine(x, rice.y + 2, x + rice.w / 12, rice.y + h - 2, stripe);
	}
}

/** @brief Merge both eyes into one nigiri, then split along the same path. @param gfx Canvas. @param cfg Config. @param layout Eyes. @param elapsed Entry time. @param a Strength. */
inline void eye_performance_sushi(Arduino_GFX *gfx, EyeConfig cfg, const EyeLayout &layout,
		uint32_t elapsed, float a) {
	EyeBox target = eye_sushi_target(cfg);
	EyeBox rice[] = {eye_scene_box(layout.left, target, a), eye_scene_box(layout.right, target, a)};
	cfg.fgColor = eye_performance_ink(cfg, true, elapsed);
	for (const EyeBox &b : rice) { eye_scene_body(gfx, cfg, b, a); }
	// At full merge paint the single topping once, not a pair of overlapping nigiri.
	eye_sushi_topping(gfx, cfg, rice[0], a);
	if (a < 1) { eye_sushi_topping(gfx, cfg, rice[1], a); }
}

/** @brief Rotating compressor viewed at an inward-facing angle. @param gfx Canvas. @param cfg Config. @param cx Center X. @param cy Center Y. @param rx Horizontal radius. @param ry Vertical radius. @param angle Rotation. @param a Strength. */
inline void eye_turbo_impeller(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cx,
		int16_t cy, int16_t rx, int16_t ry, float angle, float a) {
	uint16_t face = eye_mix(eye_rgb(175, 205, 225), cfg.bgColor, uint8_t(a * 100));
	uint16_t shade = eye_mix(eye_rgb(80, 130, 165), cfg.bgColor, uint8_t(a * 100));
	for (uint8_t i = 0; i < 8; ++i) {
		float t = angle + i * 0.785398163f;
		int16_t ix = int16_t(cx + cosf(t) * rx * 0.2f), iy = int16_t(cy + sinf(t) * ry * 0.2f);
		int16_t ox = int16_t(cx + cosf(t + 0.42f) * rx), oy = int16_t(cy + sinf(t + 0.42f) * ry);
		int16_t tx = int16_t(cx + cosf(t + 0.75f) * rx * 0.9f), ty = int16_t(cy + sinf(t + 0.75f) * ry * 0.9f);
		gfx->fillTriangle(ix, iy, ox, oy, tx, ty, face);
		gfx->fillTriangle(ix, iy, tx, ty, int16_t(cx + cosf(t + 0.5f) * rx * 0.42f),
			int16_t(cy + sinf(t + 0.5f) * ry * 0.42f), shade);
	}
	gfx->fillEllipse(cx, cy, rx / 5 + 1, ry / 5 + 1,
		eye_mix(eye_rgb(225, 175, 105), cfg.bgColor, uint8_t(a * 100)));
}

/** @brief Large turbo silhouette independent of the starting eye style. @param cfg Config. @param left Side. @return Housing target. */
inline EyeBox eye_turbo_target(const EyeConfig &cfg, bool left) {
	int16_t r = cfg.screenW / 6;
	if (r > cfg.screenH / 4) { r = cfg.screenH / 4; }
	int16_t cx = cfg.screenW * (left ? 1 : 3) / 4;
	return {int16_t(cx - r), int16_t(cfg.screenH / 2 - r), int16_t(r * 2), int16_t(r * 2), r, left};
}

/** @brief Short exhaust bursts rise outside each housing and fade without particle storage. @param gfx Canvas. @param cfg Config. @param cx Center X. @param cy Center Y. @param r Housing radius. @param left Side. @param elapsed Entry time. @param a Strength. */
inline void eye_turbo_smoke(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cx, int16_t cy,
		int16_t r, bool left, uint32_t elapsed, float a) {
	int16_t direction = left ? -1 : 1, pipeH = int16_t(r * 0.32f * a), pipeW = int16_t(r * 0.6f * a);
	int16_t end = cx + direction * int16_t(r * 1.35f * a);
	if (pipeH > 0 && pipeW > 0) {
		gfx->fillRect(direction < 0 ? end : end - pipeW, cy + r / 4, pipeW, pipeH,
			eye_mix(eye_rgb(100, 115, 130), cfg.bgColor, uint8_t(a * 100)));
	}
	uint32_t phase = elapsed % 1500;
	for (uint8_t i = 0; i < 3; ++i) {
		if (phase < i * 100u || phase >= i * 100u + 900u) { continue; }
		float t = (phase - i * 100u) / 900.0f;
		int16_t size = int16_t(r * (0.1f + 0.24f * t) * a);
		if (size < 1) { continue; }
		int16_t x = eye_limit(int16_t(end + direction * t * r * 0.3f), size, cfg.screenW - size - 1);
		int16_t y = int16_t(cy + r / 4 - t * r * 1.6f);
		uint16_t ink = eye_mix(eye_rgb(175, 185, 195), cfg.bgColor, uint8_t(a * (1 - t) * 70));
		gfx->fillCircle(x, y, size, ink);
		gfx->fillCircle(x - direction * size / 2, y - size / 2, size * 3 / 4, ink);
	}
}

/** @brief Large mirrored car turbos point their offset intakes toward the center and puff smoke outside. @param gfx Canvas. @param cfg Config. @param b Eye. @param elapsed Entry time. @param a Strength. */
inline void eye_performance_turbo(Arduino_GFX *gfx, EyeConfig cfg, const EyeBox &b,
		uint32_t elapsed, float a) {
	EyeBox target = eye_turbo_target(cfg, b.left), housing = eye_scene_box(b, target, a);
	int16_t cx = housing.x + housing.w / 2, cy = housing.y + housing.h / 2, r = target.radius;
	int16_t direction = b.left ? 1 : -1;
	eye_turbo_smoke(gfx, cfg, cx, cy, r, b.left, elapsed, a);
	uint16_t steel = eye_rgb(145, 165, 185);
	uint16_t casing = eye_mix(steel, cfg.bgColor, uint8_t(a * 100));
	int16_t tubeW = int16_t(r * 1.3f * a), tubeH = int16_t(r * 0.48f * a);
	if (tubeW > 2 && tubeH > 2) {
		int16_t x = direction > 0 ? cx : cx - tubeW;
		gfx->fillRoundRect(x, cy - r, tubeW, tubeH, tubeH / 3, casing);
		gfx->fillRect(direction > 0 ? cx + tubeW - tubeH / 3 : x, cy - r - tubeH / 6, tubeH / 3 + 1,
			tubeH * 4 / 3, eye_mix(eye_rgb(205, 215, 225), cfg.bgColor, uint8_t(a * 100)));
	}
	cfg.fgColor = eye_mix(steel, cfg.fgColor, uint8_t(a * 100));
	eye_scene_body(gfx, cfg, housing, a);
	int16_t faceX = cx + direction * int16_t(r * 0.18f * a);
	int16_t rx = int16_t(r * 0.54f * a), ry = int16_t(r * 0.72f * a), rim = int16_t(a * 3);
	gfx->fillEllipse(faceX, cy, rx + rim, ry + rim,
		eye_mix(eye_rgb(70, 90, 110), cfg.fgColor, uint8_t(a * 100)));
	gfx->fillEllipse(faceX, cy, rx, ry, cfg.bgColor);
	eye_turbo_impeller(gfx, cfg, faceX, cy, rx, ry,
		(elapsed % 6000) * (elapsed % 6000) * 0.000003f * direction, a);
	uint16_t trim = eye_mix(eye_rgb(205, 220, 230), cfg.bgColor, uint8_t(a * 100));
	for (uint8_t i = 0; i < 4; ++i) {
		float t = 0.785398163f + i * 1.570796327f;
		gfx->fillCircle(int16_t(cx + cosf(t) * r * 0.9f), int16_t(cy + sinf(t) * r * 0.9f),
			int16_t((r / 18 + 1) * a), trim);
	}
}

/** @brief Fixed sky positions with slow independent twinkles; no allocations or random state. @param gfx Canvas. @param cfg Config. @param elapsed Entry time. @param a Strength. */
inline void eye_performance_stars(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a) {
	int16_t margin = cfg.screenW / 16 + 2;
	for (uint8_t i = 0; i < 28; ++i) {
		int16_t x = margin + ((i * 73 + 17) % 211) * (cfg.screenW - 2 * margin) / 211;
		int16_t y = margin + ((i * i * 11 + 7) % 101) * (cfg.screenH * 3 / 4 - margin) / 101;
		float light = 0.55f + 0.35f * sinf((elapsed % 6000) * 0.00314159265f + i * 1.7f);
		uint16_t tint = i % 3 ? eye_rgb(180, 210, 255) : eye_rgb(255, 220, 160);
		uint16_t ink = eye_mix(tint, cfg.bgColor, uint8_t(a * light * 100));
		int16_t r = i % 7 == 0 ? cfg.screenW / 120 + 1 : 0;
		gfx->fillRect(x - r, y, r * 2 + 1, 1, ink);
		gfx->fillRect(x, y - r, 1, r * 2 + 1, ink);
	}
}
