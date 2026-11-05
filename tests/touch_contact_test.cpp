/**
 * @file touch_contact_test.cpp
 * @brief Live logical contacts leave tap, hold, pet and swipe recognition intact on both boards.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/touch/touch.h"
#include "../OTA-bot-Mochi/src/bus/i2c_bus.h"
#include "../OTA-bot-Mochi/pin_config.h"
#include <Arduino.h>
#include <cassert>
#include <iostream>

/** @brief Supply one complete CST816 contact on the existing shared bus. */
static int8_t sample(uint32_t now, int16_t x, int16_t y, bool down = true) {
	auto &r = i2c_bus().registers[0x15];
	r[2] = down ? 1 : 0;
	r[3] = uint8_t((down ? 0x80 : 0x40) | (x >> 8)); r[4] = uint8_t(x);
	r[5] = uint8_t(y >> 8); r[6] = uint8_t(y);
	return touch_poll(now);
}

/** @brief Contact reads are repeatable, logical, bounded by driver validation, and never consume a tap. */
static void contact() {
	int16_t x = -1, y = -1;
	assert(!touch_position(x, y) && x == -1 && y == -1);
	sample(100, 30, 40);
	assert(touch_pressed() && touch_position(x, y) && x == 30 && y == 40);
	assert(touch_position(x, y));
	assert(!touch_take_tap(x, y));
	sample(150, 30, 40, false);
	assert(!touch_position(x, y) && touch_take_tap(x, y) && x == 30 && y == 40);
	touch_set_flipped(true);
	sample(200, 20, 50);
	assert(touch_position(x, y) && x == LCD_WIDTH - 21 && y == LCD_HEIGHT - 51);
	sample(250, 20, 50, false);
	assert(touch_take_tap(x, y));
	touch_set_flipped(false);
	sample(300, 40, 50); sample(320, LCD_WIDTH, 50);
	assert(!touch_pressed() && !touch_position(x, y) && !touch_take_tap(x, y));
	sample(350, 40, 50); i2c_bus().read_ok = false; touch_poll(370);
	assert(!touch_position(x, y) && !touch_take_tap(x, y));
	i2c_bus().read_ok = true;
	sample(400, 40, 50);
	i2c_bus().registers[0x15][2] = 2; touch_poll(420);
	assert(!touch_position(x, y));
	touch_take_tap(x, y);
}

/** @brief Observing a held point must not swallow existing gestures. */
static void gestures() {
	int16_t x = 0, y = 0;
	sample(1000, 80, 80); sample(2250, 80, 80);
	assert(touch_position(x, y) && touch_take_hold() && !touch_take_hold());
	sample(2300, 80, 80, false); assert(!touch_take_tap(x, y));
	sample(3000, 80, 80); sample(3250, 80, 120);
	assert(touch_position(x, y) && touch_take_pet(x, y) && x == 80 && y == 120);
	sample(3300, 80, 120, false); assert(!touch_take_tap(x, y));
	sample(4000, LCD_WIDTH - 20, 100);
	int8_t swipe = sample(4200, 20, 100);
	assert(touch_position(x, y));
	swipe += sample(4300, 20, 100, false);
	assert(swipe != 0 && !touch_take_tap(x, y));
}

int main() {
	i2c_bus().present[0x15] = true;
	touch_init();
	contact(); gestures();
	std::cout << "Live contact, flip, release/error validation and non-consuming gestures passed\n";
}
