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

/** @brief Execute boot-animation regressions without hardware. @return Success. */
int main() {
	check(240, 280); check(368, 448); check(128, 160);
	std::cout << "Startup title, animation, panel bounds, duration and rollover passed\n";
}
