/**
 * @file settings_test.cpp
 * @brief Native coverage for touch/USB controls, saved profiles, and road vibration filtering.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/settings/settings_menu.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/motion_filter.h"
#include "../OTA-bot-Mochi/src/touch/touch_actions.h"
#include "../OTA-bot-Mochi/src/power/backlight.h"
#include "../OTA-bot-Mochi/src/power/battery.h"
#include "../OTA-bot-Mochi/src/ui/view_state.h"
#include "../OTA-bot-Mochi/src/ui/i18n.h"
#include "../OTA-bot-Mochi/src/ui/ui_theme.h"
#include <Preferences.h>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include <string>

/** @brief Mirrors of the Settings expression grid layout, so adding expressions never needs a test edit. */
constexpr uint8_t GRID_COLS_HINT = 3;
constexpr uint8_t GRID_PAGE_ROWS_HINT = 5;

/** @brief Test explicit persistence, malformed blobs, and unavailable storage. */
static void persistence_tests() {
	assert(settings_init());
	eyes_init(nullptr);
	settings_apply();
	assert(settings_get().dashboard && !settings_get().shake && !settings_get().dim);
	assert(eyes_config().style == MOCHI && backlight_brightness() == 30);
	CompanionSettings value = settings_get();
	value.style = 3; value.color = 2; value.brightness = 40; value.night = true;
	value.random_faces = true; value.imu_faces = true;
	value.route_large = false;
	value.sound = true; value.sound_faces = false; value.sound_alerts = false;
	value.sound_ambient = false; value.sound_night_mute = false;
	value.sound_level = 18; value.animation = 2; value.language = 1;
	value.flipped = true; value.face_flow = FACE_GROUP5;
	assert(settings_update(value));
	assert(eyes_config().style == TRAPEZ && eyes_config().fgColor == 0x07FF);
	assert(backlight_brightness() == 5 && test_flash_writes == 0);
	assert(settings_save() && test_flash_writes == 1 && !settings_dirty());
	assert(settings_save() && test_flash_writes == 1);
	assert(settings_init() && settings_get().style == 3 && settings_get().night);
	assert(settings_get().random_faces && settings_get().imu_faces);
	assert(!settings_get().route_large);
	assert(settings_get().sound && !settings_get().sound_faces && !settings_get().sound_alerts);
	assert(!settings_get().sound_ambient && !settings_get().sound_night_mute);
	assert(settings_get().sound_level == 18 && settings_get().animation == 2 && settings_get().language == 1);
	assert(settings_get().flipped);
	assert(settings_get().face_flow == FACE_GROUP5);
	value = settings_get(); value.color = 4;
	settings_update(value); test_save_fails = true;
	assert(!settings_save() && settings_dirty());
	test_save_fails = false;
	test_profile[2] ^= 0xFF;
	assert(settings_init() && settings_get().style == 1);
	test_storage_ready = false;
	assert(!settings_init() && !settings_save() && settings_get().dashboard);
	test_storage_ready = true;
	test_profile.clear();
	settings_init();
}

