/**
 * @file startup_test.cpp
 * @brief Boot branding, animation, finite duration and rollover on both panel sizes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/ui/startup.h"
#include <algorithm>
#include <assert.h>
#include <iostream>

/** @brief Verify one panel's intro, including clean completion. @param w Width. @param h Height. */
static void check(int16_t w, int16_t h) {
	StartupAnimation intro;
	Arduino_GFX gfx(w, h);
	assert(!intro.active(0) && !intro.draw(&gfx, 0, 0xFFFF));
	intro.begin(10000);
	assert(intro.draw(&gfx, 10600, 0xFFFF));
	assert(std::find(gfx.text.begin(), gfx.text.end(), "OTA BOT") != gfx.text.end());
	assert(std::find(gfx.text.begin(), gfx.text.end(), "MOCHI") != gfx.text.end());
	auto first = gfx.pixels;
	assert(std::count(first.begin(), first.end(), 0xFFFF) > 0);
	assert(intro.draw(&gfx, 11000, 0xFFFF) && gfx.pixels != first);
	assert(intro.draw(&gfx, 12199, 0xFFFF));
	auto last = gfx.pixels;
	assert(!intro.draw(&gfx, 12200, 0xFFFF) && gfx.pixels == last);
	assert(!intro.active(20000));
	intro.begin(UINT32_MAX - 500);
	assert(intro.active(499) && intro.draw(&gfx, 499, 0xFFFF));
	assert(!intro.active(1699));
	assert(!intro.draw(nullptr, 499, 0xFFFF));
}

/** @brief Farewell reuses branding, closes eyes, fades, finishes and handles rollover. */
static void shutdown_check(int16_t w, int16_t h) {
	StartupAnimation outro;
	Arduino_GFX gfx(w, h);
	outro.begin_shutdown(10000);
	assert(outro.draw(&gfx, 10000, 0xFFFF));
	assert(std::find(gfx.text.begin(), gfx.text.end(), "OTA BOT") != gfx.text.end());
	assert(std::find(gfx.text.begin(), gfx.text.end(), "MOCHI") != gfx.text.end());
	auto open = std::count(gfx.pixels.begin(), gfx.pixels.end(), 0xFFFF);
	assert(outro.draw(&gfx, 10000 + StartupAnimation::SHUTDOWN_MS / 2, 0xFFFF));
	assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), 0xFFFF) < open);
	assert(outro.draw(&gfx, 10000 + StartupAnimation::SHUTDOWN_MS - 1, 0xFFFF));
	assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), 0) == w * h);
	assert(!outro.draw(&gfx, 10000 + StartupAnimation::SHUTDOWN_MS, 0xFFFF));
	assert(!outro.active(10000 + StartupAnimation::SHUTDOWN_MS));
	uint32_t entry = UINT32_MAX - 500;
	outro.begin_shutdown(entry);
	uint32_t last = entry + StartupAnimation::SHUTDOWN_MS - 1;
	assert(outro.active(last) && outro.draw(&gfx, last, 0xFFFF));
	assert(!outro.active(last + 1));
	outro.begin(20000);
	assert(outro.active(21200));
}

/** @brief Execute boot-animation regressions without hardware. @return Success. */
int main() {
	static_assert(StartupAnimation::SHUTDOWN_MS + PowerButtonTiming::LONG_IRQ_MS ==
		PowerButtonTiming::POWER_OFF_MS, "Farewell must end at autonomous power-off");
	check(240, 280); check(368, 448); check(128, 160);
	shutdown_check(240, 280); shutdown_check(368, 448); shutdown_check(128, 160);
	std::cout << "Startup title, animation, panel bounds, duration and rollover passed\n";
}
