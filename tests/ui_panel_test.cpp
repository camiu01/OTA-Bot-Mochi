/**
 * @file ui_panel_test.cpp
 * @brief Pages and menus on one panel geometry; the runner builds it per panel.
 *
 * Every widget is laid out in the 240x280 design space and scaled to the
 * panel with ui_s()/ui_scale(), so on any panel the same design-space taps
 * must reach the same widgets, every label must fit the canvas (the raster
 * mock aborts with a message otherwise), and the strip below the scaled
 * design field must stay dead. Rendering runs on a native mock; it does not
 * prove touch digitizer calibration or panel wiring.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/ui/screens.h"
#include "../OTA-bot-Mochi/src/ui/ui_theme.h"
#include "../OTA-bot-Mochi/src/ui/view_state.h"
#include "../OTA-bot-Mochi/src/ui/context_views.h"
#include "../OTA-bot-Mochi/src/ui/context_play.h"
#include "../OTA-bot-Mochi/src/ui/context_tris.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/settings/settings_menu.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/sound/sound.h"
#include "../OTA-bot-Mochi/pin_config.h"
#include <assert.h>
#include <Preferences.h>
#include <iostream>
#include "music_panel_checks.h"
#include "diagnostics_panel_checks.h"
#include <string>

/** @brief Speaker mouth faces right; mute cross and sound waves stay inside the badge. */
static void sound_badge_checks() {
	Arduino_GFX icon(26, 17);
	constexpr int x = 2, y = 2;
	CompanionSettings saved = settings_get();
	for (uint8_t mode = 0; mode < 3; ++mode) {
		auto value = saved;
		value.sound = mode != 0;
		value.night = mode == 2;
		assert(settings_update(value));
		sound_init(0);
		bool audible = mode == 1;
		uint16_t color = audible ? UI_ACCENT : UI_MUTED;
		assert(sound_status() == (audible ? SoundStatus::Armed :
			(mode == 0 ? SoundStatus::Off : SoundStatus::Night)));
		icon.fillScreen(UI_BG);
		ui_sound_badge(&icon, x, y);
		// A broad vertical mouth, not an arrow tip, fills the right of the cone.
		for (int row = 0; row <= 12; ++row) {
			assert(icon.pixels[(y + row) * icon.w + x + 10] == color);
		}
		assert(icon.pixels[y * icon.w + x + 4] == UI_BG);
		assert(icon.pixels[(y + 6) * icon.w + x] == color);
		assert(icon.pixels[(y + 6) * icon.w + x + 12] == UI_BG);
		assert(icon.pixels[(y + 6) * icon.w + x + 17] == (audible ? UI_BG : color));
		assert(icon.pixels[(y + 6) * icon.w + x + 21] == (audible ? color : UI_BG));
		for (int row = 0; row < icon.h; ++row) {
			for (int col = 0; col < icon.w; ++col) {
				if (col < x || col > x + 21 || row < y || row > y + 12) {
					assert(icon.pixels[row * icon.w + col] == UI_BG);
				}
			}
		}
	}
	assert(settings_update(saved));
	sound_init(0);
}

/** @brief Render every carousel and tool page once with realistic content. @param gfx Canvas. @param watch Chronos clock. */
static void page_render_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	Notification notification;
	notification.app = "Messages";
	notification.title = "A long title with multiple words for wrapping on this small display";
	notification.message = std::string(1400, 'x');
	ui_notification(notification);
	watch.navigation.active = true;
	watch.navigation.directions = "Turn right at the next street after the bridge";
	ui_config(watch, CF_NAV_DATA, 1, 0);
	watch.weather_count = 1;
	watch.weather[0].temp = 18;
	ui_config(watch, CF_WEATHER, 1, 0);
	// Prime a transition so the first loop entry also repaints from black.
	assert(screens_draw(&gfx, watch, UiPage::Clock, 0));
	for (uint8_t page = 0; page < uint8_t(UiPage::Count); ++page) {
		gfx.paints = 0;
		assert(screens_draw(&gfx, watch, static_cast<UiPage>(page), 1000 + page * 1000));
		assert(gfx.paints > 0);
		(void)screens_draw(&gfx, watch, static_cast<UiPage>(page), 1033 + page * 1000);
	}
}