/** @brief Version-one profiles retain their old choices while new automation defaults off. */
static void migration_tests() {
	CompanionSettings value;
	value.color = 3; value.style = 2;
	uint8_t bytes[8] = {1, value.style, value.color, value.mood, value.brightness, value.night_brightness, 2, 0xA5};
	for (uint8_t i = 0; i < 7; ++i) { bytes[7] ^= bytes[i]; }
	CompanionSettings decoded;
	assert(settings_decode(bytes, sizeof(bytes), decoded));
	assert(decoded.color == 3 && decoded.style == 2 && !decoded.random_faces && !decoded.imu_faces);
	test_profile.assign(bytes, bytes + 8);
	assert(settings_init() && settings_get().color == 3);
	assert(decoded.route_large);
	assert(settings_dirty() && settings_save() && test_profile[0] == 6 && test_profile.size() == SETTINGS_BYTES);
	bytes[0] = 2; bytes[6] = 0x32; bytes[7] = 0xA5;
	for (uint8_t i = 0; i < 7; ++i) { bytes[7] ^= bytes[i]; }
	assert(settings_decode(bytes, sizeof(bytes), decoded) && decoded.random_faces && decoded.imu_faces && decoded.route_large);
	bytes[0] = 3; bytes[7] = 0xA5;
	for (uint8_t i = 0; i < 7; ++i) { bytes[7] ^= bytes[i]; }
	assert(settings_decode(bytes, sizeof(bytes), decoded) && !decoded.route_large && !decoded.sound);
	// Version-four twelve-byte blobs still decode, with the newer flip flag defaulting off.
	uint8_t v4[SETTINGS_V4_BYTES] = {4, 2, 3, uint8_t(LOVE), 30, 5, 2, 0, 10, 1, 0, 0xA5};
	for (uint8_t i = 0; i < SETTINGS_V4_BYTES - 1; ++i) { v4[SETTINGS_V4_BYTES - 1] ^= v4[i]; }
	CompanionSettings from_v4;
	assert(settings_decode(v4, sizeof(v4), from_v4));
	assert(from_v4.style == 2 && from_v4.color == 3 && from_v4.mood == LOVE && !from_v4.flipped);
	test_profile.assign(v4, v4 + SETTINGS_V4_BYTES);
	assert(settings_init() && settings_get().style == 2 && !settings_get().flipped);
	assert(settings_dirty() && settings_save() && test_profile[0] == 6 && test_profile.size() == SETTINGS_BYTES);
	uint8_t v5[SETTINGS_BYTES];
	CompanionSettings old; old.flipped = true;
	settings_encode(old, v5);
	v5[0] = 5; v5[12] = 0xA5;
	for (uint8_t i = 0; i < 12; ++i) { v5[12] ^= v5[i]; }
	assert(settings_decode(v5, sizeof(v5), decoded) && decoded.flipped && decoded.face_flow == FACE_RETURN);
	settings_defaults();
}

/** @brief Version five keeps its byte layout, so old mood indices stay valid. */
static void mood_range_tests() {
	assert(EXPRESSION_COUNT > 5);
	CompanionSettings legacy; legacy.mood = LOVE;
	uint8_t legacy_bytes[SETTINGS_BYTES];
	settings_encode(legacy, legacy_bytes);
	assert(legacy_bytes[0] == 6);
	CompanionSettings decoded;
	assert(settings_decode(legacy_bytes, sizeof(legacy_bytes), decoded) && decoded.mood == LOVE);
	// The wider expression range reuses the same byte, so no new profile version is needed.
	CompanionSettings wide; wide.mood = CAT;
	uint8_t wide_bytes[SETTINGS_BYTES];
	settings_encode(wide, wide_bytes);
	assert(settings_decode(wide_bytes, sizeof(wide_bytes), decoded) && decoded.mood == CAT);
	assert(settings_valid(wide));
	CompanionSettings invalid = wide; invalid.mood = EXPRESSION_COUNT;
	assert(!settings_valid(invalid));
	assert(!settings_update(invalid));
	// A freshly written wide profile reloads from flash unchanged.
	assert(settings_init() && settings_save());
	CompanionSettings candidate = settings_get(); candidate.mood = CAT;
	assert(settings_update(candidate) && settings_save());
	assert(settings_init() && settings_get().mood == CAT && eyes_config().expression == CAT);
	settings_defaults();
}

