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
#include "../OTA-bot-Mochi/src/ui/save_glow.h"
#include "../OTA-bot-Mochi/src/ui/ui_font.h"
#include "../OTA-bot-Mochi/src/ui/page_slide.h"
#include "../OTA-bot-Mochi/src/ui/notification_glow.h"
#include "../OTA-bot-Mochi/src/ui/view_state.h"
#include "../OTA-bot-Mochi/src/ui/context_views.h"
#include "../OTA-bot-Mochi/src/ui/context_play.h"
#include "../OTA-bot-Mochi/src/ui/context_tris.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/settings/settings_menu.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/sound/sound.h"
#include "../OTA-bot-Mochi/src/power/charging.h"
#include "../OTA-bot-Mochi/src/bus/i2c_bus.h"
#include "../OTA-bot-Mochi/pin_config.h"
#include <assert.h>
#include <Preferences.h>
#include <iostream>
#include "music_panel_checks.h"
#include "diagnostics_panel_checks.h"
#include "dice_panel_checks.h"
#include "dice_shape_panel_checks.h"
#include "flappy_panel_checks.h"
#include "favorites_panel_checks.h"
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

/** @brief Battery outline and inset bolt stay inside the compact charging badge. */
static void charge_badge_checks() {
	Arduino_GFX icon(12, 17);
	icon.fillScreen(UI_BG);
	ui_charge_badge(&icon, 2, 2);
	assert(icon.pixels[2 * icon.w + 4] == UI_ACCENT);
	assert(icon.pixels[4 * icon.w + 2] == UI_ACCENT);
	assert(icon.pixels[14 * icon.w + 9] == UI_ACCENT);
	assert(icon.pixels[10 * icon.w + 6] == UI_ACCENT);
	for (int y = 0; y < icon.h; ++y) {
		for (int x = 0; x < icon.w; ++x) {
			if (x < 2 || x > 9 || y < 2 || y > 14) { assert(icon.pixels[y * icon.w + x] == UI_BG); }
		}
	}
}

/** @brief Charging transitions repaint a stationary header without a phone event or timer tick. */
static void charging_header_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
#if defined(MOCHI_BOARD_AMOLED_18)
	auto &bus = i2c_bus();
	bus.present[0x34] = true;
	auto &registers = bus.registers[0x34];
	registers[0x03] = 0x4A; registers[0x01] = 0;
	assert(!charging_poll(10000) && !charging_poll(10500));
	registers[0x01] = 0x20;
	assert(!charging_poll(11000));
	assert(screens_draw(&gfx, watch, UiPage::Weather, 11000));
	assert(!screens_draw(&gfx, watch, UiPage::Weather, 11001));
	int x = ui_s(208), y = ui_s(11);
	assert(gfx.pixels[y * gfx.w + x + 2] == UI_BG);
	assert(charging_poll(11500));
	assert(screens_draw(&gfx, watch, UiPage::Weather, 11500));
	assert(gfx.pixels[y * gfx.w + x + 2] == UI_ACCENT);
	registers[0x01] = 0;
	assert(!charging_poll(12000));
	assert(screens_draw(&gfx, watch, UiPage::Weather, 12500));
	assert(!charging_poll(12500) && !charging_active());
	assert(screens_draw(&gfx, watch, UiPage::Weather, 12501));
	assert(gfx.pixels[y * gfx.w + x + 2] == UI_BG);
#else
	(void)gfx; (void)watch;
	assert(!charging_active());
