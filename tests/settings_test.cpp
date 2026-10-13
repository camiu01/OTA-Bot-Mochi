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
#include "../OTA-bot-Mochi/src/ui/view_state.h"
#include "../OTA-bot-Mochi/src/ui/i18n.h"
#include "../OTA-bot-Mochi/src/ui/ui_theme.h"
#include <Preferences.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

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
	assert(settings_dirty() && settings_save() && test_profile[0] == 4 && test_profile.size() == SETTINGS_BYTES);
	bytes[0] = 2; bytes[6] = 0x32; bytes[7] = 0xA5;
	for (uint8_t i = 0; i < 7; ++i) { bytes[7] ^= bytes[i]; }
	assert(settings_decode(bytes, sizeof(bytes), decoded) && decoded.random_faces && decoded.imu_faces && decoded.route_large);
	bytes[0] = 3; bytes[7] = 0xA5;
	for (uint8_t i = 0; i < 7; ++i) { bytes[7] ^= bytes[i]; }
	assert(settings_decode(bytes, sizeof(bytes), decoded) && !decoded.route_large && !decoded.sound);
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
	assert(settings_parse("set imu-face on", current).profile.imu_faces);
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
	assert(settings_parse("set language it", current).profile.language == 1);
	assert(settings_parse("set language fr", current).action == UsbAction::Invalid);
	assert(settings_parse("weather", current).action == UsbAction::WeatherStatus);
	assert(settings_parse("weather sync", current).action == UsbAction::WeatherSync);
	assert(settings_parse("weather sync extra", current).action == UsbAction::Invalid);
	assert(settings_parse("sound", current).action == UsbAction::SoundStatus);
	assert(settings_parse("SOUND TEST", current).action == UsbAction::SoundTest);
	assert(settings_parse("sound test extra", current).action == UsbAction::Invalid);
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
	assert(settings_menu_scroll(1));
	assert(!settings_menu_scroll(1));
	settings_menu_tap(20, 202);
	assert(!settings_get().route_large);
	assert(settings_menu_tap(210, 65) == MenuAction::Changed);
	settings_menu_draw(&gfx);
	settings_menu_tap(20, 134);
	assert(settings_get().night);
	assert(settings_menu_tap(210, 65) == MenuAction::Changed);
	settings_menu_draw(&gfx);
	settings_menu_tap(20, 134);
	assert(settings_get().shake);
	settings_menu_tap(20, 185);
	assert(settings_menu_scroll(1));
	settings_menu_tap(20, 202);
	assert(settings_get().random_faces && settings_get().imu_faces);
	assert(settings_menu_tap(-1, 100) == MenuAction::None);
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
	assert(!settings_menu_scroll(1));
	settings_menu_tap(20, 100);
	assert(settings_get().animation == 2);
	settings_menu_tap(20, 134);
	assert(settings_get().language == 1 && strcmp(tr(Text::Settings), "IMPOSTAZIONI") == 0);
	settings_menu_draw(&gfx);
	settings_menu_tap(210, 65);
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
	assert(!settings_menu_scroll(-1));
	uint8_t style = settings_get().style;
	assert(settings_menu_tap(239, 131) == MenuAction::Changed);
	assert(settings_get().style == (style + 1) % 4);
	assert(settings_menu_tap(0, 132) == MenuAction::Changed);
	assert(settings_menu_tap(239, 221) == MenuAction::Changed);
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
	assert(settings_menu_scroll(1));
	assert(settings_menu_scroll(-1));
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

/** @brief Run preference/input/motion regressions. @return Zero on success. */
int main() {
	persistence_tests();
	migration_tests();
	usb_tests();
	menu_tests();
	touch_tests();
	motion_tests();
	format_tests();
	assert(eyes_set_expression_by_name("SADDOWN"));
	assert(!eyes_set_expression_by_name(nullptr));
	assert(!eyes_set_expression_by_name("not-an-expression"));
	std::cout << "Settings persistence, menu, USB parsing, touch, and motion tests passed\n";
}