/** @brief Test strict values, partial lines, overflow recovery, and explicit USB save. */
static void usb_tests() {
	CompanionSettings current;
	assert(settings_parse("SET STYLE MOCHI", current).action == UsbAction::Set);
	assert(settings_parse("set color amber", current).profile.color == 3);
	assert(settings_parse("set brightness 91", current).action == UsbAction::Invalid);
	assert(settings_parse("set brightness 2", current).action == UsbAction::Invalid);
	assert(settings_parse("set night-brightness 21", current).action == UsbAction::Invalid);
	assert(settings_parse("set color cyan extra", current).action == UsbAction::Invalid);
	assert(settings_parse("set shake yes", current).action == UsbAction::Invalid);
	assert(settings_parse("set random on", current).profile.random_faces);
	for (uint8_t mode = 0; mode < FACE_FLOW_COUNT; ++mode) {
		String command = String("set face-flow ") + settings_face_flow_name(mode);
		auto parsed = settings_parse(command.c_str(), current);
		assert(parsed.action == UsbAction::Set && parsed.profile.face_flow == mode);
	}
	assert(settings_parse("set face-flow invalid", current).action == UsbAction::Invalid);
	assert(settings_parse("set imu-face on", current).profile.imu_faces);
	assert(settings_parse("set flip on", current).profile.flipped);
	assert(!settings_parse("set flip off", current).profile.flipped);
	assert(settings_parse("set flip maybe", current).action == UsbAction::Invalid);
	assert(settings_parse("set random yes", current).action == UsbAction::Invalid);
	assert(!settings_parse("set route details", current).profile.route_large);
	assert(settings_parse("route large", current).action == UsbAction::RouteView);
	assert(settings_parse("route default", current).route_mode == -1);
	assert(settings_parse("route details extra", current).action == UsbAction::Invalid);
	assert(settings_parse("set route round", current).action == UsbAction::Invalid);
	assert(settings_parse("set sound on", current).profile.sound);
	assert(!settings_parse("set sound-ambient off", current).profile.sound_ambient);
	assert(settings_parse("set sound-level 0", current).action == UsbAction::Invalid);
	assert(settings_parse("set sound-level 31", current).action == UsbAction::Invalid);
	assert(settings_parse("set animation playful", current).profile.animation == 2);
	assert(settings_parse("set animation invalid", current).action == UsbAction::Invalid);
	assert(settings_parse("mood names", current).action == UsbAction::MoodNames);
	assert(settings_parse("mood list", current).action == UsbAction::Invalid);
	assert(settings_parse("mood cat", current).action == UsbAction::Set);
	assert(settings_parse("mood cat", current).profile.mood == CAT);
	assert(settings_parse("MOOD SLEEPY_EYES", current).profile.mood == SLEEPY_EYES);
	assert(settings_parse("set mood star", current).profile.mood == STAR);
	assert(settings_parse("set mood cat", current).profile.mood == CAT);
	assert(settings_parse("set mood sleepy_eyes", current).profile.mood == SLEEPY_EYES);
	assert(settings_parse("set mood not-a-face", current).action == UsbAction::Invalid);
	assert(settings_parse("set mood names", current).action == UsbAction::Invalid);
	assert(settings_parse("set language it", current).profile.language == 1);
	assert(settings_parse("set language fr", current).action == UsbAction::Invalid);
	assert(settings_parse("weather", current).action == UsbAction::WeatherStatus);
	assert(settings_parse("weather sync", current).action == UsbAction::WeatherSync);
	assert(settings_parse("weather sync extra", current).action == UsbAction::Invalid);
	assert(settings_parse("sound", current).action == UsbAction::SoundStatus);
	assert(settings_parse("SOUND TEST", current).action == UsbAction::SoundTest);
	assert(settings_parse("sound test extra", current).action == UsbAction::Invalid);
	assert(settings_parse("reboot", current).action == UsbAction::Reboot);
	assert(settings_parse("reboot now", current).action == UsbAction::Invalid);
	assert(settings_parse("save extra", current).action == UsbAction::Invalid);
	assert(settings_parse(nullptr, current).action == UsbAction::Invalid);
	assert(settings_parse(std::string(96, 'x').c_str(), current).action == UsbAction::Invalid);
	Serial.clear();
	Serial.feed("set style bloub");
	settings_usb_poll();
	assert(settings_get().style == 1);
	Serial.feed("\r\nset color pink\n");
	while (Serial.available()) { settings_usb_poll(); }
	assert(settings_get().style == 2 && settings_get().color == 4);
	Serial.feed(std::string(120, 'x') + "\nshow\n");
	while (Serial.available()) { settings_usb_poll(); }
	assert(Serial.output.find("command too long") != std::string::npos);
	assert(Serial.output.find("color=pink") != std::string::npos);
	// The reboot verb reports before restarting; the restart itself is platform-side.
	Serial.clear(); Serial.feed("reboot\n");
	settings_usb_poll();
	assert(Serial.output.find("restarting") != std::string::npos && ESP.restarted);
	unsigned int writes = test_flash_writes;
	Serial.feed("save\n");
	settings_usb_poll();
	assert(!settings_dirty() && test_flash_writes == writes + 1);
	bool saved_default = settings_get().route_large;
	Serial.feed("route details\n");
	while (Serial.available()) { settings_usb_poll(); }
	assert(!view_route_large(saved_default) && !settings_dirty());
	assert(test_flash_writes == writes + 1 && settings_get().route_large == saved_default);
	Serial.feed("route default\n");
	while (Serial.available()) { settings_usb_poll(); }
	assert(view_route_large(saved_default) == saved_default);
	Serial.clear();
	Serial.feed("sound test\nsound\n");
	while (Serial.available()) { settings_usb_poll(); }
	assert(Serial.output.find("Sound test blocked: off") != std::string::npos);
	assert(Serial.output.find("sound-status=off gpio=33") != std::string::npos);
}