#endif
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
	auto original = settings_get();
	for (int16_t y : {int16_t(90), int16_t(133)}) {
		auto before = settings_get();
		assert(settings_menu_tap(ui_s(120), ui_s(y)) == MenuAction::Changed);
		assert(settings_get().style != before.style && settings_get().color == before.color);
	}
	for (int16_t y : {int16_t(134), int16_t(177)}) {
		auto before = settings_get();
		assert(settings_menu_tap(ui_s(120), ui_s(y)) == MenuAction::Changed);
		assert(settings_get().style == before.style && settings_get().color != before.color);
	}
	assert(settings_menu_tap(ui_s(120), ui_s(222)) == MenuAction::None);
	assert(settings_update(original));
	assert(settings_menu_tap(ui_s(120), ui_s(93)) == MenuAction::Changed);
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
	assert(settings_menu_tap(ui_s(8), ui_s(92)) == MenuAction::Changed);
	assert(settings_menu_tap(ui_s(115), ui_s(135)) == MenuAction::Changed);
	assert(settings_get().mood == 0);
	assert(settings_menu_tap(ui_s(118), ui_s(110)) == MenuAction::None);
	assert(settings_menu_tap(ui_s(120), ui_s(107)) == MenuAction::Changed);
	assert(settings_get().mood == 1);
	assert(settings_menu_scroll(1));
	assert(settings_menu_tap(ui_s(120), ui_s(107)) == MenuAction::Changed);
	assert(settings_get().mood == 3);
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
		assert(settings_menu_scroll(1));
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
	assert(screens_tap(UiPage::Desk, ui_s(20), ui_s(180), 0) == UiPage::AnalogClock);
	assert(screens_tap(UiPage::Desk, ui_s(200), ui_s(180), 0) == UiPage::Tally);
	assert(screens_tap(UiPage::Desk, ui_s(20), ui_s(210), 0) == UiPage::Score);
	assert(screens_tap(UiPage::Desk, ui_s(200), ui_s(210), 0) == UiPage::Phone);
	assert(screens_tap(UiPage::Desk, ui_s(20), ui_s(234), 0) == UiPage::Desk);
	assert(screens_tap(UiPage::Desk, ui_s(120), ui_s(210), 0) == UiPage::Desk);
	assert(screens_tap(UiPage::Desk, ui_s(20), ui_s(98), 0) == UiPage::Desk);
	uint8_t mode = view_timer().mode;
	screens_tap(UiPage::Timer, ui_s(120), ui_s(180), 1000);
	assert(view_timer().running);
	screens_tap(UiPage::Timer, ui_s(20), ui_s(180), 1000);
	assert(!view_timer().running);
	screens_tap(UiPage::Timer, ui_s(20), ui_s(230), 1000);
	assert(view_timer().remaining == 1500000);
	screens_tap(UiPage::Timer, ui_s(150), ui_s(230), 1000);
	assert(view_timer().mode == uint8_t((mode + 1) % DeskTimer::MODES));
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
	assert(screens_scroll(UiPage::Pong, 1) == UiPage::Games);
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
	assert(screens_scroll(UiPage::Tris, 1) == UiPage::Games);
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

/** @brief A successful save shows the toast and ring, then restores the menu. */
static void save_glow_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	if (!save_glow::Supported) { return; }
	const auto render = [&](uint32_t now) {
		gfx.text.clear();
		return screens_draw(&gfx, watch, UiPage::Settings, now, gfx.pixels.data());
	};
	const auto pixel = [&](int x, int y) { return gfx.pixels[y * LCD_WIDTH + x]; };
	const auto label = [&](const char *text) {
		return std::find(gfx.text.begin(), gfx.text.end(), text) != gfx.text.end();
	};
	constexpr uint32_t Start = UINT32_MAX - 300;
	assert(render(Start - 2));
	assert(ui_mix(UI_BG, 0xF800, 121) == 0x7800);
	assert(ui_mix(UI_BG, 0x07E0, 121) == 0x03C0);
	const int footer_gap = ui_s(78) + 4 * ui_s(12) - 1;
	assert(pixel(footer_gap, ui_s(267)) == UI_BG);
	const auto start_background = pixel(39, LCD_HEIGHT - 9);
	const auto top_background = pixel(LCD_WIDTH / 2, 8);
	const auto undimmed_card = pixel(LCD_WIDTH / 2, ui_s(96));
	assert(undimmed_card == UI_CARD);
	test_save_fails = true;
	assert(settings_menu_tap(ui_s(60), ui_s(240)) == MenuAction::Changed);
	test_save_fails = false;
	screens_invalidate();
	assert(render(Start - 1) && pixel(39, LCD_HEIGHT - 9) == start_background);
	assert(!label(tr(Text::ChangesSaved)));
	assert(settings_menu_tap(ui_s(60), ui_s(240)) == MenuAction::Changed);
	assert(render(Start) && pixel(39, LCD_HEIGHT - 9) != start_background);
	assert(pixel(LCD_WIDTH / 2, 8) == top_background);
	assert(render(Start + save_glow::SweepMs));
	assert(label(tr(Text::ChangesSaved)) && label(tr(Text::Save)) && !label(tr(Text::Saved)));
	assert(pixel(LCD_WIDTH / 2, ui_s(96)) == ui_mix(undimmed_card, UI_BG, 104));
	const auto lit_top = pixel(LCD_WIDTH / 2, 8);
	assert(lit_top != top_background && pixel(0, 0) == UI_BG);
	assert(render(Start + save_glow::SweepMs + save_glow::HoldMs + save_glow::FadeMs / 2));
	assert(pixel(LCD_WIDTH / 2, 8) != lit_top && pixel(LCD_WIDTH / 2, 8) != top_background);
	assert(render(Start + save_glow::Duration));
	assert(label(tr(Text::Save)) && !label(tr(Text::ChangesSaved)));
	assert(pixel(footer_gap, ui_s(267)) == UI_BG && pixel(LCD_WIDTH / 2, ui_s(96)) == undimmed_card);
	const auto restored = gfx.pixels;
	gfx.fillScreen(UI_BG);
	screens_invalidate();
	assert(render(Start + save_glow::Duration + 1) && gfx.pixels == restored);
	assert(!render(Start + save_glow::Duration + 2));
}

