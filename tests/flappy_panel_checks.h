/**
 * @file flappy_panel_checks.h
 * @brief Real page dispatch, frame cadence, scaled input and call pause checks.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../OTA-bot-Mochi/src/ui/context_flappy.h"
#include "../OTA-bot-Mochi/src/ui/context_hubs.h"
#include "../OTA-bot-Mochi/src/play/dice_input.h"

/** @brief Test gameplay navigation and off-page/overlay restoration on each panel. */
inline void flappy_panel_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	auto saved = settings_get();
	bool dirty = settings_dirty(); unsigned writes = test_flash_writes;
	desk_flappy() = FlappyBird{};
	assert(screens_draw(&gfx, watch, UiPage::FlappyBird, 300000));
	screens_tap(UiPage::FlappyBird, ui_s(100), ui_s(50), 300000);
	assert(!desk_flappy().running);
	screens_tap(UiPage::FlappyBird, ui_s(100), ui_s(120), 300000);
	assert(desk_flappy().running);
	assert(screens_draw(&gfx, watch, UiPage::FlappyBird, 300000));
	assert(!screens_draw(&gfx, watch, UiPage::FlappyBird, 300016));
	assert(screens_draw(&gfx, watch, UiPage::FlappyBird, 300033));
	float y = desk_flappy().y;
	assert(screens_draw(&gfx, watch, UiPage::Games, 300040));
	assert(screens_draw(&gfx, watch, UiPage::FlappyBird, 350000));
	assert(desk_flappy().y == y);
	auto p = saved; p.dashboard = false; assert(settings_update(p));
	ui_ringer("Caller", true);
	assert(screens_draw(&gfx, watch, UiPage::FlappyBird, 350033));
	assert(desk_flappy().y == y && screens_calling());
	ui_ringer("", false);
	assert(screens_draw(&gfx, watch, UiPage::FlappyBird, 360000));
	assert(desk_flappy().y == y);
	for (uint8_t language = 0; language < 2; ++language) {
		p.language = language; assert(settings_update(p));
		context_hub_draw(&gfx, true);
		for (uint8_t state = 0; state < 3; ++state) {
			desk_flappy().running = state == 1; desk_flappy().dead = state == 2;
			desk_flappy_draw(&gfx, 360000);
		}
		ui_label(&gfx, tr(Text::DiceShakeRoll), ui_s(18), ui_s(246), ui_s(204), 1, UI_BG);
	}
	assert(settings_update(saved));
	assert(settings_dirty() == dirty && test_flash_writes == writes);
}

/** @brief Shakes roll the existing maximum pool only while visible, not across calls or page entry. */
inline void dice_shake_panel_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	auto saved = settings_get();
	auto p = saved; p.dashboard = false; assert(settings_update(p));
	desk_dice() = DicePool{}; desk_dice().adjust_count(99);
	desk_dice().cycle_type(1); // 100D100, max total 10000.
	screens_draw(&gfx, watch, UiPage::Dice, 400000);
	assert(dice_input().active);
	dice_input().pending = true;
	assert(screens_draw(&gfx, watch, UiPage::Dice, 400001));
	assert(!dice_input().pending && desk_dice().rolled());
	assert(desk_dice().total() >= 100 && desk_dice().total() <= 10000);
	auto total = desk_dice().total();
	dice_input().pending = true;
	ui_ringer("Caller", true);
	assert(screens_draw(&gfx, watch, UiPage::Dice, 400002));
	assert(!dice_input().active && !dice_input().pending && desk_dice().total() == total);
	ui_ringer("", false);
	dice_input().pending = true;
	screens_draw(&gfx, watch, UiPage::Dice, 400003);
	assert(desk_dice().total() == total);
	dice_input().pending = true;
	screens_draw(&gfx, watch, UiPage::Games, 400004);
	assert(!dice_input().active && !dice_input().pending && desk_dice().total() == total);
	assert(settings_update(saved)); desk_dice() = DicePool{};
}