/** @brief Test menu tabs, touch bounds, and explicit saving. */
static void menu_tests() {
	Arduino_GFX gfx;
	settings_defaults();
	settings_menu_draw(&gfx);
	assert(settings_menu_tap(20, 100) == MenuAction::Changed);
	assert(settings_get().style == 2);
	assert(settings_menu_tap(20, 150) == MenuAction::Changed);
	assert(settings_get().color == 1);
	assert(UI_ACCENT == 0x6FBA && UI_TEXT == UI_ACCENT && UI_WARM == UI_ACCENT);
	assert(UI_CARD != UI_BG && UI_MUTED != UI_TEXT);
	assert(!settings_menu_scroll(1));
	assert(settings_menu_tap(210, 65) == MenuAction::Changed);
	settings_menu_draw(&gfx);
	settings_menu_tap(20, 155);
	assert(settings_get().night);
	// DISPLAY keeps brightness controls together; orientation belongs to SENSORS.
	assert(settings_menu_scroll(1) && !settings_menu_scroll(1));
	assert(settings_menu_tap(20, 200) == MenuAction::Changed);
	assert(settings_get().dim);
	assert(settings_menu_tap(210, 65) == MenuAction::Changed);
	settings_menu_draw(&gfx);
	settings_menu_tap(20, 100);
	assert(settings_get().animation == 2);
	settings_menu_tap(20, 140);
	assert(settings_get().random_faces);
	assert(settings_menu_tap(-1, 100) == MenuAction::None);
	assert(!settings_menu_scroll(1));
	for (uint8_t mode = 1; mode <= FACE_FLOW_COUNT; ++mode) {
		assert(settings_menu_tap(20, 202) == MenuAction::Changed);
		assert(settings_get().face_flow == mode % FACE_FLOW_COUNT);
		settings_menu_draw(&gfx);
	}
	assert(settings_menu_tap(240, 119) == MenuAction::None);
	settings_menu_tap(20, 240);
	assert(!settings_dirty());
	settings_menu_tap(20, 100);
	test_save_fails = true;
	settings_menu_tap(20, 240);
	settings_menu_draw(&gfx);
	assert(std::find(gfx.text.begin(), gfx.text.end(), "FAILED") != gfx.text.end());
	test_save_fails = false;
	assert(settings_menu_tap(160, 240) == MenuAction::Exit);
	settings_menu_tap(210, 65);
	settings_menu_tap(20, 100);
	assert(settings_get().imu_faces);
	settings_menu_tap(20, 140);
	assert(settings_get().shake);
	assert(settings_menu_scroll(1) && settings_menu_scroll(1) && !settings_menu_scroll(1));
	settings_menu_tap(20, 202);
	assert(settings_get().flipped);
	settings_menu_tap(210, 65);
	settings_menu_tap(210, 65);
	settings_menu_tap(20, 100);
	assert(settings_get().language == 1 && strcmp(tr(Text::Settings), "IMPOSTAZIONI") == 0);
	settings_menu_tap(20, 202);
	assert(!settings_get().route_large);
	settings_menu_draw(&gfx);
	settings_menu_tap(30, 65);
	settings_menu_tap(20, 100);
	assert(settings_get().sound);
	assert(settings_menu_scroll(1));
	settings_menu_tap(20, 202);
	settings_menu_tap(20, 100);
	assert(!settings_get().sound_faces);
	settings_menu_draw(&gfx);
	assert(settings_menu_tap(160, 240) == MenuAction::Changed);
	settings_menu_draw(&gfx);
	assert(settings_menu_scroll(1) && settings_menu_scroll(1));
	assert(!settings_menu_scroll(1));
	settings_menu_draw(&gfx);
	assert(std::find(gfx.text.begin(), gfx.text.end(), "Prova suoni") != gfx.text.end());
	unsigned int writes = test_flash_writes;
	assert(settings_menu_tap(20, 200) == MenuAction::Changed);
	assert(test_flash_writes == writes);
	assert(settings_menu_scroll(-1) && settings_menu_scroll(-1));
	assert(settings_menu_tap(160, 240) == MenuAction::Exit);
	settings_menu_tap(239, 52);
	settings_menu_tap(239, 52);
	assert(!settings_menu_scroll(-1));
	uint8_t style = settings_get().style;
	assert(settings_menu_tap(239, 131) == MenuAction::Changed);
	assert(settings_get().style == (style + 1) % 4);
	assert(settings_menu_tap(0, 132) == MenuAction::Changed);
	// The LOOK tab's Mood row opens the expression grid instead of cycling five moods.
	assert(settings_menu_tap(239, 221) == MenuAction::Changed);
	assert(settings_menu_grid_open());
	settings_menu_draw(&gfx);
	assert(std::find(gfx.text.begin(), gfx.text.end(), "neutro") != gfx.text.end());
	// Column gutters and out-of-range columns stay inert.
	assert(settings_menu_tap(81, 103) == MenuAction::None);
	assert(settings_menu_tap(238, 100) == MenuAction::None);
	// A real cell selects and applies the expression without writing flash.
	unsigned int grid_writes = test_flash_writes;
	assert(settings_menu_tap(196, 103) == MenuAction::Changed);
	assert(settings_get().mood == SAD && eyes_config().expression == SAD);
	assert(test_flash_writes == grid_writes);
	// Scrolling is bounded and the final page still reaches the last expression.
	uint8_t max_first = uint8_t((EXPRESSION_COUNT + GRID_COLS_HINT - 1) / GRID_COLS_HINT - GRID_PAGE_ROWS_HINT);
	for (uint8_t step = 0; step < max_first; ++step) { assert(settings_menu_scroll(1)); }
	assert(!settings_menu_scroll(1));
	// Empty tail cells on the final page stay inert.
	uint8_t last_column = uint8_t((EXPRESSION_COUNT - 1) % GRID_COLS_HINT);
	for (uint8_t column = last_column + 1; column < GRID_COLS_HINT; ++column) {
		assert(settings_menu_tap(44 + column * 78, 208) == MenuAction::None);
	}
	// The header reports real cell indices, not page numbers, and clamps to the final face.
	settings_menu_draw(&gfx);
	std::string range = std::to_string(uint16_t(max_first * GRID_COLS_HINT + 1)) + "-" +
		std::to_string(EXPRESSION_COUNT) + "/" + std::to_string(EXPRESSION_COUNT);
	assert(std::find(gfx.text.begin(), gfx.text.end(), range) != gfx.text.end());
	assert(settings_menu_tap(44 + last_column * 78, 208) == MenuAction::Changed);
	assert(settings_get().mood == EXPRESSION_COUNT - 1 && eyes_config().expression == EXPRESSION_COUNT - 1);
	assert(settings_menu_scroll(-1));
	// BACK closes the grid and restores the tab rows.
	assert(settings_menu_tap(160, 240) == MenuAction::Changed);
	assert(!settings_menu_grid_open());
	assert(settings_menu_tap(20, 222) == MenuAction::Changed);
	assert(settings_menu_tap(239, 257) == MenuAction::Exit);
	assert(settings_menu_tap(239, 258) == MenuAction::Exit);
	assert(settings_menu_tap(120, 279) == MenuAction::Exit);
	assert(settings_menu_tap(119, 279) == MenuAction::Changed);
	assert(settings_menu_tap(120, 280) == MenuAction::None);
	assert(settings_menu_tap(0, 51) == MenuAction::None);
	assert(settings_menu_tap(72, 52) == MenuAction::None);
	assert(settings_menu_tap(167, 89) == MenuAction::None);
	assert(settings_menu_tap(71, 52) == MenuAction::Changed);
	assert(settings_menu_tap(168, 89) == MenuAction::Changed);
	assert(!settings_menu_scroll(1));
	assert(!settings_menu_scroll(-1));
	settings_defaults();
	settings_apply();
	assert(UI_TEXT == 0xFFFF && UI_ACCENT == 0xFFFF);
	static const uint16_t colors[] = {0xFFFF, 0x6FBA, 0x07FF, 0xFD6C, 0xF9D6, 0x4D7F};
	for (uint8_t color = 0; color < 6; ++color) {
		CompanionSettings profile = settings_get(); profile.color = color;
		assert(settings_update(profile));
		assert(UI_TEXT == colors[color] && UI_ACCENT == colors[color] && UI_WARM == colors[color]);
	}
}