/** @brief Tab cycling, row edits, and the settings footer stay reachable. */
static void menu_checks() {
	assert(settings_menu_tap(ui_s(30), ui_s(70)) == MenuAction::Changed);
	assert(settings_menu_tap(ui_s(200), ui_s(70)) == MenuAction::Changed);
	assert(settings_menu_tap(ui_s(120), ui_s(70)) == MenuAction::None);
	assert(settings_menu_tap(ui_s(120), ui_s(93)) == MenuAction::None);
	CompanionSettings before = settings_get();
	assert(settings_menu_tap(ui_s(120), ui_s(110)) == MenuAction::Changed);
	assert(settings_get().style != before.style);
	assert(settings_menu_tap(ui_s(120), ui_s(160)) == MenuAction::Changed);
	assert(settings_get().color != before.color);
	assert(settings_menu_tap(ui_s(60), ui_s(250)) == MenuAction::Changed);
	assert(!settings_dirty());
	assert(settings_menu_tap(ui_s(180), ui_s(250)) == MenuAction::Exit);
}

/** @brief Expression grid: cell hit, page scroll, then back to the tab rows. */
static void grid_checks() {
	CompanionSettings p = settings_get();
	p.mood = 0;
	settings_update(p);
	assert(settings_menu_tap(ui_s(120), ui_s(200)) == MenuAction::Changed);
	assert(settings_menu_grid_open());
	assert(settings_menu_tap(ui_s(120), ui_s(107)) == MenuAction::Changed);
	assert(settings_get().mood == 1);
	assert(settings_menu_scroll(1));
	assert(settings_menu_tap(ui_s(120), ui_s(107)) == MenuAction::Changed);
	assert(settings_get().mood == 4);
	assert(settings_menu_tap(ui_s(180), ui_s(250)) == MenuAction::Changed);
	assert(!settings_menu_grid_open());
	assert(!settings_menu_scroll(1) && !settings_menu_scroll(-1));
}

/** @brief Every playback mode is reachable in both languages on the current panel. */
static void flow_menu_checks(Arduino_GFX &gfx) {
	CompanionSettings saved = settings_get();
	for (uint8_t language = 0; language < 2; ++language) {
		CompanionSettings p = saved; p.language = language; p.face_flow = FACE_RETURN;
		assert(settings_update(p));
		assert(settings_menu_tap(ui_s(200), ui_s(70)) == MenuAction::Changed);
		assert(settings_menu_tap(ui_s(200), ui_s(70)) == MenuAction::Changed);
		assert(!settings_menu_scroll(-1));
		for (uint8_t mode = 0; mode < FACE_FLOW_COUNT; ++mode) {
			assert(settings_get().face_flow == mode);
			assert(strcmp(tr_choice(4, mode), "?") != 0);
			settings_menu_draw(&gfx);
			assert(settings_menu_tap(ui_s(120), ui_s(200)) == MenuAction::Changed);
		}
		assert(settings_get().face_flow == FACE_RETURN);
		assert(settings_menu_tap(ui_s(30), ui_s(70)) == MenuAction::Changed);
		assert(settings_menu_tap(ui_s(30), ui_s(70)) == MenuAction::Changed);
	}
	assert(settings_update(saved));
}

/** @brief Library toggles, last-face guard, Wi-Fi action and excluded manual choices scale. */
static void library_checks(Arduino_GFX &gfx) {
	auto original = settings_get();
	for (uint8_t language = 0; language < 2; ++language) {
		settings_defaults(); assert(settings_save());
		auto p = settings_get(); p.language = language; assert(settings_update(p));
		assert(settings_menu_tap(ui_s(200), ui_s(70)) == MenuAction::Changed);
		assert(settings_menu_tap(ui_s(200), ui_s(70)) == MenuAction::Changed);
		assert(settings_menu_scroll(1) && !settings_menu_scroll(1));
		assert(settings_menu_tap(ui_s(120), ui_s(200)) == MenuAction::Changed);
		assert(settings_menu_grid_open());
		settings_menu_draw(&gfx);
		assert(settings_menu_tap(ui_s(20), ui_s(106)) == MenuAction::Changed);
		assert(!face_filter().allows(NEUTRAL) && settings_get().mood == HAPPY);
		FaceFilter one;
		for (uint8_t id = 0; id < EXPRESSION_COUNT; ++id) { if (id != HAPPY) { one.set(Expression(id), false); } }
		assert(settings_faces_update(one));
		assert(settings_menu_tap(ui_s(120), ui_s(106)) == MenuAction::Changed);
		assert(face_filter().count() == 1 && face_filter().allows(HAPPY));
		gfx.text.clear(); settings_menu_draw(&gfx);
		assert(std::find(gfx.text.begin(), gfx.text.end(), tr(Text::KeepOneFace)) != gfx.text.end());
		assert(settings_menu_tap(ui_s(200), ui_s(70)) == MenuAction::FacesWeb);
		test_save_fails = true;
		assert(settings_menu_tap(ui_s(40), ui_s(250)) == MenuAction::Changed && settings_faces_dirty());
		test_save_fails = false;
		assert(settings_menu_tap(ui_s(40), ui_s(250)) == MenuAction::Changed && !settings_faces_dirty());
		assert(settings_menu_tap(ui_s(180), ui_s(250)) == MenuAction::Changed);
		settings_menu_tap(ui_s(30), ui_s(70)); settings_menu_tap(ui_s(30), ui_s(70));
		assert(settings_menu_tap(ui_s(120), ui_s(200)) == MenuAction::Changed);
		assert(settings_menu_tap(ui_s(20), ui_s(106)) == MenuAction::None);
		assert(settings_get().mood == HAPPY);
		settings_menu_tap(ui_s(180), ui_s(250));
	}
	settings_defaults(); assert(settings_update(original) && settings_save());
}

