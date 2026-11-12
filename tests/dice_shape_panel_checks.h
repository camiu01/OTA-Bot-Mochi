/**
 * @file dice_shape_panel_checks.h
 * @brief Large solids, separate detail controls and optional native preview exports.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../OTA-bot-Mochi/src/ui/context_dice.h"
#include <fstream>
#include <cstdlib>

/** @brief Export an explicitly requested native preview, without changing production code. */
inline void dice_shape_preview(const Arduino_GFX &gfx, uint8_t sides) {
	const char *folder = std::getenv("MOCHI_DICE_PREVIEW");
	if (!folder) { return; }
	std::string name = std::string(folder) + "/dice-" + std::to_string(sides) + "-" +
		std::to_string(gfx.w) + "x" + std::to_string(gfx.h) + ".ppm";
	std::ofstream out(name, std::ios::binary);
	assert(out); out << "P6\n" << gfx.w << " " << gfx.h << "\n255\n";
	for (uint16_t pixel : gfx.pixels) {
		char rgb[] = {char((pixel >> 11) * 255 / 31), char(((pixel >> 5) & 63) * 255 / 63), char((pixel & 31) * 255 / 31)};
		out.write(rgb, 3);
	}
	assert(out);
}

/** @brief The two visible D10 numbers reconstruct every percentile result, including 100. */
inline void dice_percentile_shape_checks(Arduino_GFX &gfx) {
	desk_dice() = DicePool{}; desk_dice().cycle_type(1);
	for (uint8_t value = 1; value <= 100; ++value) {
		desk_dice().roll([value](uint8_t) { return value - 1; });
		gfx.text.clear(); desk_dice_solid(&gfx);
		assert(gfx.text.size() == 3 && gfx.text[0].size() == 2 && gfx.text[1].size() == 1);
		int tens = std::stoi(gfx.text[0]), units = std::stoi(gfx.text[1]);
		assert(tens % 10 == 0 && tens >= 0 && tens <= 90 && units >= 0 && units <= 9);
		int reconstructed = tens + units; if (!reconstructed) { reconstructed = 100; }
		assert(reconstructed == value);
	}
}

/** @brief Natural 1/20 only: reuse the real eye silhouettes and never alter the selected mood. */
inline void dice_critical_shape_checks(Arduino_GFX &gfx) {
	desk_dice() = DicePool{};
	auto mood = eyes_config().expression; auto ink = eyes_config().fgColor;
	assert(!desk_dice_reaction(&gfx));
	for (uint8_t value = 1; value <= 20; ++value) {
		desk_dice().roll([value](uint8_t) { return value - 1; });
		gfx.fillScreen(UI_BG); gfx.text.clear();
		bool critical = value == 1 || value == 20;
		assert(desk_dice_reaction(&gfx) == critical);
		if (critical) {
			assert(gfx.text.size() == 1 && gfx.text[0] == tr(value == 20 ? Text::DiceCritical : Text::DiceFumble));
			assert(std::any_of(gfx.pixels.begin(), gfx.pixels.end(), [](uint16_t c) { return c != UI_BG; }));
		}
		assert(eyes_config().expression == mood && eyes_config().fgColor == ink);
	}
	desk_dice().adjust_count(1); desk_dice().roll([](uint8_t) { return 19; });
	assert(!desk_dice_reaction(&gfx)); // A pool total is not a natural 20.
	desk_dice() = DicePool{}; desk_dice().cycle_type(1); desk_dice().roll([](uint8_t) { return 19; });
	assert(!desk_dice_reaction(&gfx)); // A D100 result of 20 is not critical.
	desk_dice().cycle_type(-1); assert(!desk_dice_reaction(&gfx));
}

/** @brief Previously dead lower-strip taps now reach the drawn buttons, not their neighbor. */
inline void dice_large_target_checks() {
	assert(dice_footer_bottom() - ui_s(224) >= ui_s(44));
	for (int16_t x : {ui_s(2), ui_s(60), int16_t(ui_s(118) - 1)}) {
		for (int16_t y : {ui_s(224), int16_t(dice_footer_bottom() - 1)}) {
			desk_dice() = DicePool{}; desk_dice_details() = false;
			screens_tap(UiPage::Dice, x, y, 0);
			assert(desk_dice().rolled() && !desk_dice_details());
		}
	}
	for (int16_t x : {ui_s(122), ui_s(180), int16_t(ui_s(238) - 1)}) {
		desk_dice() = DicePool{}; desk_dice_details() = false;
		screens_tap(UiPage::Dice, x, dice_footer_bottom() - 1, 0);
		assert(desk_dice_details() && !desk_dice().rolled());
	}
	assert(!desk_dice_tap(ui_s(120), ui_s(240)));
	assert(!desk_dice_tap(ui_s(60), dice_footer_bottom()));
	assert(!desk_dice_tap(ui_s(180), ui_s(222)));
}

/** @brief Every solid differs, fits both languages and preserves rolls across main/detail views. */
inline void dice_shape_panel_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	auto saved = settings_get(); auto pool = desk_dice(); bool details = desk_dice_details();
	unsigned writes = test_flash_writes;
	dice_large_target_checks();
	dice_percentile_shape_checks(gfx);
	for (uint8_t language = 0; language < 2; ++language) {
		auto p = saved; p.language = language; assert(settings_update(p));
		dice_critical_shape_checks(gfx);
		std::vector<std::vector<uint16_t>> shapes;
		desk_dice() = DicePool{};
		for (uint8_t type = 0; type < 7; ++type) {
			desk_dice().roll([](uint8_t limit) { return limit - 1; });
			auto total = desk_dice().total(); auto count = desk_dice().count();
			desk_dice_details() = false; gfx.fillScreen(UI_BG);
			screens_invalidate(); assert(screens_draw(&gfx, watch, UiPage::Dice, 500000 + type));
			for (const auto &other : shapes) { assert(gfx.pixels != other); }
			shapes.push_back(gfx.pixels);
			if (language == 1) { dice_shape_preview(gfx, desk_dice().sides()); }
			assert(desk_dice_tap(ui_s(180), ui_s(240)) && desk_dice_details());
			screens_invalidate(); screens_draw(&gfx, watch, UiPage::Dice, 500010 + type);
			assert(desk_dice().total() == total && desk_dice().count() == count);
			assert(desk_dice_tap(ui_s(60), ui_s(240)) && !desk_dice_details());
			assert(desk_dice().total() == total);
			desk_dice().cycle_type(1); // Old result must disappear on both views.
			gfx.fillScreen(UI_BG); desk_dice_draw(&gfx);
			assert(!desk_dice().rolled());
		}
	}
	assert(test_flash_writes == writes && settings_update(saved));
	desk_dice() = pool; desk_dice_details() = details;
}
