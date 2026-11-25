/**
 * @file screens.cpp
 * @brief Event-driven cards for music, alerts, weather, routes, and phone calls.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "screens.h"
#include "alert_layout.h"
#include "fonts/classic_bitmap.h"
#include "music_view.h"
#include "ui_theme.h"
#include "save_glow.h"
#include "settings_effects.h"
#include "text_layout.h"
#include "context_views.h"
#include "view_state.h"
#include "i18n.h"
#include "context_play.h"
#include "context_flappy.h"
#include "context_dice.h"
#include "context_drive.h"
#include "context_hubs.h"
#include "../play/dice_input.h"
#include "../play/drive_meter.h"
#include "../clock/clock_ui.h"
#include "../../pin_config.h"
#include "../settings/settings.h"
#include "../settings/settings_menu.h"
#include "../power/backlight.h"
#include "../power/battery.h"
#include "../power/charging.h"
#include "../companion/companion_state.h"

namespace {
UiSnapshot cached;
uint32_t revision = UINT32_MAX;
uint32_t last_refresh = 0;
bool invalidated = true;
bool previous_call = false;
bool overlay_active = false;
bool save_glow_painted = false;
uint32_t preferences_revision = UINT32_MAX;
uint32_t presentation_revision = UINT32_MAX;
uint8_t previous_brightness = 255;
uint8_t previous_battery = BATTERY_UNAVAILABLE;
bool previous_charging = false;
uint32_t companion_revision = UINT32_MAX;
UiPage previous_page = UiPage::Eyes;

/** @brief Invalidate chrome when local power, brightness, presentation or phone tools change. */
void invalidate_status() {
	uint8_t battery = battery_percent();
	bool charging = charging_active();
	uint32_t phone_revision = companion_snapshot().revision;
	if (battery != previous_battery || charging != previous_charging || phone_revision != companion_revision) {
		previous_battery = battery; previous_charging = charging; companion_revision = phone_revision; invalidated = true;
	}
	if (view_revision() != presentation_revision || backlight_brightness() != previous_brightness) {
		presentation_revision = view_revision(); previous_brightness = backlight_brightness(); invalidated = true;
	}
}

/** @brief Draw compact notification text while keeping the selected face. */
void message_line(Arduino_GFX *gfx, const String &text, int16_t x, int16_t y,
	int16_t width, uint8_t size, uint16_t color, uint16_t zoom) {
	if (zoom == 256) { ui_label(gfx, text, x, y, width, size, color); return; }
	if (ui_font::draw(gfx, text, x, y, width, size, color, UI_CARD, false, zoom)) { return; }
	const int16_t factor = ui_text_size(size) * zoom;
	const int16_t advance = ui_font::advance('M', size, false, zoom);
	String ascii = ui_ascii(text);
	ascii.replace('\n', ' ');
	const String fitted = ui_font::fit(ascii, width, size, false, zoom);
	for (unsigned i = 0; i < fitted.length(); ++i) {
		const auto *glyph = bitmap_font5x7 + uint8_t(fitted[i]) * 5;
		for (int16_t col = 0; col < 5; ++col) {
			const int16_t x0 = x + col * factor / 256, x1 = x + (col + 1) * factor / 256;
			for (int16_t row = 0; row < 8; ++row) {
				if (!(glyph[col] & (1 << row))) { continue; }
				const int16_t y0 = y + row * factor / 256, y1 = y + (row + 1) * factor / 256;
				gfx->fillRect(x0, y0, x1 - x0, y1 - y0, color);
			}
		}
		x += advance;
	}
}

/**
 * @brief Draw bounded wrapped text.
 * @param gfx Canvas.
 * @param text Source.
 * @param y Top edge.
 * @param count Maximum lines.
 * @param color Foreground.
 * @param size Font scale.
 * @param columns Wrap width in characters for the chosen scale.
 * @param zoom Font scale as a fraction of 256.
 * @return Number of lines painted.
 */
