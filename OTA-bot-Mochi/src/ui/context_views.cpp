/**
 * @file context_views.cpp
 * @brief Quiet desk utilities and essential turn display with bounded typography.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "context_views.h"
#include "view_state.h"
#include "ui_theme.h"
#include "text_layout.h"
#include "i18n.h"
#include "weather_sync.h"
#include "../sound/sound.h"
#include "../clock/clock_ui.h"
#include "../eyes/eyes_api.h"
#include "../settings/settings.h"
#include "../power/backlight.h"
#include "../power/battery.h"
#include <stdio.h>

namespace {
/** @brief Draw a bounded action button. @param gfx Canvas. @param x Left. @param y Top. @param w Width. @param label Text. @param h Height. @param accent Accent fill for primary actions. */
void button(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, const char *label, int16_t h = 30, bool accent = false) {
	gfx->fillRoundRect(x, y, w, h, 8, accent ? UI_ACCENT : UI_CARD);
	ui_label(gfx, label, x + 10, y + (h - 8) / 2, w - 20, 1, accent ? UI_BG : UI_TEXT);
}
/** @brief Return valid or waiting time. @param watch Clock. @return HH:MM. */
String time_text(ChronosESP32 &watch) {
	return clock_ready() ? watch.getHourZ() + watch.getTime(":%M") : "--:--";
}
/** @brief Render desk hub as four large grid cards. @param gfx Canvas. */
void hub(Arduino_GFX *gfx) {
	static const Text labels[] = {Text::BigClock, Text::Overview, Text::TimerMenu, Text::Device};
	for (uint8_t i = 0; i < 4; ++i) {
		int16_t x = i % 2 ? 126 : 6;
		int16_t y = 58 + (i / 2) * 98;
		gfx->fillRoundRect(x, y, 108, 92, 14, UI_CARD);
		TextLines lines = wrap_text(tr(labels[i]), 15);
		int16_t top = y + 50 - int16_t(lines.count ? lines.count : 1) * 8;
		for (uint8_t l = 0; l < lines.count && l < 2; ++l) {
			ui_label(gfx, lines.line[l], x + 10, top + l * 16, 88, 1, UI_TEXT);
		}
	}
}
/** @brief Render large synchronized time. @param gfx Canvas. @param watch Clock. */
void large_clock(Arduino_GFX *gfx, ChronosESP32 &watch) {
	ui_card(gfx, 62, 152);
	ui_label(gfx, time_text(watch), 30, 99, 180, 6, UI_TEXT);
	ui_label(gfx, clock_ready() ? watch.getTime(":%S") + " " + watch.getAmPmC() :
		tr(Text::OpenSync), 28, 162, 184, 1, UI_ACCENT);
	ui_label(gfx, clock_ready() ? watch.getTime("%d / %m / %Y") : tr(Text::TimeUnavailable),
		28, 190, 184, 1, UI_MUTED);
}
/** @brief Render compact desk summary with missing/stale weather. @param gfx Canvas. @param watch Clock. @param data Snapshot. @param now Time. */
void overview(Arduino_GFX *gfx, ChronosESP32 &watch, const UiSnapshot &data, uint32_t now) {
	ui_card(gfx, 62, 152);
	ui_label(gfx, time_text(watch), 45, 78, 150, 5, UI_TEXT);
	ui_label(gfx, tr(data.connected ? Text::Connected : Text::Offline), 28, 130, 184, 1, UI_MUTED);
	if (!data.has_weather) { ui_label(gfx, tr(Text::WeatherMissing), 28, 160, 184, 1, UI_MUTED); return; }
	ui_label(gfx, String(data.weather[0].temp) + " C / " + data.city, 28, 158, 184, 2, UI_ACCENT);
	ui_label(gfx, weather_stale(now, data.weather_received) ? tr(Text::Stale) :
		tr(Text::Updated) + data.weather_time, 28, 195, 184, 1, UI_MUTED);
}
/** @brief Render silent, local timer controls. @param gfx Canvas. */
void timer_screen(Arduino_GFX *gfx) {
	const DeskTimer &timer = view_timer();
	ui_card(gfx, 56, 92);
	uint32_t seconds = (timer.remaining + 999) / 1000;
	char text[12];
	snprintf(text, sizeof(text), "%02lu:%02lu", (unsigned long)(seconds / 60), (unsigned long)(seconds % 60));
	ui_label(gfx, tr(timer.finished ? Text::DoneQuiet : (timer.resting ? Text::Rest : Text::Focus)),
		28, 70, 184, 1, UI_ACCENT);
	ui_label(gfx, text, 60, 92, 120, 4, UI_TEXT);
	button(gfx, 6, 154, 228, tr(timer.running ? Text::Pause : Text::Start), 50, true);
	button(gfx, 6, 212, 108, tr(Text::Reset), 48);
	button(gfx, 126, 212, 108, tr(timer.mode == 0 ? Text::Timer5 :
		(timer.mode == 1 ? Text::Timer15 : Text::Pomodoro)), 48);
}
/** @brief Render non-sensitive device information. @param gfx Canvas. @param data Snapshot. */
void device(Arduino_GFX *gfx, const UiSnapshot &data) {
	ui_card(gfx, 62, 152);
	ui_label(gfx, tr(data.connected ? Text::BleConnected : Text::BleOffline), 28, 72, 184, 1, UI_TEXT);
	ui_label(gfx, tr(Text::LightPrefix) + String(backlight_brightness()) + "%", 28, 90, 184, 1, UI_TEXT);
	ui_label(gfx, tr(eyes_imu_ready() ? Text::ImuReady : Text::ImuMissing), 28, 108, 184, 1, UI_TEXT);
	ui_label(gfx, tr(clock_ready() ? Text::ClockSynced : Text::ClockWaiting), 28, 126, 184, 1, UI_TEXT);
	ui_label(gfx, tr(settings_dirty() ? Text::PrefsUnsaved : Text::PrefsSaved), 28, 144, 184, 1, UI_MUTED);
	static const Text audio[] = {Text::SoundOff, Text::SoundNight, Text::SoundArmed, Text::SoundReady, Text::SoundError};
	ui_label(gfx, tr(audio[uint8_t(sound_status())]), 28, 162, 184, 1, UI_MUTED);
	uint8_t charge = battery_percent();
	ui_label(gfx, tr(Text::BatteryPrefix) +
		(charge == BATTERY_UNAVAILABLE ? tr(Text::BatteryMissing) : String(charge) + "%"),
		28, 180, 184, 1, charge == BATTERY_UNAVAILABLE ? UI_MUTED : UI_TEXT);
}
}

