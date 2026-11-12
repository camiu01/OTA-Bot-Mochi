/**
 * @file context_flappy.h
 * @brief Scaled, tap-controlled Flappy Bird with original drawings.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../play/flappy_bird.h"
#include "ui_theme.h"
#include "i18n.h"

/** @brief Draw clipped pipes, score/session best, bird and start/retry hints. */
inline void desk_flappy_draw(Arduino_GFX *gfx, uint32_t now) {
	auto &game = desk_flappy(); game.tick(now);
	ui_label(gfx, String(game.score) + " / " + String(game.best), ui_s(16), ui_s(58), ui_s(208), 2, UI_ACCENT);
	gfx->fillRect(ui_s(12), ui_s(82), ui_s(216), ui_s(160), UI_CARD);
	for (const auto &pipe : game.pipes) {
		int16_t left = int16_t(pipe.x), right = left + 28;
		if (left < 0) { left = 0; }
		if (right > 216) { right = 216; }
		if (right <= left) { continue; }
		gfx->fillRect(ui_s(12 + left), ui_s(82), ui_s(right - left), ui_s(pipe.gap - 32), UI_ACCENT);
		gfx->fillRect(ui_s(12 + left), ui_s(82 + pipe.gap + 32), ui_s(right - left), ui_s(160 - pipe.gap - 32), UI_ACCENT);
	}
	int16_t y = int16_t(game.y);
	if (y < 6) { y = 6; }
	if (y > 154) { y = 154; }
	gfx->fillCircle(ui_s(58), ui_s(82 + y), ui_s(6), UI_WARM);
	gfx->fillCircle(ui_s(60), ui_s(80 + y), ui_s(2), UI_BG);
	gfx->fillTriangle(ui_s(63), ui_s(82 + y), ui_s(69), ui_s(85 + y), ui_s(63), ui_s(86 + y), UI_TEXT);
	ui_label(gfx, tr(game.dead ? Text::FlappyRetry : (game.running ? Text::FlappyTap : Text::FlappyStart)),
		ui_s(12), ui_s(250), ui_s(216), 1, UI_TEXT);
}
/** @brief Accept released court/action taps, ignoring the header and margins. */
inline bool desk_flappy_tap(int16_t x, int16_t y, uint32_t now) {
	if (x < ui_s(12) || x >= ui_s(228) || y < ui_s(82) || y >= ui_s(260)) { return false; }
	desk_flappy().flap(now); return true;
}
