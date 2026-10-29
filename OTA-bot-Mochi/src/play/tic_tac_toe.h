/**
 * @file tic_tac_toe.h
 * @brief Bounded session-only tic-tac-toe; player X faces a tactical Mochi O.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

struct TicTacToe {
	uint8_t cells[9] = {};
	bool thinking = false;
	uint32_t since = 0;

	/** @brief Return the winning line index, or -1 while no player has won. */
	int8_t winning_line() const {
		for (uint8_t line = 0; line < 8; ++line) {
			uint8_t a = line_cell(line, 0), b = line_cell(line, 1), c = line_cell(line, 2);
			if (cells[a] && cells[a] == cells[b] && cells[a] == cells[c]) { return int8_t(line); }
		}
		return -1;
	}
	/** @brief Map the eight rows/columns/diagonals to board cells. */
	static uint8_t line_cell(uint8_t line, uint8_t step) {
		return line < 3 ? line * 3 + step : (line < 6 ? line - 3 + step * 3 :
			(line == 6 ? step * 4 : 2 + step * 2));
	}
	/** @brief Return 0 ongoing, 1 X wins, 2 O wins, or 3 draw. */
	uint8_t result() const {
		int8_t line = winning_line();
		if (line >= 0) { return cells[line_cell(uint8_t(line), 0)]; }
		for (uint8_t cell : cells) { if (!cell) { return 0; } }
		return 3;
	}
	/** @brief Clear the board without writing preferences. */
	void reset() { *this = TicTacToe{}; }
	/** @brief Accept one empty cell on the player's turn only. */
	bool play(uint8_t cell, uint32_t now) {
		if (cell >= 9 || cells[cell] || thinking || result()) { return false; }
		cells[cell] = 1;
		thinking = !result(); since = now;
		return true;
	}
	/** @brief Find an immediate win/block without changing the board. */
	int8_t finishing(uint8_t mark) const {
		for (uint8_t line = 0; line < 8; ++line) {
			uint8_t count = 0; int8_t empty = -1;
			for (uint8_t step = 0; step < 3; ++step) {
				uint8_t cell = line_cell(line, step);
				if (cells[cell] == mark) { ++count; }
				else if (!cells[cell]) { empty = int8_t(cell); }
			}
			if (count == 2 && empty >= 0) { return empty; }
		}
		return -1;
	}
	/** @brief Make a legal tactical reply after a non-blocking thinking pause. */
	void tick(uint32_t now) {
		if (!thinking || uint32_t(now - since) < 450) { return; }
		thinking = false;
		if (result()) { return; }
		int8_t move = finishing(2);
		if (move < 0) { move = finishing(1); }
		static constexpr uint8_t order[] = {4, 0, 2, 6, 8, 1, 3, 5, 7};
		for (uint8_t cell : order) {
			if (move < 0 && !cells[cell]) { move = int8_t(cell); }
		}
		if (move >= 0) { cells[move] = 2; }
	}
};
