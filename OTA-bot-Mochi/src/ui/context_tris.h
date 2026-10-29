/**
 * @file context_tris.h
 * @brief Parked-use tic-tac-toe screen and matching scaled touch targets.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../play/tic_tac_toe_draw.h"
#include "ui_theme.h"
#include "i18n.h"

/** @brief Access the RAM-only game, independent of the eye animation. */
inline TicTacToe &desk_tris() { static TicTacToe game; return game; }

/** @brief Draw the board, current turn/result and a full-width restart action. */
inline void desk_tris_draw(Arduino_GFX *gfx, uint32_t now) {
	TicTacToe &game = desk_tris(); game.tick(now);
	static constexpr Text outcomes[] = {Text::TrisTurn, Text::TrisWin, Text::TrisLose, Text::TrisDraw};
	ui_label(gfx, tr(game.thinking ? Text::TrisThinking : outcomes[game.result()]),
		ui_s(18), ui_s(58), ui_s(204), 1, UI_ACCENT);
	tic_tac_toe_draw(gfx, game, ui_s(48), ui_s(78), ui_s(144),
		UI_MUTED, UI_TEXT, UI_ACCENT);
	gfx->fillRoundRect(ui_s(6), ui_s(230), ui_s(228), ui_s(30), ui_s(6), UI_CARD);
	ui_label(gfx, tr(Text::TrisNew), ui_s(18), ui_s(241), ui_s(204), 1, UI_TEXT);
}

/** @brief Accept a released board tap, ignoring the header, margins and occupied cells. */
inline void desk_tris_tap(int16_t x, int16_t y, uint32_t now) {
	if (x >= ui_s(6) && x < ui_s(234) && y >= ui_s(230) && y < ui_s(260)) {
		desk_tris().reset(); return;
	}
	for (uint8_t i = 0; i < 9; ++i) {
		int16_t left = ui_s(48) + (i % 3) * (ui_s(144) / 3);
		int16_t top = ui_s(78) + (i / 3) * (ui_s(144) / 3);
		if (x >= left && x < left + ui_s(144) / 3 && y >= top && y < top + ui_s(144) / 3) {
			desk_tris().play(i, now); return;
		}
	}
}