/** @brief Desk cards, timer buttons, and the strip below the scaled field. */
static void desk_checks() {
	assert(screens_tap(UiPage::Desk, ui_s(20), ui_s(70), 0) == UiPage::BigClock);
	assert(screens_tap(UiPage::Desk, ui_s(200), ui_s(70), 0) == UiPage::Overview);
	assert(screens_tap(UiPage::Desk, ui_s(20), ui_s(140), 0) == UiPage::Timer);
	assert(screens_tap(UiPage::Desk, ui_s(200), ui_s(140), 0) == UiPage::Device);
	assert(screens_tap(UiPage::Desk, ui_s(20), ui_s(180), 0) == UiPage::Pong);
	assert(screens_tap(UiPage::Desk, ui_s(200), ui_s(180), 0) == UiPage::AnalogClock);
	assert(screens_tap(UiPage::Desk, ui_s(20), ui_s(230), 0) == UiPage::Tris);
	assert(screens_tap(UiPage::Desk, ui_s(120), ui_s(210), 0) == UiPage::Desk);
	assert(screens_tap(UiPage::Desk, ui_s(20), ui_s(104), 0) == UiPage::Desk);
	uint8_t mode = view_timer().mode;
	screens_tap(UiPage::Timer, ui_s(120), ui_s(180), 1000);
	assert(view_timer().running);
	screens_tap(UiPage::Timer, ui_s(20), ui_s(180), 1000);
	assert(!view_timer().running);
	screens_tap(UiPage::Timer, ui_s(20), ui_s(230), 1000);
	assert(view_timer().remaining == 1500000);
	screens_tap(UiPage::Timer, ui_s(150), ui_s(230), 1000);
	assert(view_timer().mode == uint8_t((mode + 1) % 3));
	// A panel taller than the scaled design field grows black below it; that
	// strip must not press widgets drawn only inside the field.
	if (LCD_HEIGHT > ui_s(UI_DESIGN_H)) {
		assert(screens_tap(UiPage::Desk, LCD_WIDTH / 2, LCD_HEIGHT - 4, 0) == UiPage::Desk);
		assert(settings_menu_tap(LCD_WIDTH / 2, LCD_HEIGHT - 4) == MenuAction::None);
		uint8_t before = view_timer().mode;
		screens_tap(UiPage::Timer, LCD_WIDTH / 2, LCD_HEIGHT - 4, 0);
		assert(view_timer().mode == before);
	}
}