/** @brief Cards keep equal gutters while only the displayed value moves. */
static void settings_slide_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	if (!save_glow::Supported) { return; }
	const auto original = settings_get();
	const auto render = [&](uint32_t now) {
		gfx.text.clear();
		return screens_draw(&gfx, watch, UiPage::Settings, now, gfx.pixels.data());
	};
	const auto label = [&](const char *text) {
		return std::find(gfx.text.begin(), gfx.text.end(), text) != gfx.text.end();
	};
	screens_invalidate();
	assert(render(2400));
	for (int16_t boundary : {ui_s(134), ui_s(178)}) {
		const int16_t start = boundary - (ui_s(4) + 1) / 2;
		const int center = LCD_WIDTH / 2;
		assert(gfx.pixels[(start - 1) * LCD_WIDTH + center] == UI_CARD);
		for (int16_t y = start; y < start + ui_s(4); ++y) {
			assert(gfx.pixels[y * LCD_WIDTH + center] == UI_BG);
		}
		assert(gfx.pixels[(start + ui_s(4)) * LCD_WIDTH + center] == UI_CARD);
	}
	assert(settings_menu_tap(ui_s(100), ui_s(110)) == MenuAction::Changed);
	const char *old_value = tr_choice(0, original.style);
	const char *new_value = tr_choice(0, settings_get().style);
	assert(render(2401) && label(old_value) && !label(new_value));
	assert(render(2511) && !label(old_value) && !label(new_value));
	assert(render(2561) && !label(old_value) && label(new_value));
	assert(render(2621) && label(new_value));
	assert(!render(2622));
	assert(settings_menu_tap(ui_s(200), ui_s(70)) == MenuAction::Changed);
	assert(render(2700) && label(tr_settings_tab(0)) && !label(tr_settings_tab(1)));
	assert(render(2810) && !label(tr_settings_tab(0)) && !label(tr_settings_tab(1)));
	assert(render(2860) && label(tr_settings_tab(1)));
	assert(render(2920) && label(tr_settings_tab(1)));
	assert(settings_menu_tap(ui_s(30), ui_s(70)) == MenuAction::Changed);
	assert(render(3100) && label(tr_settings_tab(1)));
	assert(render(3320) && label(tr_settings_tab(0)));
	assert(settings_update(original) && settings_save());
}

