/**
 * @file tic_tac_toe.h
 * @brief Eyes become the first X/O pieces of a miniature match, like the Pong morph.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"
#include "../face_backgrounds.h"
#include "../../play/tic_tac_toe_draw.h"

namespace face_defs { namespace tic_tac_toe_face {
/** @brief Carve four growing wedges from a style body until it becomes a thick X. */
inline void carve_cross(Arduino_GFX *gfx, const EyeBox &b, float a, uint16_t bg) {
	int16_t cx = b.x + b.w / 2, cy = b.y + b.h / 2;
	int16_t dx = int16_t(b.w * 0.42f * a), dy = int16_t(b.h * 0.42f * a);
	if (dx < 1 || dy < 1) { return; }
	gfx->fillTriangle(cx - dx, b.y, cx + dx, b.y, cx, b.y + dy, bg);
	gfx->fillTriangle(cx - dx, b.y + b.h - 1, cx + dx, b.y + b.h - 1, cx, b.y + b.h - 1 - dy, bg);
	gfx->fillTriangle(b.x, cy - dy, b.x, cy + dy, b.x + dx, cy, bg);
	gfx->fillTriangle(b.x + b.w - 1, cy - dy, b.x + b.w - 1, cy + dy, b.x + b.w - 1 - dx, cy, bg);
}
/** @brief Move one eye into its first board piece, retaining visible ink throughout. */
inline void piece(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &eye,
		const EyeBox &target, float a) {
	EyeBox b = eye_scene_box(eye, target, a);
	EyeConfig ink = cfg;
	ink.fgColor = eye_mix(eye.left ? eye_rgb(70, 220, 235) : eye_rgb(255, 185, 80),
		cfg.fgColor, uint8_t(a * 100));
	eye_scene_body(gfx, ink, b, a);
	if (eye.left) { carve_cross(gfx, b, a, cfg.bgColor); }
	else if (a > 0) {
		int16_t r = int16_t((b.w < b.h ? b.w : b.h) * 0.42f * a);
		if (r > 0) { gfx->fillCircle(b.x + b.w / 2, b.y + b.h / 2, r, cfg.bgColor); }
	}
}
/** @brief Run a twelve-second elapsed-time match with direct eye-to-piece morphs. */
inline bool scene(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float a) {
	int16_t side = int16_t((cfg.screenW < cfg.screenH ? cfg.screenW : cfg.screenH) * 3 / 4);
	int16_t cell = side / 3, mark = cell / 2;
	int16_t x = (cfg.screenW - cell * 3) / 2, y = (cfg.screenH - cell * 3) / 2;
	TicTacToe demo;
	uint32_t phase = elapsed % 12000;
	if (phase >= 4000) { demo.cells[4] = 1; }
	if (phase >= 5800) { demo.cells[1] = 2; }
	if (phase >= 7600) { demo.cells[8] = 1; }
	tic_tac_toe_draw(gfx, demo, x, y, cell * 3,
		eye_mix(cfg.fgColor, cfg.bgColor, uint8_t(a * 45)),
		eye_mix(eye_rgb(70, 220, 235), cfg.bgColor, uint8_t(a * 100)),
		eye_mix(eye_rgb(255, 185, 80), cfg.bgColor, uint8_t(a * 100)));
	EyeBox targets[] = {
		{int16_t(x + (cell - mark) / 2), int16_t(y + (cell - mark) / 2), mark, mark, 0, true},
		{int16_t(x + 2 * cell + (cell - mark) / 2), int16_t(y + (cell - mark) / 2), mark, mark, int16_t(mark / 2), false}
	};
	piece(gfx, cfg, layout.left, targets[0], a);
	piece(gfx, cfg, layout.right, targets[1], a);
	if (phase >= 7600) {
		uint16_t ink = eye_mix(eye_rgb(70, 220, 235), cfg.bgColor, uint8_t(a * 100));
		gfx->drawLine(x + cell / 2, y + cell / 2, x + cell * 5 / 2, y + cell * 5 / 2, ink);
	}
	return true;
}
/** @brief Register a Pong-style twelve-second scene with no persistent state. */
inline constexpr FacePreset preset = [] {
	FacePreset p; p.name = "tic_tac_toe";
	p.fullScene = true; p.scene = scene;
	p.bounds = face_backgrounds::full_bounds;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid tic-tac-toe face");
} }