/** @brief Reject truncated/future/corrupt preferences without modifying the supplied output. */
static void format_tests() {
	CompanionSettings p;
	uint8_t bytes[SETTINGS_BYTES];
	settings_encode(p, bytes);
	CompanionSettings decoded; decoded.color = 3;
	assert(!settings_decode(bytes, 8, decoded) && decoded.color == 3);
	assert(!settings_decode(bytes, 11, decoded));
	assert(!settings_decode(nullptr, 12, decoded));
	bytes[7] |= 0x80; bytes[11] ^= 0x80;
	assert(!settings_decode(bytes, sizeof(bytes), decoded));
	settings_encode(p, bytes);
	bytes[10] = 2; bytes[11] ^= 2;
	assert(!settings_decode(bytes, sizeof(bytes), decoded));
	assert(decoded.color == 3);
	// Version-six playback/flip bytes reject reserved bits; versions cannot mix lengths.
	settings_encode(p, bytes);
	bytes[11] |= 8;
	bytes[12] = 0xA5;
	for (uint8_t i = 0; i < 12; ++i) { bytes[12] ^= bytes[i]; }
	assert(!settings_decode(bytes, sizeof(bytes), decoded));
	assert(!settings_decode(bytes, SETTINGS_V4_BYTES, decoded));
}
/** @brief Test stationary hold/tap isolation from swipe motion and rollover. */
static void touch_tests() {
	TouchActions input;
	input.update(true, 100, 100, 0);
	input.update(false, 0, 0, 200);
	assert(input.tap && !input.hold && input.x == 100);
	input.cancel();
	input.update(true, 100, 100, 300);
	input.update(true, 140, 100, 350);
	input.update(false, 0, 0, 400);
	assert(!input.tap && !input.hold);
	input.update(true, 100, 100, UINT32_MAX - 100);
	input.update(true, 100, 100, 1099);
	assert(input.hold && !input.tap);
	input.hold = false;
	input.update(true, 100, 100, 2000);
	assert(!input.hold);
	input.update(false, 0, 0, 2100);
	assert(!input.tap);
	input.cancel();
	input.update(true, 100, 160, 3000);
	input.update(true, 104, 120, 3100);
	assert(input.scroll == 1 && input.vertical && !input.horizontal);
	input.scroll = 0;
	input.update(true, 100, 70, 3200);
	input.update(false, 0, 0, 3300);
	assert(!input.scroll && !input.tap && !input.hold);
	input.update(true, 100, 100, 3400);
	input.update(true, 105, 140, 3500);
	assert(input.scroll == -1 && input.vertical);
	input.cancel();
	input.update(true, 100, 100, 3600);
	input.update(true, 150, 102, 3700);
	input.update(true, 150, 220, 3800);
	assert(input.horizontal && !input.vertical && !input.scroll);
	input.cancel();
	input.update(true, 100, 100, 4000);
	input.update(true, 116, 116, 4500);
	input.update(false, 0, 0, 4700);
	assert(input.tap);
	input.cancel();
	input.update(true, 100, 100, UINT32_MAX - 20);
	input.update(true, 100, 60, 20);
	assert(input.scroll == 1);
	input.cancel();
	assert(!input.scroll && !input.vertical && !input.active);
}