/** @brief Check both slide directions, the flipped canvas and an unchanged destination frame. */
static void page_slide_checks(Arduino_GFX &gfx) {
	if (!PageSlide::Supported) { return; }
	PageSlide slide(std::malloc);
	auto *pixels = gfx.pixels.data();
	constexpr uint32_t Start = UINT32_MAX - 50;
	for (int8_t direction : {1, -1}) {
		for (bool flipped : {false, true}) {
			gfx.fillScreen(0x1111);
			slide.after_draw(pixels, Start - 1);
			slide.request(pixels, direction);
			gfx.fillScreen(0xAAAA);
			slide.after_draw(pixels, Start, flipped);
			assert(pixels[0] == 0x1111 && pixels[LCD_WIDTH - 1] == 0x1111);
			assert(slide.paint(pixels, Start + PageSlide::Duration / 2, flipped));
			const bool left = (direction > 0) != flipped;
			assert(pixels[LCD_WIDTH / 2 - 1] == (left ? 0x1111 : 0xAAAA));
			assert(pixels[LCD_WIDTH / 2] == (left ? 0xAAAA : 0x1111));
			assert(slide.paint(pixels, Start + PageSlide::Duration, flipped));
			assert(pixels[0] == 0xAAAA && pixels[LCD_WIDTH - 1] == 0xAAAA);
			assert(!slide.paint(pixels, Start + PageSlide::Duration + 1, flipped));
		}
	}
	screens_invalidate();
}

/** @brief Change the proportional font in System and verify explicit save and legacy defaults. */
static void settings_font_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	if (!ui_font::Supported) { return; }
	const auto original = settings_get();
	const auto legacy = test_profile;
	assert(legacy.size() == SETTINGS_BYTES && legacy[0] == 12 && original.font == UiFont::Default);
	assert(screens_draw(&gfx, watch, UiPage::Clock, 4000));
	assert(!gfx.text.empty());
	const auto default_clock = gfx.pixels;
	assert(settings_menu_tap(ui_s(30), ui_s(70)) == MenuAction::Changed);
	gfx.text.clear();
	settings_menu_draw(&gfx);
	assert(std::find(gfx.text.begin(), gfx.text.end(), "Default") != gfx.text.end());
	const auto old_pixels = gfx.pixels;
	const unsigned writes = test_flash_writes;
	assert(settings_menu_tap(ui_s(120), ui_s(155)) == MenuAction::Changed);
	assert(ui_font::active() && settings_get().font == UiFont::Inter && settings_dirty() && test_flash_writes == writes);
	assert(ui_font::width("WWW", 1) > ui_font::width("iii", 1));
	settings_menu_draw(&gfx);
	assert(gfx.pixels != old_pixels);
	assert(settings_menu_tap(ui_s(60), ui_s(240)) == MenuAction::Changed);
	assert(test_profile[0] == 14 && settings_init() && settings_get().font == UiFont::Inter);
	assert(!settings_dirty());
	ui_font::bind(&gfx, gfx.pixels.data());
	const UiPage pages[] = {UiPage::Music, UiPage::Clock, UiPage::Weather, UiPage::Drive, UiPage::Desk, UiPage::Games};
	std::vector<uint16_t> inter_clock;
	for (auto page : pages) {
		gfx.text.clear();
		assert(screens_draw(&gfx, watch, page, 5000));
		assert(gfx.text.empty());
		if (page == UiPage::Clock) { assert(gfx.pixels != default_clock); inter_clock = gfx.pixels; }
	}
	assert(settings_menu_tap(ui_s(120), ui_s(155)) == MenuAction::Changed);
	assert(ui_font::active() && settings_get().font == UiFont::DmSans && settings_dirty());
	assert(settings_menu_tap(ui_s(60), ui_s(240)) == MenuAction::Changed);
	assert(test_profile[0] == 14 && settings_init() && settings_get().font == UiFont::DmSans && !settings_dirty());
	gfx.text.clear();
	assert(screens_draw(&gfx, watch, UiPage::Clock, 5500) && gfx.text.empty());
	assert(gfx.pixels != inter_clock && gfx.pixels != default_clock);
	assert(settings_menu_tap(ui_s(120), ui_s(155)) == MenuAction::Changed);
	assert(!ui_font::active() && settings_get().font == UiFont::Default);
	ui_font::bind(nullptr, nullptr);
	test_profile = legacy;
	assert(settings_init() && settings_get().font == UiFont::Default && !settings_dirty());
	gfx.text.clear();
	assert(screens_draw(&gfx, watch, UiPage::Clock, 6000) && !gfx.text.empty());
	assert(settings_menu_tap(ui_s(200), ui_s(70)) == MenuAction::Changed);
	settings_menu_save_elapsed(0, false);
	screens_invalidate();
}

