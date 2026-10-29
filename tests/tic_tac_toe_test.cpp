/**
 * @file tic_tac_toe_test.cpp
 * @brief Legal turns, all winning lines, tactical replies, draws and timer rollover.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/play/tic_tac_toe.h"
#include <cassert>
#include <iostream>

/** @brief Explore every player continuation against the deterministic opponent. */
static void explore(TicTacToe game, uint32_t now) {
	if (game.result()) {
		assert(!game.play(0, now));
		auto finished = game; game.tick(now + 1000);
		for (uint8_t i = 0; i < 9; ++i) { assert(game.cells[i] == finished.cells[i]); }
		return;
	}
	for (uint8_t cell = 0; cell < 9; ++cell) {
		if (game.cells[cell]) { continue; }
		auto next = game;
		assert(next.play(cell, now));
		assert(!next.play(9, now) && !next.play(cell, now));
		if (next.thinking) {
			auto waiting = next;
			next.tick(now + 449);
			for (uint8_t i = 0; i < 9; ++i) { assert(next.cells[i] == waiting.cells[i]); }
			next.tick(now + 450);
			unsigned replies = 0;
			for (uint8_t i = 0; i < 9; ++i) {
				if (waiting.cells[i]) { assert(next.cells[i] == waiting.cells[i]); }
				else if (next.cells[i]) { assert(next.cells[i] == 2); ++replies; }
			}
			assert(replies == 1 && !next.thinking);
		}
		explore(next, now + 1000);
	}
}

int main() {
	for (uint8_t line = 0; line < 8; ++line) {
		for (uint8_t mark : {1, 2}) {
			TicTacToe game;
			for (uint8_t step = 0; step < 3; ++step) { game.cells[TicTacToe::line_cell(line, step)] = mark; }
			assert(game.result() == mark && game.winning_line() == line);
			game.cells[TicTacToe::line_cell(line, 2)] = 0;
			game.thinking = true; game.tick(450);
			assert(game.cells[TicTacToe::line_cell(line, 2)] == 2); // Win or block.
		}
	}
	TicTacToe draw{{1, 2, 1, 1, 2, 2, 2, 1, 1}};
	assert(draw.result() == 3 && !draw.play(8, 0));
	draw.reset(); assert(draw.result() == 0 && !draw.thinking);
	TicTacToe win{{2, 2, 0, 1, 1, 0, 0, 0, 1}, true};
	win.tick(450); assert(win.cells[2] == 2 && win.result() == 2);
	explore(TicTacToe{}, 0); explore(TicTacToe{}, UINT32_MAX - 100);
	std::cout << "Tris legal games, tactics, eight winning lines, draws and rollover passed\n";
}
