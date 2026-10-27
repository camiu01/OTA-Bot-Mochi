/**
 * @file startup.h
 * @brief Silent, time-driven boot/farewell title; callers keep servicing BLE, USB and touch.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Arduino_GFX_Library.h>
#include "../power/power_button.h"

/** @brief Main-task startup animation with unsigned rollover-safe timing. */
class StartupAnimation {
	uint32_t started = 0;
	bool enabled = false;
	bool closing = false;

	/** @brief Shade RGB565 against black. @param ink Color. @param level Percent. @return Shaded color. */
	static uint16_t shade(uint16_t ink, uint16_t level) {
		return uint16_t(((ink >> 11) * level / 100) << 11) |
			uint16_t(((((ink >> 5) & 63) * level / 100) << 5)) |
			uint16_t((ink & 31) * level / 100);
	}
	/** @brief Center fixed-font ASCII text. @param gfx Canvas. @param text Label. @param count Characters. @param y Top. @param size Font size. @param ink Color. */
	static void label(Arduino_GFX *gfx, const char *text, uint8_t count,
			int16_t y, uint8_t size, uint16_t ink) {
		gfx->setTextSize(size); gfx->setTextColor(ink);
		gfx->setCursor((gfx->width() - count * 6 * size) / 2, y);
		gfx->print(text);
	}
public:
	static constexpr uint32_t DURATION_MS = 2200; /**< Visible boot duration. */
	static constexpr uint32_t SHUTDOWN_MS = PowerButtonTiming::FAREWELL_MS; /**< Remaining hold until PMU power-off. */
	/** @brief Start after hardware initialization. @param now Timestamp. */
	void begin(uint32_t now) { started = now; enabled = true; closing = false; }
	/** @brief Reverse the opening eyes and fade to black. @param now Timestamp. */
	void begin_shutdown(uint32_t now) { begin(now); closing = true; }
	/** @brief Whether the intro still owns the screen. @param now Timestamp. @return Intro active. */
	bool active(uint32_t now) const {
		return enabled && uint32_t(now - started) < (closing ? SHUTDOWN_MS : DURATION_MS);
	}
	/** @brief Render one frame without delaying or flushing. @param gfx Canvas. @param now Timestamp. @param ink Theme color. @return Intro painted, false after completion. */
	bool draw(Arduino_GFX *gfx, uint32_t now, uint16_t ink) const {
		if (!gfx || !active(now)) { return false; }
		uint32_t elapsed = now - started;
		if (closing) { elapsed = 1000 - elapsed * 1000 / SHUTDOWN_MS; }
		int16_t w = gfx->width(), h = gfx->height();
		int16_t scale = w * 100 / 240 < h * 100 / 280 ? w * 100 / 240 : h * 100 / 280;
		uint8_t title_size = scale >= 140 ? 4 : (scale >= 80 ? 3 : 2);
		uint8_t small_size = scale >= 140 ? 3 : (scale >= 80 ? 2 : 1);
		uint16_t level = elapsed < 300 ? elapsed / 3 : (elapsed > 1800 ? (2200 - elapsed) / 4 : 100);
		gfx->fillScreen(0);
		label(gfx, "OTA BOT", 7, h / 5, title_size, shade(ink, level));
		label(gfx, "MOCHI", 5, h / 5 + title_size * 8 + 12 * scale / 100, small_size, shade(ink, level * 55 / 100));
		float opening = elapsed < 300 ? 0 : (elapsed >= 1000 ? 1 : (elapsed - 300) / 700.0f);
		opening = opening * opening * (3 - 2 * opening);
		float blink = elapsed >= 1350 && elapsed < 1550 ? (1550 - elapsed) / 200.0f : 1;
		if (elapsed >= 1550 && elapsed < 1750) { blink = (elapsed - 1550) / 200.0f; }
		int16_t eye_w = 28 * scale / 100;
		int16_t eye_h = int16_t((6 + 46 * opening) * scale / 100 * (1 - 0.85f * (1 - blink)));
		if (eye_h < 2) { eye_h = 2; }
		int16_t radius = (eye_w < eye_h ? eye_w : eye_h) / 2;
		const int16_t sides[] = {-1, 1};
		for (int16_t side : sides) {
			int16_t x = w / 2 + side * 28 * scale / 100 - eye_w / 2;
			gfx->fillRoundRect(x, h * 3 / 5 - eye_h / 2, eye_w, eye_h, radius, shade(ink, level));
		}
		int16_t track = w / 3;
		gfx->fillRect((w - track) / 2, h * 4 / 5, track, 2, shade(ink, level / 8));
		gfx->fillRect((w - track) / 2, h * 4 / 5, int32_t(track) * elapsed / DURATION_MS, 2, shade(ink, level / 2));
		return true;
	}
};
