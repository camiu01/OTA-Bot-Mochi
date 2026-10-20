/**
 * @file ui_test.cpp
 * @brief Native screen regression tests; no board or BLE connection required.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/ui/screens.h"
#include "../OTA-bot-Mochi/src/ui/auto_page.h"
#include "../OTA-bot-Mochi/src/ui/ui_theme.h"
#include "../OTA-bot-Mochi/src/ui/text_layout.h"
#include "../OTA-bot-Mochi/src/ui/view_state.h"
#include "../OTA-bot-Mochi/src/ui/context_views.h"
#include "../OTA-bot-Mochi/src/ui/i18n.h"
#include "../OTA-bot-Mochi/src/ui/weather_sync.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include "../OTA-bot-Mochi/src/clock/clock_ui.h"
#include "../OTA-bot-Mochi/src/power/brightness_policy.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include <assert.h>
#include <algorithm>
#include <iostream>

/** @brief Test wrapping, clipping, accents, and timestamp rollover. */
static void text_tests() {
	assert(wrap_text(nullptr, 30).count == 0);
	assert(wrap_text("hello", 0).count == 0);
	assert(wrap_text("hello", 35).count == 0);
	TextLines lines = wrap_text("one two three", 7);
	assert(lines.count == 2 && strcmp(lines.line[0], "one two") == 0);
	lines = wrap_text("1234567890\nhello", 4);
	assert(lines.count == 5 && strcmp(lines.line[0], "1234") == 0);
	lines = wrap_text(std::string(1500, 'x').c_str(), 30);
	assert(lines.count == 40 && lines.clipped && strlen(lines.line[39]) <= 30);
	assert(ui_ascii("citt\xC3\xA0, caff\xC3\xA8") == "citta, caffe");
	assert(ui_ascii("hello \xF0\x9F\x98\x80") == "hello ?");
	assert(!weather_stale(100, UINT32_MAX - 100));
	assert(weather_stale(10800000, 0));
}

/** @brief Test idle dimming, touch wake, long holds, and rollover. */
static void brightness_tests() {
	BrightnessPolicy policy;
	assert(policy.output(60000) == 30);
	policy.dim = true;
	assert(policy.output(59999) == 30 && policy.output(60000) == 3);
	policy.update(60001, true, false);
	assert(policy.output(60001) == 30);
	policy.brightness = 1;
	assert(policy.output(120001) == 1);
	policy.last_activity = UINT32_MAX - 100;
	assert(policy.output(100) == 1);
	policy.update(100000, false, true);
	assert(policy.output(100000) == 1);
}

/** @brief Check clock fields and 12/24-hour modes. @param gfx Canvas. @param watch Source. */
static void clock_tests(Arduino_GFX &gfx, ChronosESP32 &watch) {
	assert(screens_draw(&gfx, watch, UiPage::Eyes, 0));
	assert(!screens_draw(&gfx, watch, UiPage::Eyes, 33));
	assert(screens_draw(&gfx, watch, UiPage::Clock, 66));
	assert(!screens_draw(&gfx, watch, UiPage::Clock, 99));
	clock_time_synced();
	ui_config(watch, CF_TIME, 1, 0);
	assert(screens_draw(&gfx, watch, UiPage::Clock, 132));
	assert(std::find(gfx.text.begin(), gfx.text.end(), "13:45") != gfx.text.end());
	assert(!screens_draw(&gfx, watch, UiPage::Clock, 165));
	watch.seconds = ":31";
	assert(screens_draw(&gfx, watch, UiPage::Clock, 198));
	watch.hour24 = false;
	ui_config(watch, CF_HR24, 0, 0);
	screens_draw(&gfx, watch, UiPage::Clock, 200);
	assert(std::find(gfx.text.begin(), gfx.text.end(), "01:45") != gfx.text.end());
	ui_connection(false);
	screens_draw(&gfx, watch, UiPage::Clock, 231);
	assert(std::find(gfx.text.begin(), gfx.text.end(), "PHONE OFFLINE") != gfx.text.end());
}