/** @brief Reject single bumps and vibration, require deliberate repeated reversals. */
static void motion_tests() {
	MotionFilter filter;
	assert(filter.update(0.4f, 0, 0.9f, 0, true).x == 0);
	for (uint32_t now = 50; now < 1000; now += 50) {
		MotionSample sample = filter.update(0.41f, 0.01f, 0.9f, now, true);
		assert(sample.x == 0 && sample.y == 0 && !sample.shake);
	}
	assert(!filter.update(0.4f, 0, 3.9f, 1000, true).shake);
	assert(!filter.update(0.4f, 0, 0.9f, 2000, true).shake);
	for (uint32_t now = 2050; now <= 2200; now += 50) {
		MotionSample sample = filter.update(((now / 50) % 2 ? -2.0f : 2.0f) + 0.4f, 0, 0.9f, now, true);
		assert(sample.shake == (now == 2200));
	}
	assert(!filter.update(-2, 0, 0.9f, 2250, true).shake);
	MotionFilter disabled;
	disabled.update(0, 0, 1, 0, false);
	for (uint32_t now = 50; now < 1000; now += 50) {
		assert(!disabled.update(now % 100 ? -3 : 3, 0, 1, now, false).shake);
	}
}

/** @brief Voltage curve, divider plausibility, and the consecutive-sample trust policy. */
static void battery_tests() {
	assert(battery_percent_mv(4200) == 100);
	assert(battery_percent_mv(3000) == 0);
	assert(battery_percent_mv(3820) == 50);
	assert(battery_percent_mv(3850) > 50 && battery_percent_mv(3850) < 60);
	assert(battery_percent_mv(3600) > 10 && battery_percent_mv(3600) < 30);
	// 2643 raw is about 2.0 V at the pin: a plausible 4.0 V cell through the 2:1 divider.
	uint16_t mv = battery_mv_raw(2643);
	assert(mv >= 3950 && mv <= 4050);
	// Railing or floating samples stay out of the plausible LiPo window.
	assert(battery_mv_raw(0) == 0 && battery_mv_raw(4095) == 0);
	assert(battery_mv_raw(1500) == 0);
	// The tracker needs three plausible samples before trusting the pin, five to drop it.
	BatteryTracker tracker;
	tracker.update(2643); tracker.update(2643);
	assert(tracker.percent == BATTERY_UNAVAILABLE);
	tracker.update(2643);
	assert(tracker.percent >= 80 && tracker.percent <= 90);
	tracker.update(0); tracker.update(0); tracker.update(0);
	assert(tracker.percent >= 80);
	tracker.update(0); tracker.update(0);
	assert(tracker.percent == BATTERY_UNAVAILABLE);
	// An unplugged divider (native zero sample) stays "not measured" end to end.
	test_analog_read = 0;
	battery_sample(1000); battery_sample(3100); battery_sample(5100);
	assert(battery_percent() == BATTERY_UNAVAILABLE);
	// A plausible divider reading appears after the priming samples, then tracks.
	test_analog_read = 2643;
	battery_sample(35200); battery_sample(65300); battery_sample(95500);
	assert(battery_percent() >= 80 && battery_percent() <= 90);
	test_analog_read = 0;
	for (uint32_t now = 125600; now <= 246000; now += 30100) { battery_sample(now); }
	assert(battery_percent() == BATTERY_UNAVAILABLE);
}