uint8_t paragraph(Arduino_GFX *gfx, const String &text, int16_t y,
	uint8_t count, uint16_t color, uint8_t size = 1, uint8_t columns = 30, uint16_t zoom = 256) {
	const int16_t cell = 6 * ui_text_size(size) * zoom / 256;
	uint8_t fit = uint8_t((ui_s(184) - 4) / cell);
	if (columns > fit) { columns = fit; }
	TextLines lines = wrap_text(ui_ascii(text).c_str(), columns, ui_s(184) - 4, size, zoom);
	int16_t pitch = ui_s(int16_t(7 * size + 7));
	for (uint8_t i = 0; i < lines.count && i < count; ++i) {
		String line = lines.line[i];
		if (i + 1 == count && lines.count > count) {
			line = (ui_font::active() ? line : line.substring(0, columns - 3)) + "...";
		}
		message_line(gfx, line, ui_s(28), ui_s(y) + i * pitch, ui_s(184), size, color, zoom);
	}
	return lines.count < count ? lines.count : count;
}

/** @brief Align a short alert label in a bounded timestamp, counter or button area. */
void alert_label(Arduino_GFX *gfx, const String &text, int16_t x, int16_t y,
	int16_t width, uint16_t color, bool centered = false) {
	const String fitted = ui_font::fit(ui_ascii(text), width, 1);
	const int16_t used = ui_font::width(fitted, 1);
	const int16_t space = width - used;
	ui_label(gfx, fitted, x + (centered ? space / 2 : space), y, used, 1, color);
}

/** @brief Draw the latest notification with automatic message pagination. */
void notification_screen(Arduino_GFX *gfx, uint32_t now) {
	if (!cached.has_notification) {
		ui_empty(gfx, tr(Text::Quiet), tr(Text::AlertHint));
		return;
	}
	ui_card(gfx, 58, 160);
	const Notification &n = cached.notification;
	constexpr uint16_t zoom = ui_font::Supported ? 224 : 256;
	message_line(gfx, n.app.length() ? n.app : tr(Text::Phone), ui_s(28), ui_s(66), ui_s(126), 2, UI_ACCENT, zoom);
	alert_label(gfx, n.time, ui_s(164), ui_s(70), ui_s(48), UI_MUTED);
	const uint8_t title_lines = paragraph(gfx, n.title, 90, 2, UI_TEXT, 2, 18, zoom);
	const int16_t divider = 94 + 20 * title_lines;
	gfx->drawFastHLine(ui_s(28), ui_s(divider), ui_s(184), UI_LINE);
	const uint8_t columns = uint8_t((ui_s(184) - 4) / (6 * ui_text_size(2) * zoom / 256));
	TextLines lines = wrap_text(ui_ascii(n.message).c_str(), columns, ui_s(184) - 4, 2, zoom);
	const uint8_t per_page = 5 - title_lines;
	uint8_t pages = lines.count ? (lines.count + per_page - 1) / per_page : 1;
	uint8_t page = ((uint32_t)(now - cached.notification_received) / 6000) % pages;
	const uint8_t first = page * per_page;
	for (uint8_t i = 0; i < per_page && first + i < lines.count; ++i) {
		message_line(gfx, lines.line[first + i], ui_s(28), ui_s(divider + 8 + 19 * i), ui_s(184), 2, UI_TEXT, zoom);
	}
	alert_label(gfx, String(cached.notification_index + 1) + "/" + String(cached.notification_count),
		ui_s(166), ui_s(202), ui_s(46), UI_MUTED);
	static const char *arrows[] = {"<", nullptr, ">"};
	for (uint8_t i = 0; i < 3; ++i) {
		const int16_t x = alert_layout::button_left(i);
		ui_button(gfx, x, alert_layout::button_top(), alert_layout::button_width(), alert_layout::button_height(), arrows[i] ? arrows[i] : tr(Text::Dismiss));
	}
	ui_label(gfx, String(page + 1) + " / " + String(pages), ui_s(28), ui_s(202), ui_s(100), 1, UI_MUTED);
}

/**
 * @brief Draw an incoming-call overlay without claiming on-device answering.
 * @param gfx Canvas.
 */