/** @brief Check missing and stale weather. @param gfx Canvas. @param watch Source. */
static void weather_tests(Arduino_GFX &gfx, ChronosESP32 &watch) {
	screens_draw(&gfx, watch, UiPage::Weather, 264);
	assert(std::find(gfx.text.begin(), gfx.text.end(), "Weather not synced") != gfx.text.end());
	watch.weather_count = 7;
	watch.weather[0].temp = -12;
	test_now = 1000;
	ui_config(watch, CF_WEATHER, 1, 1);
	assert(ui_snapshot().weather_count == 3 && !ui_snapshot().has_forecast);
	ui_config(watch, CF_WEATHER, 2, 0);
	assert(ui_snapshot().has_forecast);
	screens_draw(&gfx, watch, UiPage::Weather, 1000);
	screens_draw(&gfx, watch, UiPage::Weather, 10801000);
	assert(std::find(gfx.text.begin(), gfx.text.end(), "STALE / OPEN CHRONOS") != gfx.text.end());
}

/** @brief Check alerts and call restoration. @param gfx Canvas. @param watch Source. */
static void alert_tests(Arduino_GFX &gfx, ChronosESP32 &watch) {
	screens_draw(&gfx, watch, UiPage::Notification, 10802000);
	Notification notification;
	notification.app = "Messages";
	notification.title = "A long title with multiple words for wrapping on this small display";
	notification.message = std::string(1400, 'x');
	test_now = 10802000;
	ui_notification(notification);
	assert(ui_snapshot().notification.message.length() == 1024);
	screens_draw(&gfx, watch, UiPage::Notification, test_now);
	assert(gfx.text.back() == "1 / 14");
	screens_draw(&gfx, watch, UiPage::Notification, test_now + 6000);
	assert(gfx.text.back() == "2 / 14");
	ui_ringer("Test caller", true);
	screens_draw(&gfx, watch, UiPage::Eyes, test_now + 6033);
	assert(!screens_calling() && !screens_keep_awake());
	screens_draw(&gfx, watch, UiPage::Navigation, test_now + 6040);
	assert(!screens_calling());
	screens_draw(&gfx, watch, UiPage::Settings, test_now + 6041);
	assert(!screens_calling());
	screens_draw(&gfx, watch, UiPage::Notification, test_now + 6045);
	assert(screens_calling() && screens_keep_awake());
	CompanionSettings p = settings_get();
	p.dashboard = false;
	settings_update(p);
	screens_draw(&gfx, watch, UiPage::Eyes, test_now + 6050);
	assert(screens_calling());
	p.dashboard = true;
	settings_update(p);
	screens_draw(&gfx, watch, UiPage::Eyes, test_now + 6055);
	assert(!screens_calling());
	ui_ringer("", false);
	assert(!screens_draw(&gfx, watch, UiPage::Eyes, test_now + 6066));
	assert(!screens_calling());
	assert(!screens_draw(&gfx, watch, UiPage::Eyes, test_now + 6099));
}

/** @brief Check complete navigation icons and disconnect. @param gfx Canvas. @param watch Source. */
static void navigation_tests(Arduino_GFX &gfx, ChronosESP32 &watch) {
	watch.navigation.active = true;
	watch.navigation.hasIcon = true;
	watch.navigation.iconCRC = 123;
	watch.navigation.directions = "Turn right at the next street";
	ui_config(watch, CF_NAV_DATA, 1, 0);
	ui_config(watch, CF_NAV_ICON, 2, 123);
	assert(!ui_snapshot().has_icon);
	ui_config(watch, CF_NAV_ICON, 0, 123);
	ui_config(watch, CF_NAV_ICON, 1, 123);
	watch.navigation.icon[0] = 0xAA;
	ui_config(watch, CF_NAV_ICON, 2, 123);
	assert(ui_snapshot().has_icon && ui_snapshot().icon[0] == 0xAA);
	screens_draw(&gfx, watch, UiPage::Navigation, test_now + 6132);
	assert(screens_keep_awake());
	ui_connection(false);
	screens_draw(&gfx, watch, UiPage::Navigation, test_now + 6165);
	assert(!screens_keep_awake() && !ui_snapshot().navigation.active);
	assert(std::find(gfx.text.begin(), gfx.text.end(), "No active route") != gfx.text.end());
	int16_t labelWidth = 18 * ui_text_size(1); // Three glyph columns at the actual panel's text scale.
	ui_label(&gfx, "overflowing label", gfx.width() - labelWidth - 2, 12, labelWidth, 1);
	assert(gfx.text.back() == "...");
	for (int i = 0; i < 9; ++i) { ui_weather_icon(&gfx, i, 180, 120); }
}