/** @brief Check the continuous loop covers all edges and leaves the content area alone. */
static void notification_glow_checks() {
	std::vector<uint16_t> first(size_t(LCD_WIDTH) * LCD_HEIGHT, UI_BG);
	auto next = first;
	notification_glow::draw(first.data(), false, 0);
	if (!notification_glow::Supported) { assert(first == next); return; }
	const auto pixel = [&](int x, int y) { return first[y * LCD_WIDTH + x]; };
	assert(pixel(4, LCD_HEIGHT / 2) && pixel(LCD_WIDTH - 5, LCD_HEIGHT / 2));
	assert(pixel(LCD_WIDTH / 2, 4) && pixel(LCD_WIDTH / 2, LCD_HEIGHT - 5));
	assert(pixel(15, 15) && !pixel(0, 0) && !pixel(LCD_WIDTH / 2, LCD_HEIGHT / 2));
	notification_glow::draw(next.data(), false, notification_glow::PeriodMs);
	assert(next == first);
	std::fill(next.begin(), next.end(), UI_BG);
	notification_glow::draw(next.data(), false, notification_glow::PeriodMs / 3);
	assert(next != first && next[(LCD_HEIGHT / 2) * LCD_WIDTH + 4] != UI_BG);
	const auto clean = next;
	std::vector<uint16_t> backup(notification_glow::BackupPixels);
	notification_glow::draw(next.data(), true, 100, 130, backup.data());
	assert(next != clean);
	notification_glow::restore(next.data(), true, backup.data());
	assert(next == clean);

	notification_glow::Pulse pulse;
	constexpr uint32_t start = UINT32_MAX - 300;
	assert(!pulse.update(0, start - 1).refresh);
	assert(pulse.update(1, start).refresh);
	assert(pulse.update(1, start + notification_glow::FadeInMs).opacity == 255);
	assert(pulse.update(1, start + 10000).opacity == 255);
	const uint32_t fade = start + notification_glow::DurationMs - notification_glow::FadeOutMs / 2;
	const auto fading = pulse.update(1, fade);
	assert(fading.opacity > 0 && fading.opacity < 255);
	assert(pulse.update(2, fade).elapsed == 0);
	const auto last = pulse.update(2, fade + notification_glow::DurationMs);
	assert(last.refresh && !last.opacity && !pulse.update(2, fade + notification_glow::DurationMs + 1).refresh);
	assert(pulse.update(3, 50000).refresh && pulse.update(3, 50100).opacity == 255);
	pulse.dismiss(3);
	const auto tapped = pulse.update(3, 50120);
	assert(tapped.refresh && !tapped.opacity && !pulse.update(3, 50200).refresh);
	assert(pulse.update(4, 50300).refresh);
}

/** @brief Run the layout proof for the panel the binary was compiled for. @return Success. */
int main() {
	assert(ui_state_init());
	assert(settings_init());
	ChronosESP32 watch;
	Arduino_GFX gfx;
	eyes_init(&gfx);
	settings_apply();
	save_glow_checks(gfx, watch);
	settings_slide_checks(gfx, watch);
	page_slide_checks(gfx);
	notification_glow_checks();
	settings_font_checks(gfx, watch);
	sd_settings_checks();
	music_panel_checks(gfx, watch);
	sound_badge_checks();
	charge_badge_checks();
	charging_header_checks(gfx, watch);
	page_render_checks(gfx, watch);
	menu_checks();
	grid_checks();
	flow_menu_checks(gfx);
	library_checks(gfx);
	favorites_panel_checks(gfx);
	diagnostics_panel_checks(gfx);
	desk_checks();
	play_checks(gfx, watch);
	tris_checks(gfx, watch);
	dice_panel_checks(gfx, watch);
	flappy_panel_checks(gfx, watch);
	dice_shake_panel_checks(gfx, watch);
	dice_shape_panel_checks(gfx, watch);
	std::cout << "Pages and menu layout passed on " << LCD_WIDTH << "x" << LCD_HEIGHT << "\n";
	return 0;
}
