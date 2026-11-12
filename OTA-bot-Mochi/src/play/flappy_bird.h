/**
 * @file flappy_bird.h
 * @brief Fixed-step, bounded Flappy Bird physics in a 216x160 court.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Arduino.h>

struct FlappyBird {
	struct Pipe { float x; uint8_t gap; bool scored; };
	Pipe pipes[2] = {{220, 80, false}, {350, 80, false}};
	float y = 80, velocity = 0;
	uint16_t score = 0, best = 0;
	uint32_t last = 0, remainder = 0;
	bool running = false, dead = false;

	/** @brief Restart locally; the session best survives. */
	void reset(uint32_t now) {
		uint16_t record = best;
		*this = FlappyBird{}; best = record; last = now;
		for (auto &pipe : pipes) { pipe.gap = uint8_t(random(44, 117)); }
	}
	/** @brief Start/restart or give the bird an upward impulse. */
	void flap(uint32_t now) {
		if (!running) { reset(now); running = true; }
		velocity = -2.7f;
	}
	/** @brief Advance one 10ms step, checking overlap before scoring. */
	void step() {
		velocity += 0.095f; y += velocity;
		if (y < 6 || y > 154) { running = false; dead = true; return; }
		for (auto &pipe : pipes) {
			pipe.x -= 0.75f;
			if (pipe.x < 52 && pipe.x + 28 > 40 && (y - 6 < pipe.gap - 32 || y + 6 > pipe.gap + 32)) {
				running = false; dead = true; return;
			}
			if (!pipe.scored && pipe.x + 28 <= 40) {
				pipe.scored = true;
				if (score < UINT16_MAX) { ++score; }
				if (score > best) { best = score; }
			}
		}
		for (uint8_t i = 0; i < 2; ++i) {
			if (pipes[i].x + 28 < 0) {
				pipes[i] = {pipes[1 - i].x + 130, uint8_t(random(44, 117)), false};
			}
		}
	}
	/** @brief Bound catch-up and retain substep time for a steady game speed. */
	void tick(uint32_t now) {
		uint32_t elapsed = uint32_t(now - last); last = now;
		if (!running) { remainder = 0; return; }
		remainder += elapsed > 100 ? 100 : elapsed;
		while (remainder >= 10 && running) { remainder -= 10; step(); }
	}
	/** @brief Resume without advancing off-page or overlay time. */
	void resume(uint32_t now) { last = now; remainder = 0; }
};
/** @brief Access the session-only game. */
inline FlappyBird &desk_flappy() { static FlappyBird game; return game; }