/** @brief Draw large turn/distance without guessing missing icons. @param gfx Canvas. @param data Snapshot. */
void context_route(Arduino_GFX *gfx, const UiSnapshot &data) {
	const Navigation &nav = data.navigation;
	if (!nav.active) { ui_empty(gfx, tr(Text::NoRoute), tr(Text::RouteHint)); return; }
	if (nav.hasIcon && data.has_icon && data.icon_crc == nav.iconCRC) {
		for (uint8_t y = 0; y < 48; ++y) {
			for (uint8_t x = 0; x < 48; ++x) {
				if (data.icon[y * 6 + x / 8] & (0x80 >> (x % 8))) {
					gfx->fillRect(72 + x * 2, 62 + y * 2, 2, 2, UI_ACCENT);
				}
			}
		}
	} else {
		gfx->drawRoundRect(72, 62, 96, 96, 14, UI_MUTED);
		ui_label(gfx, "...", 102, 100, 42, 2, UI_MUTED);
	}
	String distance = nav.distance.length() ? nav.distance : "--";
	ui_label(gfx, distance, 28, 164, 184, ui_ascii(distance).length() <= 10 ? 3 : 2, UI_ACCENT);
	TextLines lines = wrap_text(ui_ascii(nav.directions.length() ? nav.directions : nav.title).c_str(), 15);
	for (uint8_t i = 0; i < lines.count && i < 2; ++i) {
		String line = lines.line[i];
		if (i == 1 && lines.count > 2) { line = line.substring(0, 12) + "..."; }
		ui_label(gfx, line, 28, 194 + i * 18, 184, 2, UI_TEXT);
	}
}

/** @brief Draw a desk tool page. @param gfx Canvas. @param watch Clock. @param page Tool. @param data Snapshot. @param now Time. */
void context_draw(Arduino_GFX *gfx, ChronosESP32 &watch, UiPage page, const UiSnapshot &data, uint32_t now) {
	if (page == UiPage::Desk) { hub(gfx); }
	if (page == UiPage::BigClock) { large_clock(gfx, watch); }
	if (page == UiPage::Overview) { overview(gfx, watch, data, now); }
	if (page == UiPage::Timer) { timer_screen(gfx); }
	if (page == UiPage::Device) { device(gfx, data); }
}

/** @brief Dispatch short parked-use utility taps without persisting session choices. @param page Page. @param x X. @param y Y. @param now Time. @return Target. */
UiPage screens_tap(UiPage page, int16_t x, int16_t y, uint32_t now) {
	if (x < 0 || x >= 240 || y < 0 || y >= 280) { return page; }
	if (page == UiPage::Desk && y >= 52 && y < 252) {
		return static_cast<UiPage>(uint8_t(UiPage::BigClock) + ((y - 52) / 100) * 2 + (x >= 120));
	}
	if (page == UiPage::Timer) {
		if (y >= 152 && y < 208) { view_timer().toggle(now); screens_invalidate(); }
		if (y >= 208 && y < 262 && x < 120) { view_timer().reset(now); screens_invalidate(); }
		if (y >= 208 && y < 262 && x >= 120) { view_timer().cycle(now); screens_invalidate(); }
	}
	return page;
}

/** @brief Dispatch vertical gestures without persisting session choices. @param page Page. @param scroll +1 up, -1 down. @return Target. */
UiPage screens_scroll(UiPage page, int8_t scroll) {
	if (!scroll) { return page; }
	if (scroll > 0) {
		if (page == UiPage::Desk) { return UiPage::Clock; }
		if (uint8_t(page) > uint8_t(UiPage::Desk)) { return UiPage::Desk; }
		return page;
	}
	if (page == UiPage::Clock) { return UiPage::Desk; }
	if (page == UiPage::Navigation) {
		view_route_temporary(!view_route_large(settings_get().route_large));
		screens_invalidate();
		return page;
	}
	if (page == UiPage::Weather) { weather_request(); screens_invalidate(); return page; }
	return UiPage::Eyes;
}