/** @brief Test actual Chronos page events, preserving manual choices on route updates. @param watch Source. */
static void auto_page_tests(ChronosESP32 &watch) {
	AutoPagePolicy policy;
	UiPageEvents events = ui_page_events();
	policy.notifications = events.notifications;
	policy.route_starts = events.route_starts;
	ui_notification(Notification{});
	assert(policy.update(UiPage::Eyes, ui_page_events(), false) == UiPage::Notification);
	assert(policy.update(UiPage::Settings, ui_page_events(), false) == UiPage::Settings);
	ui_config(watch, CF_TIME, 1, 0);
	ui_ringer("Caller", true);
	assert(policy.update(UiPage::Weather, ui_page_events(), false) == UiPage::Weather);
	ui_ringer("", false);
	watch.navigation.active = true; watch.navigation.isNavigation = false;
	ui_config(watch, CF_NAV_DATA, 1, 0);
	assert(policy.update(UiPage::Eyes, ui_page_events(), false) == UiPage::Eyes);
	watch.navigation.isNavigation = true;
	ui_config(watch, CF_NAV_DATA, 1, 0);
	assert(policy.update(UiPage::Clock, ui_page_events(), false) == UiPage::Navigation);
	watch.navigation.title = "Next turn";
	ui_config(watch, CF_NAV_DATA, 1, 0);
	assert(policy.update(UiPage::Eyes, ui_page_events(), false) == UiPage::Eyes);
	ui_notification(Notification{});
	assert(policy.update(UiPage::Navigation, ui_page_events(), false) == UiPage::Notification);
	watch.navigation.active = false;
	ui_config(watch, CF_NAV_DATA, 0, 0);
	ui_notification(Notification{});
	watch.navigation.active = true;
	ui_config(watch, CF_NAV_DATA, 1, 0);
	assert(policy.update(UiPage::Clock, ui_page_events(), false) == UiPage::Navigation);
	ui_connection(false);
	assert(policy.update(UiPage::Weather, ui_page_events(), false) == UiPage::Weather);
	ui_notification(Notification{});
	assert(policy.update(UiPage::Settings, ui_page_events(), true) == UiPage::Settings);
	assert(policy.update(UiPage::Settings, ui_page_events(), false) == UiPage::Notification);
	ui_config(watch, CF_NAV_DATA, 1, 0);
	watch.navigation.active = false;
	ui_config(watch, CF_NAV_DATA, 0, 0);
	assert(policy.update(UiPage::Eyes, ui_page_events(), false) == UiPage::Eyes);
	policy.notifications = UINT32_MAX;
	events = {};
	events.route_starts = policy.route_starts;
	assert(policy.update(UiPage::Eyes, events, false) == UiPage::Notification);
}

