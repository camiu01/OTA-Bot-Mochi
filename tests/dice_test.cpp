/**
 * @file dice_test.cpp
 * @brief Exhaustive die values, bounded pool edits, wide sums and stable result pagination.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/play/dice.h"
#include <cassert>
#include <iostream>

/** @brief Every standard type accepts every possible face and full 100-die extrema. */
static void roll_checks() {
	DicePool dice;
	for (uint8_t sides : {20, 100, 4, 6, 8, 10, 12}) {
		assert(dice.sides() == sides && !dice.rolled());
		for (uint8_t face = 1; face <= sides; ++face) {
			unsigned draws = 0;
			dice.roll([&](uint8_t limit) { ++draws; assert(limit == sides); return face - 1; });
			assert(draws == 1 && dice.value(0) == face && dice.total() == face);
			assert(dice.value(1) == 0 && dice.value(255) == 0);
		}
		assert(dice.adjust_count(99));
		assert(!dice.rolled() && dice.total() == 0 && dice.value(0) == 0);
		unsigned draws = 0;
		dice.roll([&](uint8_t limit) { ++draws; return limit - 1; });
		assert(draws == 100 && dice.total() == uint16_t(100 * sides));
		for (uint8_t i = 0; i < 100; ++i) { assert(dice.value(i) == sides); }
		dice.roll([](uint8_t) { return 0; });
		assert(dice.total() == 100);
		dice.adjust_count(-99); dice.cycle_type(1);
	}
	assert(dice.sides() == 20);
	dice.cycle_type(-1); assert(dice.sides() == 12);
	dice.cycle_type(1); assert(dice.sides() == 20);
}
/** @brief Clamp edits, preserve results for no-ops, reset on changes and browse without consuming RNG. */
static void state_checks() {
	DicePool dice;
	assert(dice.count() == 1 && dice.sides() == 20 && dice.pages() == 1 && !dice.move_page(1));
	assert(!dice.adjust_count(0) && !dice.adjust_count(-32768));
	assert(dice.adjust_count(32767) && dice.count() == 100 && dice.pages() == 9);
	unsigned draws = 0;
	dice.roll([&](uint8_t limit) { return draws++ % limit; });
	uint16_t total = dice.total();
	assert(!dice.adjust_count(10) && dice.rolled());
	dice.cycle_type(0); assert(dice.rolled());
	assert(!dice.move_page(-1) && !dice.move_page(0));
	for (uint8_t page = 1; page < 9; ++page) { assert(dice.move_page(1) && dice.page() == page); }
	assert(!dice.move_page(1) && draws == 100 && dice.total() == total);
	for (uint8_t i = 0; i < 100; ++i) { assert(dice.value(i) == i % 20 + 1); }
	for (uint8_t page = 8; page; --page) { assert(dice.move_page(-1)); }
	assert(!dice.move_page(-1));
	dice.move_page(1); dice.roll([](uint8_t limit) { return limit - 1; });
	assert(dice.page() == 0 && dice.total() == 2000);
	dice.cycle_type(1); dice.roll([](uint8_t limit) { return limit - 1; });
	assert(dice.sides() == 100 && dice.total() == 10000);
	dice.move_page(1); assert(dice.adjust_count(-87));
	assert(dice.count() == 13 && dice.pages() == 2 && !dice.rolled() && dice.page() == 0);
	dice.roll([](uint8_t) { return 0; });
	dice.cycle_type(1); assert(!dice.rolled() && dice.total() == 0 && dice.value(0) == 0);
}
/** @brief Execute portable dice rules without a display or device. @return Success. */
int main() {
	roll_checks(); state_checks();
	std::cout << "D&D dice types, all faces, 1-100 counts, totals and result pagination passed\n";
}
