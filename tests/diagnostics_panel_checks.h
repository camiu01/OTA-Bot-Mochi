/**
 * @file diagnostics_panel_checks.h
 * @brief Diagnostics navigation, live values, read-only behavior and scaled bilingual layout.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../OTA-bot-Mochi/src/diagnostics/diagnostics_view.h"

/** @brief Verify real counters and unknown host data remain distinguishable. */
inline void diagnostics_value_checks() {
	device_status::Snapshot state;
	assert(diagnostics_view::value(6, state, false) == "--");
	state.measured = true; state.heap = 123 * 1024; state.minimum_heap = 40 * 1024;
	state.psram = 0; state.uptime = 7380000; state.reset = "Brownout";
	assert(diagnostics_view::value(6, state, false) == "123 KiB");
	assert(diagnostics_view::value(7, state, false) == "40 KiB");
	assert(diagnostics_view::value(8, state, false) == "0 KiB");
	assert(diagnostics_view::value(9, state, false) == "2h 3m");
	assert(diagnostics_view::value(10, state, false) == "Brownout");
	assert(diagnostics_view::value(1, state, true) == tr(Text::Connected));
	assert(diagnostics_view::value(1, state, false) == tr(Text::Offline));
	state.wifi = true;
	assert(diagnostics_view::value(2, state, false) == tr(Text::Connected));
	assert(diagnostics_view::value(255, state, false) == "--");
	state.uptime = 5000000000ULL;
	assert(diagnostics_view::value(9, state, false) == "1388h 53m");
}

/** @brief Keep unsupported, disabled and last-attempt SD observations distinct. */
inline void diagnostics_sd_checks() {
#if MOCHI_BOARD_LCD_169
	assert(strcmp(diagnostics_view::sd_label(), tr(Text::DiagUnsupported)) == 0);
#else
	const auto original = settings_get();
	auto p = original; p.sound_sd = false; assert(settings_update(p));
	assert(strcmp(diagnostics_view::sd_label(), tr(Text::Off)) == 0);
	p.sound_sd = true; assert(settings_update(p));
	const Text labels[] = {Text::DiagUnchecked, Text::DiagReady, Text::DiagMissing, Text::Failed};
	for (uint8_t i = 0; i < 4; ++i) {
		device_status::sd = device_status::SdState(i);
		assert(strcmp(diagnostics_view::sd_label(), tr(labels[i])) == 0);
	}
	device_status::sd = device_status::SdState::Unchecked;
	assert(settings_update(original));
#endif
}

/** @brief Exercise the overlay from SYSTEM and return to LOOK without settings writes. @param gfx Panel mock. */
inline void diagnostics_panel_checks(Arduino_GFX &gfx) {
	const auto original = settings_get();
	for (uint8_t language = 0; language < 2; ++language) {
		auto p = original; p.language = language; assert(settings_update(p));
		for (uint8_t tab = 0; tab < 5; ++tab) { settings_menu_tap(ui_s(200), ui_s(70)); }
		for (uint8_t row = 0; row < 4; ++row) { assert(settings_menu_scroll(1)); }
		assert(!settings_menu_scroll(1));
		unsigned writes = test_flash_writes;
		bool dirty = settings_dirty(); uint32_t revision = settings_revision();
		assert(settings_menu_tap(ui_s(120), ui_s(200)) == MenuAction::Changed);
		assert(!settings_menu_scroll(-1));
		gfx.text.clear(); settings_menu_draw(&gfx);
		assert(std::find(gfx.text.begin(), gfx.text.end(), MOCHI_BOARD_LCD_169 ? "LCD 1.69 OLD" : "AMOLED 1.8 V2") != gfx.text.end());
		assert(settings_menu_tap(ui_s(200), ui_s(70)) == MenuAction::None);
		assert(settings_menu_tap(ui_s(20), ui_s(110)) == MenuAction::None);
		for (uint8_t row = 0; row < diagnostics_view::ROWS - 3; ++row) {
			assert(settings_menu_scroll(1)); settings_menu_draw(&gfx);
		}
		assert(!settings_menu_scroll(1));
		for (uint8_t row = 0; row < diagnostics_view::ROWS - 3; ++row) { assert(settings_menu_scroll(-1)); }
		assert(!settings_menu_scroll(-1));
		assert(settings_menu_tap(-1, ui_s(250)) == MenuAction::None);
		assert(test_flash_writes == writes && settings_dirty() == dirty && settings_revision() == revision);
		assert(settings_menu_tap(ui_s(20), ui_s(250)) == MenuAction::Changed);
		settings_menu_draw(&gfx);
		for (uint8_t tab = 0; tab < 5; ++tab) { settings_menu_tap(ui_s(30), ui_s(70)); }
	}
	assert(settings_update(original));
	diagnostics_value_checks();
	diagnostics_sd_checks();
}
