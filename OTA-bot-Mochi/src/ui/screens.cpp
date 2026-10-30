/**
 * @file screens.cpp
 * @brief Event-driven cards for music, alerts, weather, routes, and phone calls.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "screens.h"
#include "music_view.h"
#include "ui_theme.h"
#include "text_layout.h"
#include "context_views.h"
#include "view_state.h"
#include "i18n.h"
#include "context_play.h"
#include "../clock/clock_ui.h"
#include "../../pin_config.h"
#include "../settings/settings.h"
#include "../settings/settings_menu.h"
#include "../power/backlight.h"

namespace {
UiSnapshot cached;
uint32_t revision = UINT32_MAX;
uint32_t last_refresh = 0;
bool invalidated = true;
bool previous_call = false;
bool overlay_active = false;
uint32_t preferences_revision = UINT32_MAX;
uint32_t presentation_revision = UINT32_MAX;
uint8_t previous_brightness = 255;
UiPage previous_page = UiPage::Eyes;

/**
 * @brief Draw bounded wrapped text.
 * @param gfx Canvas.
 * @param text Source.
 * @param y Top edge.
 * @param count Maximum lines.
 * @param color Foreground.
 * @param size Font scale.
 * @param columns Wrap width in characters for the chosen scale.
 */
void paragraph(Arduino_GFX *gfx, const String &text, int16_t y,
	uint8_t count, uint16_t color, uint8_t size = 1, uint8_t columns = 30) {
	uint8_t fit = uint8_t((ui_s(184) - 4) / (6 * ui_text_size(size)));
	if (columns > fit) { columns = fit; }
	TextLines lines = wrap_text(ui_ascii(text).c_str(), columns);
	int16_t pitch = ui_s(int16_t(7 * size + 7));
	for (uint8_t i = 0; i < lines.count && i < count; ++i) {
		String line = lines.line[i];
		if (i + 1 == count && lines.count > count) { line = line.substring(0, columns - 3) + "..."; }
		ui_label(gfx, line, ui_s(28), ui_s(y) + i * pitch, ui_s(184), size, color);
	}
}

/**
 * @brief Draw the latest notification with automatic message pagination.
 * @param gfx Canvas.
 * @param now Millisecond timestamp.
 */
void notification_screen(Arduino_GFX *gfx, uint32_t now) {
	if (!cached.has_notification) {
		ui_empty(gfx, tr(Text::Quiet), tr(Text::AlertHint));
		return;
	}
	ui_card(gfx, 62, 152);
	const Notification &n = cached.notification;
	ui_label(gfx, n.app.length() ? n.app : tr(Text::Phone), ui_s(28), ui_s(70), ui_s(126), 2, UI_ACCENT);
	ui_label(gfx, n.time, ui_s(164), ui_s(74), ui_s(48), 1, UI_MUTED);
	paragraph(gfx, n.title, 90, 2, UI_TEXT, 2, 15);
	gfx->drawFastHLine(ui_s(28), ui_s(134), ui_s(184), UI_MUTED);
	uint8_t columns = uint8_t((ui_s(184) - 4) / (6 * ui_text_size(2)));
	TextLines lines = wrap_text(ui_ascii(n.message).c_str(), columns);
	uint8_t pages = lines.count ? (lines.count + 2) / 3 : 1;
	uint8_t page = ((uint32_t)(now - cached.notification_received) / 6000) % pages;
	for (uint8_t i = 0; i < 3 && page * 3 + i < lines.count; ++i) {
		ui_label(gfx, lines.line[page * 3 + i], ui_s(28), ui_s(142) + i * ui_s(21), ui_s(184), 2, UI_TEXT);
	}
	ui_label(gfx, String(cached.notification_index + 1) + "/" + String(cached.notification_count),
		ui_s(166), ui_s(210), ui_s(48), 1, UI_MUTED);
	static const char *arrows[] = {"<", nullptr, ">"};
	for (uint8_t i = 0; i < 3; ++i) {
		int16_t x = ui_s(12 + i * 74);
		gfx->fillRoundRect(x, ui_s(224), ui_s(68), ui_s(32), ui_s(6), UI_CARD);
		ui_label(gfx, arrows[i] ? arrows[i] : tr(Text::Dismiss), x + ui_s(6), ui_s(235), ui_s(56), 1, UI_TEXT);
	}
	ui_label(gfx, String(page + 1) + " / " + String(pages), ui_s(28), ui_s(210), ui_s(100), 1, UI_MUTED);
}

/**
 * @brief Draw an incoming-call overlay without claiming on-device answering.
 * @param gfx Canvas.
 */
