/**
 * @file dice_panel_checks.h
 * @brief Games hierarchy, D&D controls, EN/IT result bounds and RAM-only roll persistence.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../OTA-bot-Mochi/src/ui/context_dice.h"

/** @brief Walk both contextual layers and ignore unused card space. */
inline void games_navigation_checks() {
	assert(screens_scroll(UiPage::Clock, -1) == UiPage::Desk);
	assert(screens_scroll(UiPage::Desk, -1) == UiPage::Games);
	assert(screens_scroll(UiPage::Games, 1) == UiPage::Desk);
	assert(screens_scroll(UiPage::Desk, 1) == UiPage::Clock);
	assert(screens_scroll(UiPage::Games, -1) == UiPage::Games);
	const UiPage games[] = {UiPage::Dice, UiPage::Pong, UiPage::Tris};
	for (uint8_t i = 0; i < 3; ++i) {
		assert(screens_tap(UiPage::Games, ui_s(i % 2 ? 200 : 20), ui_s(70 + i / 2 * 50), 0) == games[i]);
		assert(screens_scroll(games[i], 1) == UiPage::Games);
		assert(screens_parent(games[i]) == UiPage::Clock);
		assert(screens_swipe(games[i], 1) == UiPage::Music);
		assert(screens_swipe(games[i], -1) == UiPage::Eyes);
	}
	for (auto point : {std::pair<int, int>{120, 70}, {20, 104}, {200, 120}, {20, 240}}) {
		assert(screens_tap(UiPage::Games, ui_s(point.first), ui_s(point.second), 0) == UiPage::Games);
	}
}
/** @brief Check the drawn controls, count/type changes and non-interactive gaps. */
inline void dice_control_checks() {
	desk_dice() = DicePool{};
	assert(!desk_dice_tap(ui_s(120), ui_s(74)));
	assert(!desk_dice_tap(ui_s(120), ui_s(120)));
	assert(!desk_dice_tap(ui_s(52), ui_s(120)));
	assert(!desk_dice_tap(ui_s(20), ui_s(216)));
	assert(!desk_dice_tap(-1, ui_s(240)) && !desk_dice_tap(ui_s(234), ui_s(240)));
	assert(desk_dice_tap(ui_s(210), ui_s(74)) && desk_dice().sides() == 100);
	assert(desk_dice_tap(ui_s(20), ui_s(74)) && desk_dice().sides() == 20);
	assert(desk_dice_tap(ui_s(210), ui_s(120)) && desk_dice().count() == 11);
	assert(desk_dice_tap(ui_s(70), ui_s(120)) && desk_dice().count() == 10);
	assert(desk_dice_tap(ui_s(170), ui_s(120)) && desk_dice().count() == 11);
	assert(desk_dice_tap(ui_s(20), ui_s(120)) && desk_dice().count() == 1);
	assert(screens_tap(UiPage::Dice, ui_s(120), ui_s(240), 0) == UiPage::Dice);
	assert(desk_dice().rolled() && desk_dice().total() >= 1 && desk_dice().total() <= 20);
}
/** @brief Max pool/results and every type fit both locales without random draws or preference writes. */
inline void dice_panel_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	games_navigation_checks(); dice_control_checks();
	const auto original = settings_get();
	for (uint8_t language = 0; language < 2; ++language) {
		auto p = original; p.language = language; assert(settings_update(p));
		uint32_t revision = settings_revision(); unsigned writes = test_flash_writes;
		bool dirty = settings_dirty();
		desk_dice() = DicePool{}; desk_dice().adjust_count(99);
		for (uint8_t type = 0; type < 7; ++type) {
			desk_dice().roll([](uint8_t limit) { return limit - 1; });
			for (uint8_t page = 0; page < desk_dice().pages(); ++page) {
				gfx.text.clear(); screens_invalidate();
				assert(screens_draw(&gfx, watch, UiPage::Dice, 100000 + type * 100 + page));
				assert(std::find(gfx.text.begin(), gfx.text.end(), tr(Text::DiceRoll)) != gfx.text.end());
				assert(desk_dice().total() == uint16_t(100 * desk_dice().sides()));
				if (page + 1 < desk_dice().pages()) { assert(desk_dice_tap(ui_s(200), ui_s(180))); }
			}
			assert(!desk_dice_tap(ui_s(200), ui_s(180)));
			desk_dice().cycle_type(1);
		}
		desk_dice().roll([](uint8_t limit) { return limit - 1; });
		auto pool = desk_dice();
		for (UiPage page : {UiPage::Games, UiPage::Desk, UiPage::Clock, UiPage::Dice}) {
			screens_invalidate(); assert(screens_draw(&gfx, watch, page, 200000));
		}
		assert(desk_dice().total() == pool.total() && desk_dice().value(99) == pool.value(99));
		assert(settings_revision() == revision && test_flash_writes == writes && settings_dirty() == dirty);
	}
	assert(settings_update(original)); desk_dice() = DicePool{};
}
