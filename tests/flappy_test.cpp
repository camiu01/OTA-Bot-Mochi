/**
 * @file flappy_test.cpp
 * @brief Fixed-step physics, collision, score and pause regressions.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/play/flappy_bird.h"
#include <assert.h>
#include <iostream>

int main() {
	FlappyBird game, regular, delayed;
	regular.flap(0); delayed.flap(0);
	for (uint32_t now = 10; now <= 100; now += 10) { regular.tick(now); }
	delayed.tick(100);
	assert(regular.y == delayed.y && regular.pipes[0].x == delayed.pipes[0].x);
	delayed.resume(90000); float y = delayed.y;
	delayed.tick(90000); assert(delayed.y == y);
	delayed.tick(190000); assert(delayed.pipes[0].x == regular.pipes[0].x - 7.5f);
	for (float height : {10.0f, 150.0f}) {
		game.reset(0); game.running = true; game.y = height;
		game.pipes[0] = {51, 80, false}; game.tick(10);
		assert(game.dead && !game.running && game.score == 0);
	}
	game.reset(0); game.running = true; game.pipes[0] = {12, 80, false};
	game.tick(10); assert(game.score == 1 && game.best == 1);
	game.tick(20); assert(game.score == 1);
	game.pipes[0].x = -29; game.tick(30);
	assert(game.pipes[0].x > game.pipes[1].x && !game.pipes[0].scored);
	assert(game.pipes[0].gap >= 44 && game.pipes[0].gap <= 116);
	game.y = 154; game.velocity = 2; game.tick(40);
	assert(game.dead && !game.running);
	game.flap(50); assert(game.running && !game.dead && game.best == 1 && game.score == 0);
	game.y = 6; game.velocity = -2; game.tick(60); assert(game.dead);
	game.flap(UINT32_MAX - 10); game.tick(9);
	assert(game.running && game.y < 80);
	game.score = game.best = UINT16_MAX; game.y = 80;
	game.pipes[0] = {12, 80, false}; game.tick(19);
	assert(game.score == UINT16_MAX);
	std::cout << "Flappy Bird physics, pause, collisions and scoring passed\n";
}
