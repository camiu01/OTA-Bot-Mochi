/**
 * @file favorites_panel_checks.h
 * @brief Reachable stars and compact selection on every test panel, in both languages.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
/** @brief Enter the FACES tab and scroll to its last three rows. */
static void favorites_tab() {
	settings_menu_tap(ui_s(200), ui_s(70)); settings_menu_tap(ui_s(200), ui_s(70));
	assert(settings_menu_scroll(1) && settings_menu_scroll(1) && settings_menu_scroll(1));
	assert(!settings_menu_scroll(1));
}
/** @brief Back closes overlays without losing stars or editing another field. */
static void favorites_panel_checks(Arduino_GFX &gfx) {
	auto original = settings_get();
	for (uint8_t language = 0; language < 2; ++language) {
		settings_defaults(); auto p = settings_get(); p.language = language; settings_update(p); assert(settings_save());
		favorites_tab();
		settings_menu_tap(ui_s(120), ui_s(155)); // Empty favorites list.
		gfx.text.clear(); settings_menu_draw(&gfx);
		assert(std::find(gfx.text.begin(), gfx.text.end(), tr(Text::NoFavorites)) != gfx.text.end());
		assert(settings_menu_tap(ui_s(20), ui_s(106)) == MenuAction::None && !settings_menu_scroll(1));
		settings_menu_tap(ui_s(180), ui_s(250));
		assert(settings_menu_scroll(1) && settings_menu_scroll(1) && settings_menu_scroll(1));
		settings_menu_tap(ui_s(120), ui_s(200)); // Edit favorites.
		unsigned writes = test_flash_writes;
		settings_menu_tap(ui_s(20), ui_s(106)); settings_menu_tap(ui_s(120), ui_s(150));
		assert(settings_favorites().contains(NEUTRAL) && settings_favorites().contains(ANGRY));
		assert(settings_favorites_dirty() && test_flash_writes == writes && settings_get().mood == NEUTRAL);
		FaceFilter excluded; assert(excluded.set(NEUTRAL, false) && settings_faces_update(excluded));
		settings_menu_draw(&gfx);
		settings_menu_tap(ui_s(180), ui_s(250));
		assert(settings_menu_scroll(1) && settings_menu_scroll(1) && settings_menu_scroll(1));
		settings_menu_tap(ui_s(120), ui_s(155)); settings_menu_draw(&gfx);
		assert(settings_menu_tap(ui_s(20), ui_s(106)) == MenuAction::Changed && settings_get().mood == ANGRY);
		assert(settings_menu_tap(ui_s(120), ui_s(106)) == MenuAction::None && !settings_menu_scroll(1));
		auto two = settings_favorites(); FaceFavorites all;
		for (uint16_t id = 0; id < EXPRESSION_COUNT; ++id) { all.set(Expression(id), true); }
		settings_favorites_update(all);
		unsigned scrolled = 0; while (settings_menu_scroll(1)) { ++scrolled; }
		assert(scrolled == ((EXPRESSION_COUNT - 1 + 1) / 2 - 3));
		settings_menu_draw(&gfx);
		assert(settings_menu_tap(ui_s(20), ui_s(200)) == MenuAction::Changed);
		assert(settings_get().mood == BALLOON_EYES && settings_menu_tap(ui_s(120), ui_s(200)) == MenuAction::None);
		settings_favorites_update(two); // External updates also clamp the old scroll position before a tap.
		assert(settings_menu_tap(ui_s(20), ui_s(106)) == MenuAction::Changed && settings_get().mood == ANGRY);
		test_save_fails = true; settings_menu_tap(ui_s(40), ui_s(250)); assert(settings_favorites_dirty());
		test_save_fails = false; settings_menu_tap(ui_s(40), ui_s(250)); assert(!settings_dirty());
		assert(settings_init() && settings_favorites().count() == 2 && settings_get().mood == ANGRY);
		assert(settings_menu_tap(ui_s(180), std::max(int(ui_s(280)), LCD_HEIGHT - 4)) == MenuAction::None);
		settings_menu_tap(ui_s(180), ui_s(250));
		settings_menu_tap(ui_s(30), ui_s(70)); settings_menu_tap(ui_s(30), ui_s(70));
	}
	settings_defaults(); assert(settings_update(original) && settings_save());
}