void call_screen(Arduino_GFX *gfx) {
	ui_card(gfx, 62, 152);
	gfx->drawCircle(ui_s(120), ui_s(95), ui_s(22), UI_WARM);
	ui_label(gfx, tr(Text::CallTag), ui_s(96), ui_s(92), ui_s(48), 1, UI_WARM);
	ui_label(gfx, tr(Text::Incoming), ui_s(28), ui_s(137), ui_s(184), 2, UI_TEXT);
	paragraph(gfx, cached.caller.length() ? cached.caller : tr(Text::UnknownCaller), 172, 3, UI_WARM);
	ui_label(gfx, tr(Text::AnswerPhone), ui_s(28), ui_s(226), ui_s(184), 1, UI_MUTED);
}

/**
 * @brief Draw turn icon, instructions, distance, and estimated arrival.
 * @param gfx Canvas.
 */
void navigation_screen(Arduino_GFX *gfx) {
	if (view_route_large(settings_get().route_large)) { context_route(gfx, cached); return; }
	const Navigation &nav = cached.navigation;
	if (!nav.active) {
		ui_empty(gfx, tr(Text::NoRoute), tr(Text::RouteHint));
		return;
	}
	ui_card(gfx, 62, 98);
	if (nav.hasIcon && cached.has_icon && cached.icon_crc == nav.iconCRC) {
		gfx->drawBitmap(ui_s(28), ui_s(80), cached.icon, 48, 48, UI_ACCENT);
	} else {
		gfx->drawRoundRect(ui_s(28), ui_s(80), ui_s(48), ui_s(48), ui_s(10), UI_MUTED);
		ui_label(gfx, "...", ui_s(42), ui_s(101), ui_s(24), 1, UI_MUTED);
	}
	ui_label(gfx, nav.title, ui_s(88), ui_s(80), ui_s(124), 2, UI_TEXT);
	ui_label(gfx, nav.distance, ui_s(88), ui_s(110), ui_s(124), 1, UI_ACCENT);
	ui_label(gfx, nav.speed, ui_s(88), ui_s(132), ui_s(124), 1, UI_MUTED);
	ui_card(gfx, 174, 60);
	paragraph(gfx, nav.directions, 178, 3, UI_TEXT);
	ui_label(gfx, "ETA " + nav.eta + " / " + nav.duration, ui_s(28), ui_s(220), ui_s(184), 1, UI_MUTED);
}

/**
 * @brief Translate a Chronos weekday.
 * @param day Sunday-based weekday index.
 * @return Short weekday label, or a placeholder for invalid data.
 */
const char *weekday(int day) {
	static const Text days[] = {Text::Sunday, Text::Monday, Text::Tuesday, Text::Wednesday, Text::Thursday, Text::Friday, Text::Saturday};
	return day >= 0 && day < 7 ? tr(days[day]) : "---";
}

/**
 * @brief Draw current conditions and up to three high/low forecasts.
 * @param gfx Canvas.
 * @param now Millisecond timestamp.
 */
void weather_screen(Arduino_GFX *gfx, uint32_t now) {
	if (!cached.has_weather) {
		ui_empty(gfx, tr(Text::WeatherMissing), tr(Text::WeatherHint));
		return;
	}
	const Weather &w = cached.weather[0];
	ui_card(gfx, 62, 110);
	ui_label(gfx, cached.city.length() ? cached.city : tr(Text::Weather), ui_s(28), ui_s(76), ui_s(184), 1, UI_ACCENT);
	ui_label(gfx, String(w.temp) + "C", ui_s(28), ui_s(105), ui_s(132), 4, UI_TEXT);
	ui_weather_icon(gfx, w.icon, ui_s(184), ui_s(124));
	ui_label(gfx, weather_stale(now, cached.weather_received) ? tr(cached.weather_internet ? Text::WeatherStaleWifi : Text::Stale) :
		(cached.weather_internet ? String("Open-Meteo ") : String(tr(Text::Updated))) + cached.weather_time,
		ui_s(28), ui_s(151), ui_s(184), 1, UI_MUTED);
	ui_card(gfx, 176, 50);
	if (!cached.has_forecast) {
		ui_label(gfx, tr(Text::ForecastPending), ui_s(28), ui_s(190), ui_s(184), 1, UI_MUTED);
		return;
	}
	for (uint8_t i = 0; i < cached.weather_count; ++i) {
		const Weather &day = cached.weather[i];
		int16_t x = ui_s(int16_t(26 + i * 66));
		ui_label(gfx, weekday(day.day), x, ui_s(186), ui_s(60), 1, UI_MUTED);
		ui_label(gfx, String(day.high) + "/" + String(day.low), x, ui_s(206), ui_s(60), 1, UI_TEXT);
	}
}

