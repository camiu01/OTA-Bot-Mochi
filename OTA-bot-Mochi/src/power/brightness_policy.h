/**
 * @file brightness_policy.h
 * @brief Host-testable night-safe brightness and optional idle dimming.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

struct BrightnessPolicy {
	uint8_t brightness = 30;
	bool dim = false;
	uint32_t last_activity = 0;

	/**
	 * @brief Keep active touch, routes, and visible calls readable.
	 * @param now Timestamp.
	 * @param touching Finger contact.
	 * @param keep_awake Critical content.
	 */
	void update(uint32_t now, bool touching, bool keep_awake) {
		if (touching || keep_awake) { last_activity = now; }
	}

	/**
	 * @brief Dim only when requested, never increasing a low night brightness.
	 * @param now Timestamp.
	 * @return Selected brightness or an idle cap of 3 percent.
	 */
	uint8_t output(uint32_t now) const {
		if (dim && uint32_t(now - last_activity) >= 60000) {
			return brightness < 3 ? brightness : 3;
		}
		return brightness;
	}
};
