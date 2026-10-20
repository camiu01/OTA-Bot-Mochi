/**
 * @file mini_widgets.h
 * @brief Fixed-size Pong simulation and bounded clock/Pong drawing shared by eyes and desk.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Arduino_GFX_Library.h>
#include <stdint.h>
#include <math.h>

/** @brief Portable 240x160 court; left paddle is controlled by taps or demo AI. */
struct PongGame {
	float x = 120, y = 80, vx = 100, vy = 60, left = 80, right = 80;
	uint8_t player = 0, opponent = 0;
	uint32_t last = 0;
	bool started = false;

	/** @brief Reset a session without writing preferences. @param now Time. */
	void reset(uint32_t now) { *this = PongGame{}; last = now; started = true; }
	/** @brief Clamp a paddle center to the court. @param y Desired center. */
	void move(float y) { left = y < 16 ? 16 : (y > 144 ? 144 : y); }
	/** @brief Track the ball with bounded speed. @param paddle Center. @param speed Travel this step. */
	void follow(float &paddle, float speed) {
		float delta = y - paddle;
		paddle += delta < -speed ? -speed : (delta > speed ? speed : delta);
		paddle = paddle < 16 ? 16 : (paddle > 144 ? 144 : paddle);
	}
	/** @brief Advance at most 100 ms, including unsigned clock rollover. @param now Time. @param demo Both paddles automatic. */
	void tick(uint32_t now, bool demo) {
		if (!started) { reset(now); return; }
		uint32_t elapsed = now - last;
		last = now;
		if (elapsed > 100) { elapsed = 100; }
		while (elapsed) {
			uint32_t ms = elapsed > 10 ? 10 : elapsed;
			elapsed -= ms;
			step(ms / 1000.0f, demo);
		}
	}
	/** @brief Advance one bounded physics step. @param dt Seconds, at most 0.01. @param demo Automatic left paddle. */
	void step(float dt, bool demo) {
		if (demo) { follow(left, 120 * dt); }
		follow(right, 72 * dt);
		float old_x = x;
		x += vx * dt; y += vy * dt;
		if (y < 3) { y = 6 - y; vy = fabsf(vy); }
		if (y > 157) { y = 314 - y; vy = -fabsf(vy); }
		if (vx < 0 && old_x >= 15 && x <= 15 && fabsf(y - left) <= 19) {
			x = 30 - x; vx = fabsf(vx); vy = (y - left) * 4;
			if (fabsf(vy) < 30) { vy = y < 80 ? -40 : 40; }
		}
		if (vx > 0 && old_x <= 225 && x >= 225 && fabsf(y - right) <= 19) {
			x = 450 - x; vx = -fabsf(vx); vy = (y - right) * 4;
			if (fabsf(vy) < 30) { vy = y < 80 ? -40 : 40; }
		}
		if (x >= 3 && x <= 237) { return; }
		bool missed = x < 3;
		if (missed) { opponent = uint8_t((opponent + 1) % 100); }
		else { player = uint8_t((player + 1) % 100); }
		x = 120; y = 80; vx = missed ? -100 : 100; vy = 60;
	}
};

/** @brief Paint a court inside a supplied rectangle; caller owns clearing. @param gfx Canvas. @param game State. @param x Left. @param y Top. @param w Width. @param h Height. @param ink Color. */
inline void mini_pong_draw(Arduino_GFX *gfx, const PongGame &game, int16_t x, int16_t y,
		int16_t w, int16_t h, uint16_t ink) {
	for (int16_t i = 4; i < 156; i += 12) {
		gfx->fillRect(x + w / 2, y + i * h / 160, 1, h * 5 / 160 + 1, ink);
	}
	gfx->fillRect(x + w * 8 / 240, y + int16_t(game.left - 16) * h / 160,
		w * 4 / 240 + 1, h * 32 / 160, ink);
	gfx->fillRect(x + w * 228 / 240, y + int16_t(game.right - 16) * h / 160,
		w * 4 / 240 + 1, h * 32 / 160, ink);
	int16_t ball = w * 5 / 240;
	if (ball < 2) { ball = 2; }
	gfx->fillRect(x + int16_t(game.x) * w / 240 - ball / 2,
		y + int16_t(game.y) * h / 160 - ball / 2, ball, ball, ink);
}

/** @brief Draw one hand wholly inside a dial. @param gfx Canvas. @param x Center. @param y Center. @param r Length. @param turns Clockwise turns from twelve. @param color Ink. */
inline void mini_clock_hand(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t r, float turns, uint16_t color) {
	float angle = turns * 6.2831853f - 1.5707963f;
	gfx->drawLine(x, y, int16_t(x + cosf(angle) * r), int16_t(y + sinf(angle) * r), color);
}

/** @brief Render hour/minute/second hands without a time-source dependency. @param gfx Canvas. @param x Center. @param y Center. @param r Radius. @param seconds Seconds since midnight or decorative uptime. @param ink Dial/hands. @param accent Seconds hand. */
inline void mini_clock_draw(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t r,
		uint32_t seconds, uint16_t ink, uint16_t accent) {
	if (r < 3) { return; }
	gfx->drawCircle(x, y, r, ink);
	for (uint8_t i = 0; i < 12; ++i) {
		float a = i * 0.5235988f;
		int16_t tx = int16_t(x + sinf(a) * (r - 2));
		int16_t ty = int16_t(y - cosf(a) * (r - 2));
		gfx->fillRect(tx, ty, 1, 1, ink);
	}
	mini_clock_hand(gfx, x, y, r / 2, (seconds % 43200) / 43200.0f, ink);
	mini_clock_hand(gfx, x, y, r * 3 / 4, (seconds % 3600) / 3600.0f, ink);
	mini_clock_hand(gfx, x, y, r - 2, (seconds % 60) / 60.0f, accent);
	gfx->fillCircle(x, y, 1, ink);
}
