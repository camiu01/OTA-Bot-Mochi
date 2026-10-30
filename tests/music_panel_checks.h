/**
 * @file music_panel_checks.h
 * @brief Panel-scaled media rendering, translations, and released-tap regressions.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../OTA-bot-Mochi/src/ui/music_view.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <cassert>
#include <string>

/** @brief Expose the SD toggle only on the board with an SD slot and codec. */
inline void sd_settings_checks() {
#if defined(MOCHI_BOARD_AMOLED_18)
	CompanionSettings original = settings_get();
	unsigned writes = test_flash_writes;
	for (uint8_t i = 0; i < 4; ++i) { settings_menu_tap(ui_s(210), ui_s(65)); }
	// SD sounds now sits in the main audio list, above the options and test rows.
	assert(settings_menu_scroll(1));
	assert(settings_menu_tap(ui_s(20), ui_s(200)) == MenuAction::Changed);
	assert(settings_get().sound_sd != original.sound_sd && test_flash_writes == writes);
	for (uint8_t i = 0; i < 4; ++i) { settings_menu_tap(ui_s(30), ui_s(65)); }
	assert(settings_update(original));
#endif
}

/** @brief Check controls and inactive gaps on the compiled panel geometry. */
inline void music_tap_checks() {
	ui_music(UiMusic{"Song", "Artist", true, false, 28});
	const MusicAction actions[] = {MusicAction::Previous, MusicAction::Toggle, MusicAction::Next};
	for (uint8_t i = 0; i < 3; ++i) {
		screens_tap(UiPage::Music, ui_s(46 + 74 * i), ui_s(230), 0);
		assert(ui_music_take() == actions[i]);
	}
	for (int16_t x : {0, 81, 155, 234}) {
		screens_tap(UiPage::Music, ui_s(x), ui_s(230), 0);
		assert(ui_music_take() == MusicAction::None);
	}
	screens_tap(UiPage::Music, ui_s(120), ui_s(205), 0);
	assert(ui_music_take() == MusicAction::None);
	screens_tap(UiPage::Music, ui_s(120), ui_s(254), 0);
	assert(ui_music_take() == MusicAction::None);
	ui_music(UiMusic{"Song", "Artist", true, true, 124, 60});
	music_tap(ui_s(46), ui_s(180)); assert(ui_music_take() == MusicAction::VolumeDown);
	music_tap(ui_s(194), ui_s(180)); assert(ui_music_take() == MusicAction::VolumeUp);
	music_tap(ui_s(120), ui_s(180)); assert(ui_music_take() == MusicAction::None);
	ui_music(UiMusic{"", "", true, false, 0});
	screens_tap(UiPage::Music, ui_s(120), ui_s(230), 0);
	assert(ui_music_take() == MusicAction::None);
}

/** @brief Render every media state and long labels in both languages. @param gfx Canvas. @param watch Clock. */
inline void music_panel_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	assert(screens_swipe(UiPage::Clock, 1) == UiPage::Music);
	assert(screens_swipe(UiPage::Music, 1) == UiPage::Navigation);
	assert(screens_swipe(UiPage::Music, -1) == UiPage::Clock);
	assert(screens_swipe(UiPage::Settings, 1) == UiPage::Eyes);
	CompanionSettings original = settings_get();
	for (uint8_t language = 0; language < 2; ++language) {
		auto value = original; value.language = language; assert(settings_update(value));
		ui_connection(false); screens_invalidate();
		assert(screens_draw(&gfx, watch, UiPage::Music, 0));
		ui_connection(true); screens_invalidate();
		assert(screens_draw(&gfx, watch, UiPage::Music, 1));
		ui_music(UiMusic{"", "", true, false, 28});
		assert(screens_draw(&gfx, watch, UiPage::Music, 2));
		ui_music(UiMusic{String(std::string(160, 'T')), String(std::string(96, 'A')), true, true, 124, 100});
		for (uint32_t now = 1000; now < 60000; now += 2000) {
			assert(screens_draw(&gfx, watch, UiPage::Music, now));
		}
		music_tap_checks();
	}
	assert(settings_update(original));
}
