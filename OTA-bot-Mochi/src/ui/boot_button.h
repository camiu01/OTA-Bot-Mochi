/**
 * @file boot_button.h
 * @brief Debounced AMOLED BOOT release advances the existing six-page carousel.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../../pin_config.h"
#include "screens.h"
#include <Arduino.h>

/** @brief Consume one released click; ignore boot holds and blocked interactions. */
class BootPageButton {
#if defined(MOCHI_BOARD_AMOLED_18)
	bool raw = false, stable = false, armed = false;
	uint32_t changed = 0;
#endif
public:
	/** @brief Read BOOT without changing its reset-time strapping role. @param now Time. */
	void begin(uint32_t now) {
#if defined(MOCHI_BOARD_AMOLED_18)
		pinMode(BOOT_BUTTON, INPUT_PULLUP);
		raw = stable = digitalRead(BOOT_BUTTON) == LOW;
		armed = false; changed = now;
#else
		(void)now;
#endif
	}
	/** @brief Poll even while blocked, discarding rather than queuing clicks. @param now Time. @param page Current page. @param blocked Intro, shutdown, touch or call. @return Target page. */
	UiPage poll(uint32_t now, UiPage page, bool blocked) {
#if defined(MOCHI_BOARD_AMOLED_18)
		bool down = digitalRead(BOOT_BUTTON) == LOW;
		if (down != raw) { raw = down; changed = now; }
		if (blocked) { armed = false; }
		if (raw == stable || uint32_t(now - changed) < 30) { return page; }
		stable = raw;
		if (stable) { armed = !blocked; return page; }
		bool click = armed && !blocked;
		armed = false;
		return click ? screens_swipe(page, 1) : page;
#else
		(void)now; (void)blocked;
		return page;
#endif
	}
};