/** @brief Session presentation never leaks into saved route defaults; tools stay contextual. */
static void context_tests(Arduino_GFX &gfx, ChronosESP32 &watch) {
	view_reset();
	settings_defaults();
	assert(settings_get().route_large && view_route_large(settings_get().route_large));
	assert(screens_scroll(UiPage::Navigation, -1) == UiPage::Navigation);
	assert(!view_route_large(settings_get().route_large) && settings_get().route_large);
	assert(settings_save() && settings_init() && settings_get().route_large);
	assert(!view_route_large(settings_get().route_large));
	view_reset();
	assert(view_route_large(settings_get().route_large));
	assert(screens_scroll(UiPage::Weather, -1) == UiPage::Weather);
	assert(screens_scroll(UiPage::Notification, -1) == UiPage::Eyes);
	assert(screens_scroll(UiPage::Clock, -1) == UiPage::Desk);
	assert(screens_scroll(UiPage::Clock, 1) == UiPage::Clock);
	assert(screens_scroll(UiPage::Desk, 1) == UiPage::Clock);
	for (uint8_t i = 0; i < 6; ++i) {
		UiPage page = static_cast<UiPage>(uint8_t(UiPage::BigClock) + i);
		assert(screens_tap(UiPage::Desk, i % 2 ? 200 : 20, 60 + (i / 2) * 64, 0) == page);
		assert(screens_parent(page) == UiPage::Clock);
		assert(screens_swipe(page, 1) == UiPage::Navigation);
		assert(screens_swipe(page, -1) == UiPage::Eyes);
		assert(screens_scroll(page, 1) == UiPage::Desk);
		assert(screens_draw(&gfx, watch, page, 20000000 + i * 1000));
	}
	assert(screens_tap(UiPage::Desk, 20, 70, 0) == UiPage::BigClock);
	assert(screens_tap(UiPage::Desk, 200, 98, 0) == UiPage::Overview);
	assert(screens_tap(UiPage::Desk, 239, 211, 0) == UiPage::Desk);
	assert(screens_tap(UiPage::Desk, 240, 98, 0) == UiPage::Desk);
	assert(screens_tap(UiPage::Desk, 20, 252, 0) == UiPage::Desk);
	assert(screens_tap(UiPage::Clock, 120, 250, 0) == UiPage::Clock);
	assert(screens_tap(UiPage::Timer, 120, 180, 0) == UiPage::Timer);
	assert(view_timer().running);
	screens_draw(&gfx, watch, UiPage::Eyes, 1000);
	assert(view_timer().remaining == 1499000 && view_timer().running);
	screens_tap(UiPage::Timer, 20, 180, 1000);
	assert(!view_timer().running && view_timer().remaining == 1499000);
	screens_tap(UiPage::Timer, 20, 230, 1000);
	assert(view_timer().remaining == 1500000);
	screens_tap(UiPage::Timer, 150, 230, 1000);
	assert(view_timer().mode == 0 && view_timer().remaining == 300000);
	view_route_temporary(0);
	screens_draw(&gfx, watch, UiPage::Navigation, 20005000);
	view_route_temporary(1);
	assert(screens_draw(&gfx, watch, UiPage::Navigation, 20005001));
	AutoPagePolicy policy;
	UiPageEvents events; events.route_starts = 1; events.route_active = true;
	assert(policy.update(UiPage::Timer, events, false) == UiPage::Navigation);
}

/** @brief Timers run independently of phone/pages and handle pause, rollover, and large delays. */
static void timer_tests() {
	DeskTimer timer;
	timer.mode = 0; timer.reset(UINT32_MAX - 100); timer.toggle(UINT32_MAX - 100);
	timer.tick(100);
	assert(timer.remaining == 299799);
	timer.toggle(100);
	timer.tick(5000);
	assert(timer.remaining == 299799 && !timer.running);
	timer.toggle(6000); timer.tick(305799);
	assert(timer.finished && !timer.running && timer.remaining == 0);
	timer.mode = 2; timer.reset(0); timer.toggle(0); timer.tick(1500000);
	assert(timer.resting && timer.running && timer.remaining == 300000);
	timer.tick(1800000);
	assert(timer.finished && !timer.running);
	timer.reset(0); timer.toggle(0); timer.tick(2000000);
	assert(timer.finished && timer.remaining == 0);
	timer.cycle(2000001);
	assert(timer.mode == 0 && !timer.running && timer.remaining == 300000);
}

/** @brief Count accent pixels inside the route icon box, excluding the accent distance label. @param gfx Canvas. @return Matching pixels. */
static int route_icon_pixels(const Arduino_GFX &gfx) {
	int count = 0;
	for (int y = 62; y < 158; ++y) {
		for (int x = 72; x < 168; ++x) {
			if (gfx.pixels[size_t(y) * 240 + x] == UI_ACCENT) { ++count; }
		}
	}
	return count;
}

/** @brief Scale only complete matching icon data, without synthesizing arrows. */
static void route_scale_tests() {
	Arduino_GFX gfx;
	UiSnapshot data;
	data.navigation.active = true; data.navigation.hasIcon = true;
	data.navigation.iconCRC = data.icon_crc = 42;
	data.has_icon = true; data.icon[0] = 0x80;
	context_route(&gfx, data);
	assert(gfx.pixels[62 * 240 + 72] == UI_ACCENT);
	assert(gfx.pixels[63 * 240 + 73] == UI_ACCENT);
	assert(route_icon_pixels(gfx) == 4);
	gfx.fillScreen(0);
	data.icon_crc = 43;
	context_route(&gfx, data);
	assert(route_icon_pixels(gfx) == 0);
}