/** @brief Run preference/input/motion regressions. @return Zero on success. */
int main() {
	persistence_tests();
	migration_tests();
	mood_range_tests();
	usb_tests();
	menu_tests();
	touch_tests();
	motion_tests();
	battery_tests();
	format_tests();
	assert(eyes_set_expression_by_name("SADDOWN"));
	assert(eyes_set_expression_by_name("star"));
	assert(eyes_set_expression_by_name("CAT"));
	assert(eyes_set_expression_by_name("Swirly"));
	assert(eyes_config().expression == SWIRLY);
	assert(!eyes_set_expression_by_name(nullptr));
	assert(!eyes_set_expression_by_name("not-an-expression"));
	assert(!eyes_set_expression_by_name("starr"));
	// Every expression has a stable name that parses back to the same value.
	for (uint8_t i = 0; i < EXPRESSION_COUNT; ++i) {
		assert(eyes_set_expression_by_name(eyes_expression_name(static_cast<Expression>(i))));
		assert(eyes_config().expression == static_cast<Expression>(i));
		assert(strcmp(settings_mood_name(i), eyes_expression_name(static_cast<Expression>(i))) == 0);
		assert(strcmp(tr_expression(i), "?") != 0 && strlen(tr_expression(i)) > 0);
	}
	assert(strcmp(eyes_expression_name(static_cast<Expression>(EXPRESSION_COUNT)), "unknown") == 0);
	assert(strcmp(tr_expression(EXPRESSION_COUNT), "?") == 0);
	std::cout << "Settings persistence, menu, USB parsing, touch, and motion tests passed\n";
}