/** @brief Paint non-clock utility content. @param gfx Canvas. @param watch Clock. @param page Page. @param now Time. */
void content(Arduino_GFX *gfx, ChronosESP32 &watch, UiPage page, uint32_t now) {
	gfx->fillRect(0, ui_s(52), LCD_WIDTH, ui_s(206), UI_BG);
	if (overlay_active) { call_screen(gfx); }
	else if (page == UiPage::Notification) { notification_screen(gfx, now); }
	else if (page == UiPage::Navigation) { navigation_screen(gfx); }
	else if (page == UiPage::Weather) { weather_screen(gfx, now); }
	else if (page == UiPage::Music) { music_draw(gfx, cached, now); }
	else if (page == UiPage::Settings) { settings_menu_draw(gfx); }
	else if (uint8_t(page) >= uint8_t(UiPage::Desk)) { context_draw(gfx, watch, page, cached, now); }
}
}

/** @brief Force repaint after a gesture. */
void screens_invalidate() { invalidated = true; clock_invalidate(); }

/** @brief Guard alert actions against asynchronous replacement of the visible entry. @param action Newer/dismiss/older. */
void screens_alert_action(int8_t action) {
	if (overlay_active) { return; }
	if (action == 0) { ui_notification_dismiss(cached.revision); }
	else { ui_notification_step(action, cached.revision); }
	screens_invalidate();
}

/** @brief Expose the cached call flag to the eyes dispatcher. @return Call active. */
bool screens_calling() { return overlay_active; }

/** @brief Inhibit dimming during routes and calls. @return Active critical data. */
bool screens_keep_awake() { return overlay_active || cached.navigation.active; }

/**
 * @brief Repaint on events, page changes, or time-dependent fields.
 * @param gfx Canvas.
 * @param watch Chronos clock.
 * @param page Current utility page.
 * @param now Millisecond timestamp.
 * @return Whether a frame needs flushing.
 */
bool screens_draw(Arduino_GFX *gfx, ChronosESP32 &watch, UiPage page, uint32_t now) {
	view_timer().tick(now);
	if (view_revision() != presentation_revision || backlight_brightness() != previous_brightness) {
		presentation_revision = view_revision();
		previous_brightness = backlight_brightness(); invalidated = true;
	}
	if (uint8_t(page) >= uint8_t(UiPage::Count)) { page = UiPage::Eyes; }
	uint32_t next_revision = ui_revision();
	bool changed = next_revision != revision;
	if (changed) { cached = ui_snapshot(); revision = cached.revision; }
	uint32_t next_preferences = settings_revision();
	if (next_preferences != preferences_revision) {
		preferences_revision = next_preferences;
		invalidated = true;
	}
	overlay_active = cached.calling && (!settings_get().dashboard || page == UiPage::Notification);
	bool transition = page != previous_page || overlay_active != previous_call;
	if (transition) {
		gfx->fillScreen(UI_BG); screens_invalidate();
		if (page == UiPage::Pong) { desk_pong().last = now; }
	}
	previous_page = page;
	previous_call = overlay_active;
	if (page == UiPage::Eyes && !overlay_active) {
		bool frame = transition || invalidated;
		invalidated = false;
		return frame;
	}
	bool timer = (uint32_t)(now - last_refresh) >=
		((page == UiPage::Pong || page == UiPage::Tris) && !overlay_active ? 33U : 1000U);
	if (!invalidated && !changed && !timer && page != UiPage::Clock) { return false; }
	if (invalidated || changed || transition) {
		static const Text titles[] = {Text::Eyes, Text::Clock, Text::Music, Text::Route, Text::Alerts, Text::Weather, Text::Settings,
			Text::Desk, Text::BigClock, Text::Overview, Text::Timer, Text::Device, Text::Pong, Text::AnalogClock, Text::Tris};
		ui_chrome(gfx, tr(overlay_active ? Text::CallTitle : titles[uint8_t(page)]),
			cached.connected, uint8_t(screens_parent(page)));
		clock_invalidate();
	}
	if (page == UiPage::Clock && !overlay_active) {
		bool painted = clock_draw(gfx, watch, cached.connected);
		bool frame = invalidated || changed || painted;
		invalidated = false;
		return frame;
	}
	content(gfx, watch, page, now);
	last_refresh = now;
	invalidated = false;
	return true;
}