/** @brief Forecast-first packets survive subsequent current updates; empty/invalid data remains missing. */
static void weather_order_tests(ChronosESP32 &watch) {
	watch.weather_count = 0; test_now = 30000000;
	ui_config(watch, CF_WEATHER, 1, 0);
	watch.weather[0].high = 20; watch.weather[0].low = 10;
	ui_config(watch, CF_WEATHER, 2, 0);
	assert(!ui_snapshot().has_weather && !ui_snapshot().has_forecast);
	watch.weather_count = 1; watch.weather[0].temp = 18;
	++test_now; ui_config(watch, CF_WEATHER, 1, 0);
	assert(ui_snapshot().has_weather && ui_snapshot().has_forecast);
	assert(ui_snapshot().weather[0].high == 20);
	++test_now; ui_config(watch, CF_WEATHER, 1, 0);
	assert(ui_snapshot().has_forecast);
	watch.weather[0].temp = 127;
	ui_config(watch, CF_WEATHER, 1, 0);
	assert(!ui_snapshot().has_weather);
	ui_config(watch, CF_WEATHER, 0, 1);
	Serial.clear(); Serial.feed("weather\n");
	settings_usb_poll();
	assert(Serial.output.find("current-packets=") != std::string::npos);
	if (watch.getWeatherCity().length()) { assert(Serial.output.find(watch.getWeatherCity().c_str()) == std::string::npos); }
	WeatherSyncPolicy sync;
	assert(!sync.update(0, true, false) && !sync.update(3999, true, true));
	assert(sync.update(4000, true, true) && !sync.update(4001, true, true));
	sync.request();
	assert(!sync.update(33999, true, true) && sync.update(34000, true, true));
	WeatherSyncPolicy rollover;
	assert(!rollover.update(UINT32_MAX - 100, true, true));
	assert(rollover.update(4000, true, true));
}

/** @brief Both locales have complete, bounded labels and repaint all pages after a live switch. */
static void locale_tests(Arduino_GFX &gfx, ChronosESP32 &watch) {
	for (uint8_t key = 0; key < uint8_t(Text::Count); ++key) {
		assert(strlen(tr_language(static_cast<Text>(key), 0)) && strlen(tr_language(static_cast<Text>(key), 1)));
	}
	assert(strcmp(tr_language(Text::Clock, 255), "CLOCK") == 0);
	assert(strcmp(tr_language(static_cast<Text>(255), 0), "?") == 0);
	auto p = settings_get(); p.language = 1; settings_update(p);
	for (uint8_t page = 0; page < uint8_t(UiPage::Count); ++page) {
		screens_draw(&gfx, watch, static_cast<UiPage>(page), 31000000 + page * 1000);
	}
	assert(std::find(gfx.text.begin(), gfx.text.end(), "IMPOSTAZIONI") != gfx.text.end());
	assert(strcmp(tr_choice(1, 0), "bianco") == 0);
	assert(settings_save() && settings_init() && settings_get().language == 1);
}

/** @brief Run native regression checks. @return Zero on success. */
int main() {
	text_tests();
	brightness_tests();
	assert(ui_state_init());
	assert(settings_init());
	ChronosESP32 watch;
	Arduino_GFX gfx;
	eyes_init(&gfx);
	settings_apply();
	clock_tests(gfx, watch);
	weather_tests(gfx, watch);
	alert_tests(gfx, watch);
	navigation_tests(gfx, watch);
	screens_draw(&gfx, watch, UiPage::Settings, test_now + 6198);
	assert(screens_draw(&gfx, watch, UiPage::Count, test_now + 6231));
	auto_page_tests(watch);
	context_tests(gfx, watch);
	timer_tests();
	route_scale_tests();
	weather_order_tests(watch);
	locale_tests(gfx, watch);
	std::cout << "UI, text layout, snapshots, and brightness tests passed\n";
	return 0;
}