void call_screen(Arduino_GFX *gfx) {
	ui_card(gfx, 62, 152);
	gfx->drawCircle(ui_s(120), ui_s(98), ui_s(28), UI_LINE);
	gfx->fillCircle(ui_s(120), ui_s(98), ui_s(22), UI_WARM);
	ui_center(gfx, tr(Text::CallTag), 95, 1, UI_BG);
	ui_center(gfx, tr(Text::Incoming), 137, 2, UI_TEXT);
	paragraph(gfx, cached.caller.length() ? cached.caller : tr(Text::UnknownCaller), 172, 3, UI_WARM);
	ui_center(gfx, tr(Text::AnswerPhone), 226, 1, UI_MUTED);
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
		gfx->drawRoundRect(ui_s(28), ui_s(80), ui_s(48), ui_s(48), ui_s(10), UI_LINE);
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
		ui_empty(gfx, tr(Text::WeatherMissing), tr(settings_get().phone_ios ? Text::IosWeatherHint : Text::WeatherHint));
		return;
	}
	const Weather &w = cached.weather[0];
	ui_card(gfx, 62, 110);
	ui_label(gfx, cached.city.length() ? cached.city : tr(Text::Weather), ui_s(28), ui_s(76), ui_s(184), 1, UI_ACCENT);
	ui_label(gfx, String(w.temp) + "C", ui_s(28), ui_s(105), ui_s(132), 4, UI_TEXT);
	ui_weather_icon(gfx, w.icon, ui_s(184), ui_s(124));
	bool stale = weather_stale(now, cached.weather_received);
	ui_label(gfx, stale ? tr(cached.weather_internet ? Text::WeatherStaleWifi : Text::Stale) :
		(cached.weather_internet ? String("Open-Meteo ") : String(tr(Text::Updated))) + cached.weather_time,
		ui_s(28), ui_s(151), ui_s(184), 1, stale ? UI_WARM : UI_MUTED);
	ui_card(gfx, 176, 50);
	if (!cached.has_forecast) {
		ui_label(gfx, tr(Text::ForecastPending), ui_s(28), ui_s(190), ui_s(184), 1, UI_MUTED);
		return;
	}
	for (uint8_t i = 0; i < cached.weather_count; ++i) {
		const Weather &day = cached.weather[i];
		int16_t x = ui_s(int16_t(26 + i * 66));
		if (i) { gfx->drawFastVLine(x - ui_s(5), ui_s(184), ui_s(34), UI_LINE); }
		ui_label(gfx, weekday(day.day), x, ui_s(186), ui_s(60), 1, i ? UI_MUTED : UI_ACCENT);
		ui_label(gfx, String(day.high), x, ui_s(206), ui_s(24), 1, UI_TEXT);
		ui_label(gfx, String(day.low), x + ui_s(26), ui_s(206), ui_s(30), 1, UI_MUTED);
	}
}