/** @brief Game cadence/input, clock validation and contextual navigation work at panel scale. */
static void play_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	uint32_t seconds = 123;
	assert(desk_clock_seconds("23:59:59", seconds) && seconds == 86399);
	assert(desk_clock_seconds("00:00:00", seconds) && seconds == 0);
	for (const char *bad : {"24:00:00", "12:60:00", "12:00:60", "xx:00:00", "1:00:00", "12:00:00z", ""}) {
		assert(!desk_clock_seconds(bad, seconds) && seconds == 0);
	}
	assert(!desk_clock_seconds(nullptr, seconds));
	bool dirty = settings_dirty();
	desk_pong().reset(50000);
	assert(screens_draw(&gfx, watch, UiPage::Pong, 50000));
	assert(!screens_draw(&gfx, watch, UiPage::Pong, 50016));
	float previous_x = desk_pong().x;
	assert(screens_draw(&gfx, watch, UiPage::Pong, 50033));
	assert(desk_pong().x > previous_x);
	screens_tap(UiPage::Pong, ui_s(100), ui_s(84), 50040);
	assert(desk_pong().left == 16);
	screens_tap(UiPage::Pong, ui_s(100), ui_s(208), 50040);
	assert(desk_pong().left == 144);
	screens_tap(UiPage::Pong, ui_s(100), ui_s(50), 50040);
	assert(desk_pong().left == 144);
	screens_tap(UiPage::Pong, ui_s(100), ui_s(245), 50040);
	assert(desk_pong().left == 80 && desk_pong().player == 0);
	assert(screens_draw(&gfx, watch, UiPage::Desk, 50050));
	previous_x = desk_pong().x;
	assert(screens_draw(&gfx, watch, UiPage::Pong, 90000));
	assert(desk_pong().x == previous_x); // Off-page time must not advance a game.
	assert(settings_dirty() == dirty);
	assert(screens_scroll(UiPage::Pong, 1) == UiPage::Desk);
	assert(screens_swipe(UiPage::AnalogClock, 1) == UiPage::Music);
	assert(screens_parent(UiPage::Pong) == UiPage::Clock);
	watch.analog_time = "bad";
	clock_time_synced();
	assert(screens_draw(&gfx, watch, UiPage::AnalogClock, 91000));
	assert(gfx.text.back() == tr(Text::OpenSync));
	watch.analog_time = "13:45:30";
	screens_invalidate();
	assert(screens_draw(&gfx, watch, UiPage::AnalogClock, 91001));
	auto before = gfx.pixels;
	watch.analog_time = "13:45:45";
	assert(screens_draw(&gfx, watch, UiPage::AnalogClock, 92001));
	assert(gfx.pixels != before);
}

/** @brief Tris touch cells, turn delay, reset and contextual navigation scale with the panel. */
static void tris_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	bool dirty = settings_dirty();
	desk_tris().reset();
	screens_tap(UiPage::Tris, ui_s(20), ui_s(90), 1000);
	assert(!desk_tris().thinking);
	for (uint8_t i = 0; i < 9; ++i) {
		desk_tris().reset();
		screens_tap(UiPage::Tris, ui_s(72 + i % 3 * 48), ui_s(102 + i / 3 * 48), 1000);
		assert(desk_tris().cells[i] == 1 && desk_tris().thinking);
		assert(screens_draw(&gfx, watch, UiPage::Tris, 1000));
		assert(desk_tris().thinking);
		screens_invalidate();
		assert(screens_draw(&gfx, watch, UiPage::Tris, 1450));
		assert(!desk_tris().thinking);
	}
	screens_tap(UiPage::Tris, ui_s(120), ui_s(244), 1500);
	for (uint8_t cell : desk_tris().cells) { assert(cell == 0); }
	assert(screens_scroll(UiPage::Tris, 1) == UiPage::Desk);
	assert(screens_swipe(UiPage::Tris, 1) == UiPage::Music);
	assert(settings_dirty() == dirty);
	for (uint8_t language = 0; language < 2; ++language) {
		auto p = settings_get(); p.language = language; settings_update(p);
		for (const TicTacToe game : {TicTacToe{}, TicTacToe{{1,1,1}}, TicTacToe{{2,2,2}},
				TicTacToe{{1,2,1,1,2,2,2,1,1}}}) {
			desk_tris() = game; desk_tris_draw(&gfx, 2000);
		}
	}
}

/** @brief Run the layout proof for the panel the binary was compiled for. @return Success. */
int main() {
	assert(ui_state_init());
	assert(settings_init());
	ChronosESP32 watch;
	Arduino_GFX gfx;
	eyes_init(&gfx);
	settings_apply();
	sd_settings_checks();
	music_panel_checks(gfx, watch);
	sound_badge_checks();
	page_render_checks(gfx, watch);
	menu_checks();
	grid_checks();
	flow_menu_checks(gfx);
	library_checks(gfx);
	diagnostics_panel_checks(gfx);
	desk_checks();
	play_checks(gfx, watch);
	tris_checks(gfx, watch);
	std::cout << "Pages and menu layout passed on " << LCD_WIDTH << "x" << LCD_HEIGHT << "\n";
	return 0;
}
