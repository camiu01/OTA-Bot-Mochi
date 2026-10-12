/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <stdint.h>

/** @brief Recognize one horizontal swipe per contact, independently of hardware. */
struct SwipeTracker {
	bool active = false;
	bool consumed = false;
	int16_t startX = 0;
	int16_t startY = 0;
	uint32_t startMs = 0;

	/**
	 * @brief Consume a touch sample and return the screen direction.
	 * @param touching True while a finger is down.
	 * @param x Horizontal coordinate in pixels.
	 * @param y Vertical coordinate in pixels.
	 * @param now Timestamp in milliseconds.
	 * @return +1 for left, -1 for right, or zero without a new swipe.
	 */
	int8_t update(bool touching, int16_t x, int16_t y, uint32_t now) {
		if (!touching) {
			active = false;
			return 0;
		}
		if (!active) {
			active = true;
			consumed = false;
			startX = x;
			startY = y;
			startMs = now;
			return 0;
		}
		if (consumed || (uint32_t)(now - startMs) > 1000) {
			return 0;
		}
		int32_t dx = (int32_t)x - startX;
		int32_t dy = (int32_t)y - startY;
		int32_t ax = dx < 0 ? -dx : dx;
		int32_t ay = dy < 0 ? -dy : dy;
		if (ax < 40 || ay * 2 > ax) {
			return 0;
		}
		consumed = true;
		return dx < 0 ? 1 : -1;
	}
};
