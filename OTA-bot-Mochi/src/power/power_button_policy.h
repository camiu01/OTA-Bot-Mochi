/**
 * @file power_button_policy.h
 * @brief Debounced hold-and-release shutdown, ignoring the power-on press.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

/** @brief Request shutdown once on release after a four-second hold. */
class PowerButtonHold {
	bool raw = false, stable = false, armed = false;
	uint32_t changed = 0, pressed = 0;
public:
	/** @brief Ignore a button already held while the device boots. */
	void begin(bool down, uint32_t now) {
		raw = stable = down; armed = !down; changed = pressed = now;
	}
	/** @brief Debounce input and report a released long press, including rollover. */
	bool update(bool down, uint32_t now) {
		if (down != raw) { raw = down; changed = now; }
		if (uint32_t(now - changed) < 30 || raw == stable) { return false; }
		stable = raw;
		if (stable) { pressed = now; return false; }
		bool request = armed && uint32_t(now - pressed) >= 4000;
		armed = true;
		return request;
	}
};