/** @brief Paint non-clock utility content. @param gfx Canvas. @param watch Clock. @param page Page. @param now Time. */
void content(Arduino_GFX *gfx, ChronosESP32 &watch, UiPage page, uint32_t now) {
	const int16_t height = page == UiPage::Notification ? alert_layout::button_bottom() - ui_s(52) : ui_s(206);
	gfx->fillRect(0, ui_s(52), LCD_WIDTH, height, UI_BG);
	if (overlay_active) { call_screen(gfx); }
	else if (page == UiPage::Notification) { notification_screen(gfx, now); }
	else if (page == UiPage::Navigation) { navigation_screen(gfx); }
	else if (page == UiPage::Weather) { weather_screen(gfx, now); }
	else if (page == UiPage::Music) { music_draw(gfx, cached, now); }
	else if (page == UiPage::Settings) { settings_menu_draw(gfx); }
	else if (page == UiPage::Drive) { drive_home_draw(gfx, now); }
	else if (page == UiPage::DriveHub) { context_hub_draw(gfx, page); }
	else if (screens_drive(page)) { drive_tool_draw(gfx, watch, cached, page, now); }
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

/** @brief Inhibit dimming during routes, calls and the drive dashboard. @return Active critical data. */
bool screens_auto_home(UiPage page, uint32_t idle) {
	if (page == UiPage::Eyes || idle < SCREENS_HOME_IDLE_MS || screens_calling() || screens_drive(page)) { return false; }
	if (page == UiPage::Navigation && cached.navigation.active) { return false; }
	if (page == UiPage::Timer && view_timer().running) { return false; }
	return !(page == UiPage::Settings && settings_dirty());
}

bool screens_keep_awake() { return overlay_active || cached.navigation.active || drive_meter().active; }

/**
 * @brief Repaint on events, battery/charging changes, page changes, or time-dependent fields.
 * @param gfx Canvas.
 * @param watch Chronos clock.
 * @param page Current utility page.
 * @param now Millisecond timestamp.
 * @return Whether a frame needs flushing.
 */
bool screens_draw(Arduino_GFX *gfx, ChronosESP32 &watch, UiPage page, uint32_t now,
	uint16_t *pixels, bool flipped) {
	if (page == UiPage::Phone && settings_get().phone_ios) { page = UiPage::Desk; }
	view_timer().tick(now);
	invalidate_status();
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
	auto &input = dice_input();
	input.active = page == UiPage::Dice && !overlay_active;
	if (input.pending && input.active && previous_page == page && !previous_call) {
		desk_dice().roll([](uint8_t limit) { return random(long(limit)); }); invalidated = true;
	}
	input.pending = false;
	bool drive = screens_drive(page);
	drive_meter().active = drive && !overlay_active;
	obd_want(drive && settings_get().obd_scan);
	bool transition = page != previous_page || overlay_active != previous_call;
	const bool settings_visible = page == UiPage::Settings && !overlay_active;
	const bool slide_frame = settings_menu_animate(now, settings_visible);
	const uint32_t save_elapsed = settings_menu_save_elapsed(now, pixels && settings_visible);
	const bool paint_glow = save_elapsed < save_glow::Duration;
	if (transition || paint_glow || save_glow_painted) {
		gfx->fillScreen(UI_BG); screens_invalidate();
	}
	save_glow_painted = paint_glow;
	if (transition) {
		if (page == UiPage::Pong) { desk_pong().last = now; }
		if (page == UiPage::FlappyBird) { desk_flappy().resume(now); }
	}
	previous_page = page;
	previous_call = overlay_active;
	if (page == UiPage::Eyes && !overlay_active) {
		bool frame = transition || invalidated;
		invalidated = false;
		return frame;
	}
	bool timer = (uint32_t)(now - last_refresh) >=
		((page == UiPage::Pong || page == UiPage::Tris || page == UiPage::FlappyBird) && !overlay_active ? 33U :
		(drive && !overlay_active ? 100U : 1000U));
	if (!invalidated && !changed && !timer && !slide_frame && page != UiPage::Clock) { return false; }
	if (invalidated || changed || transition) {
		static const Text titles[] = {Text::Eyes, Text::Clock, Text::Music, Text::Route, Text::Alerts, Text::Weather, Text::Drive, Text::Settings,
			Text::Desk, Text::BigClock, Text::Overview, Text::Timer, Text::Device, Text::Pong, Text::AnalogClock, Text::Tris, Text::CompanionTitle,
			Text::Games, Text::Dice, Text::FlappyBird,
			Text::DriveHubTitle, Text::DriveBigLean, Text::DriveGMeter, Text::DriveGraphTitle, Text::DriveTrip, Text::DriveObd};
		static_assert(sizeof(titles) / sizeof(titles[0]) == uint8_t(UiPage::Count), "Every page needs a title");
		ui_chrome(gfx, tr(overlay_active ? Text::CallTitle : titles[uint8_t(page)]),
			cached.connected, uint8_t(screens_parent(page)), settings_visible);
		clock_invalidate();
	}
	if (page == UiPage::Clock && !overlay_active) {
		bool painted = clock_draw(gfx, watch, cached.connected);
		bool frame = invalidated || changed || painted;
		invalidated = false;
		return frame;
	}
	content(gfx, watch, page, now);
	if (paint_glow) {
		settings_effects::saved_toast(gfx, pixels, flipped, save_elapsed);
		save_glow::draw(pixels, flipped, save_elapsed);
	}
	last_refresh = now;
	invalidated = false;
	return true;
}
