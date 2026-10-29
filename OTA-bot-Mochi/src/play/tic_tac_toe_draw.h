/**
 * @file tic_tac_toe_draw.h
 * @brief Shared scalable grid, X/O marks and winning stroke for game and eye demo.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "tic_tac_toe.h"
#include <Arduino_GFX_Library.h>

/** @brief Draw a square board with distinct geometric marks, never font glyphs. */
inline void tic_tac_toe_draw(Arduino_GFX *gfx, const TicTacToe &game,
		int16_t x, int16_t y, int16_t size, uint16_t grid, uint16_t cross, uint16_t ring) {
	int16_t cell = size / 3, stroke = size / 90 + 1;
	for (uint8_t i = 1; i < 3; ++i) {
		gfx->fillRect(x + i * cell, y, stroke, cell * 3, grid);
		gfx->fillRect(x, y + i * cell, cell * 3, stroke, grid);
	}
	for (uint8_t i = 0; i < 9; ++i) {
		int16_t cx = x + (i % 3) * cell + cell / 2, cy = y + (i / 3) * cell + cell / 2;
		int16_t r = cell / 4;
		for (int16_t s = 0; s < stroke; ++s) {
			if (game.cells[i] == 1) {
				gfx->drawLine(cx - r, cy - r + s, cx + r, cy + r + s, cross);
				gfx->drawLine(cx - r, cy + r + s, cx + r, cy - r + s, cross);
			} else if (game.cells[i] == 2) { gfx->drawCircle(cx, cy, r - s, ring); }
		}
	}
	int8_t line = game.winning_line();
	if (line < 0) { return; }
	uint8_t first = TicTacToe::line_cell(uint8_t(line), 0), last = TicTacToe::line_cell(uint8_t(line), 2);
	for (int16_t s = 0; s < stroke; ++s) {
		gfx->drawLine(x + first % 3 * cell + cell / 2, y + first / 3 * cell + cell / 2 + s,
			x + last % 3 * cell + cell / 2, y + last / 3 * cell + cell / 2 + s,
			game.result() == 1 ? cross : ring);
	}
}
