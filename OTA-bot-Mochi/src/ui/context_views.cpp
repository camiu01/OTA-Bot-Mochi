/**
 * @file context_views.cpp
 * @brief Quiet desk utilities and essential turn display with bounded typography.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "context_views.h"
#include "../companion/phone_view.h"
#include "music_view.h"
#include "view_state.h"
#include "ui_theme.h"
#include "text_layout.h"
#include "i18n.h"
#include "weather_sync.h"
#include "context_play.h"
#include "context_tris.h"
#include "context_hubs.h"
#include "context_dice.h"
#include "context_flappy.h"
#include "context_drive.h"
#include "../sound/sound.h"
#include "../clock/clock_ui.h"
#include "../eyes/eyes_api.h"
#include "../settings/settings.h"
#include "../power/backlight.h"
#include "../power/battery.h"
#include "../power/charging.h"
#include "../../pin_config.h"
#include <stdio.h>

namespace {
UiPage last_drive_tool = UiPage::DriveHub;	/**< Session-only target of a tap on the Drive home. */
/** @brief Draw a bounded action button. @param gfx Canvas. @param x Left (design). @param y Top (design). @param w Width (design). @param label Text. @param h Height (design). @param accent Accent fill for primary actions. */
void button(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, const char *label, int16_t h = 30, bool accent = false) {
	ui_button(gfx, ui_s(x), ui_s(y), ui_s(w), ui_s(h), label, accent);
}
/** @brief Return valid or waiting time. @param watch Clock. @return HH:MM. */
String time_text(ChronosESP32 &watch) {
	return clock_ready() ? watch.getHourZ() + watch.getTime(":%M") : "--:--";
}
/**
 * @brief Geometry of the timer's reset/mode button row.
 *
 * Doubled labels outgrow the symmetric design halves on scaled panels, so the
 * mode button borrows columns from reset and the tap split follows the seam.
 * The reference panel keeps the original 6/108, 126/108, split 120 layout.
 * @return Reset width, mode left/width, and the tap split in design pixels.
 */
struct TimerRow { int16_t reset_w, mode_x, mode_w, split; };
TimerRow timer_row() {
	return ui_scale() > 100 ? TimerRow{78, 96, 138, 90} : TimerRow{108, 126, 108, 120};
}
/** @brief Render large synchronized time. @param gfx Canvas. @param watch Clock. */
void large_clock(Arduino_GFX *gfx, ChronosESP32 &watch) {
	ui_card(gfx, 62, 152);
	ui_center(gfx, time_text(watch), 92, 6, UI_TEXT);
	if (clock_ready()) {
		// Thin seconds sweep under the digits instead of a second text line.
		ui_bar(gfx, ui_s(40), ui_s(152), LCD_WIDTH - 2 * ui_s(40), ui_s(4), watch.getTime("%S").toInt() * 1000L / 59, UI_ACCENT);
		ui_center(gfx, watch.getTime("%d / %m / %Y") + " " + watch.getAmPmC(), 172, 1, UI_TEXT);
		return;
	}
	ui_center(gfx, tr(settings_get().phone_ios ? Text::IosTimeHint : Text::OpenSync), 162, 1, UI_ACCENT);
	ui_center(gfx, tr(Text::TimeUnavailable), 190, 1, UI_MUTED);
}
/** @brief Render compact desk summary with missing/stale weather. @param gfx Canvas. @param watch Clock. @param data Snapshot. @param now Time. */
void overview(Arduino_GFX *gfx, ChronosESP32 &watch, const UiSnapshot &data, uint32_t now) {
	ui_card(gfx, 62, 152);
	ui_center(gfx, time_text(watch), 76, 5, UI_TEXT);
	ui_status(gfx, 128, data.connected, tr(data.connected ? Text::Connected : Text::Offline));
	gfx->drawFastHLine(ui_s(28), ui_s(146), LCD_WIDTH - 2 * ui_s(28), UI_LINE);
	if (!data.has_weather) { ui_label(gfx, tr(Text::WeatherMissing), ui_s(28), ui_s(160), ui_s(184), 1, UI_MUTED); return; }
	ui_weather_icon(gfx, data.weather[0].icon, ui_s(52), ui_s(178));
	ui_label(gfx, String(data.weather[0].temp) + " C", ui_s(84), ui_s(158), ui_s(128), 2, UI_TEXT);
	ui_label(gfx, data.city, ui_s(84), ui_s(178), ui_s(128), 1, UI_MUTED);
	bool stale = weather_stale(now, data.weather_received);
	ui_label(gfx, stale ? tr(Text::Stale) : tr(Text::Updated) + data.weather_time, ui_s(84), ui_s(194), ui_s(128), 1,
		stale ? UI_WARM : UI_MUTED);
}
/** @brief Render silent, local timer controls. @param gfx Canvas. */
void timer_screen(Arduino_GFX *gfx) {
	const DeskTimer &timer = view_timer();
	ui_card(gfx, 56, 92);
	bool watch = timer.mode == DeskTimer::STOPWATCH;
	unsigned long seconds = watch ? timer.remaining / 1000 : (timer.remaining + 999) / 1000;
	char text[12];
	if (seconds >= 3600) { snprintf(text, sizeof(text), "%lu:%02lu:%02lu", seconds / 3600, seconds / 60 % 60, seconds % 60); }
	else { snprintf(text, sizeof(text), "%02lu:%02lu", seconds / 60, seconds % 60); }
	ui_center(gfx, tr(timer.finished ? Text::DoneQuiet : (watch ? Text::Elapsed : (timer.resting ? Text::Rest : Text::Focus))),
		66, 1, timer.finished ? UI_WARM : UI_MUTED);
	ui_center(gfx, text, 84, 4, timer.running ? UI_TEXT : UI_MUTED);
	if (!watch) {	// Countdown share left in the current phase.
		static const uint32_t length[] = {300000, 900000, 1500000};
		uint32_t total = timer.resting ? 300000 : length[timer.mode];
		ui_bar(gfx, ui_s(28), ui_s(132), LCD_WIDTH - 2 * ui_s(28), ui_s(5), int32_t(uint64_t(timer.remaining) * 1000 / total), UI_ACCENT);
	}
	TimerRow bottom = timer_row();
	static const Text modes[DeskTimer::MODES] = {Text::Timer5, Text::Timer15, Text::Pomodoro, Text::Stopwatch};
	button(gfx, 6, 154, 228, tr(timer.running ? Text::Pause : Text::Start), 50, true);
	button(gfx, 6, 212, bottom.reset_w, tr(Text::Reset), 48);
	button(gfx, bottom.mode_x, 212, bottom.mode_w, tr(modes[timer.mode]), 48);
}
/** @brief Render non-sensitive device information. @param gfx Canvas. @param data Snapshot. */
void device(Arduino_GFX *gfx, const UiSnapshot &data) {
	ui_card(gfx, 62, 152);
	ui_status(gfx, 72, data.connected, tr(data.connected ? Text::BleConnected : Text::BleOffline));
	ui_status(gfx, 90, true, tr(Text::LightPrefix) + String(backlight_brightness()) + "%");
	ui_status(gfx, 108, eyes_imu_ready(), tr(eyes_imu_ready() ? Text::ImuReady : Text::ImuMissing));
	ui_status(gfx, 126, clock_ready(), tr(clock_ready() ? Text::ClockSynced : Text::ClockWaiting));
	ui_status(gfx, 144, !settings_dirty(), tr(settings_dirty() ? Text::PrefsUnsaved : Text::PrefsSaved));
	static const Text audio[] = {Text::SoundOff, Text::SoundNight, Text::SoundArmed, Text::SoundReady, Text::SoundError};
	SoundStatus sound = sound_status();
	ui_status(gfx, 162, sound == SoundStatus::Armed || sound == SoundStatus::Ready, tr(audio[uint8_t(sound)]));
	uint8_t charge = battery_percent();
	ui_status(gfx, 180, charge != BATTERY_UNAVAILABLE, tr(Text::BatteryPrefix) +
		(charge == BATTERY_UNAVAILABLE ? tr(Text::BatteryMissing) : String(charge) + "%"));
	if (charge != BATTERY_UNAVAILABLE) { ui_bar(gfx, ui_s(44), ui_s(196), ui_s(150), ui_s(4), charge * 10, charge < 20 ? UI_WARM : UI_ACCENT); }
	if (charging_active()) { ui_charge_badge(gfx, ui_s(206), ui_s(176)); }
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
					gfx->fillRect(ui_s(int16_t(72 + x * 2)), ui_s(int16_t(62 + y * 2)), ui_s(2), ui_s(2), UI_ACCENT);
				}
			}
		}
	} else {
		gfx->drawRoundRect(ui_s(72), ui_s(62), ui_s(96), ui_s(96), ui_s(14), UI_LINE);
		ui_label(gfx, "...", ui_s(102), ui_s(100), ui_s(42), 2, UI_MUTED);
	}
	String distance = nav.distance.length() ? nav.distance : "--";
	ui_label(gfx, distance, ui_s(28), ui_s(164), ui_s(184), ui_ascii(distance).length() <= 10 ? 3 : 2, UI_ACCENT);
	uint8_t columns = uint8_t((ui_s(184) - 4) / (6 * ui_text_size(2)));
	TextLines lines = wrap_text(ui_ascii(nav.directions.length() ? nav.directions : nav.title).c_str(), columns);
	for (uint8_t i = 0; i < lines.count && i < 2; ++i) {
		String line = lines.line[i];
		if (i == 1 && lines.count > 2) { line = line.substring(0, columns - 3) + "..."; }
		ui_label(gfx, line, ui_s(28), ui_s(194) + i * ui_s(18), ui_s(184), 2, UI_TEXT);
	}
}

/** @brief Draw local desk tools or the Android-only phone tools. @param gfx Canvas. @param watch Clock. @param page Tool. @param data Snapshot. @param now Time. */
void context_draw(Arduino_GFX *gfx, ChronosESP32 &watch, UiPage page, const UiSnapshot &data, uint32_t now) {
	if (page == UiPage::Desk || page == UiPage::Games) { context_hub_draw(gfx, page); }
	if (page == UiPage::Dice) { desk_dice_draw(gfx); }
	if (page == UiPage::FlappyBird) { desk_flappy_draw(gfx, now); }
	if (page == UiPage::BigClock) { large_clock(gfx, watch); }
	if (page == UiPage::Overview) { overview(gfx, watch, data, now); }
	if (page == UiPage::Timer) { timer_screen(gfx); }
	if (page == UiPage::Device) { device(gfx, data); }
	if (page == UiPage::Pong) { desk_pong_draw(gfx, now); }
	if (page == UiPage::AnalogClock) { desk_analog_draw(gfx, watch); }
	if (page == UiPage::Tris) { desk_tris_draw(gfx, now); }
	if (page == UiPage::Phone) { phone_view_draw(gfx, watch.getDayofYear()); }
}

/** @brief Dispatch short parked-use utility taps without persisting session choices. @param page Page. @param x X. @param y Y. @param now Time. @return Target. */
UiPage screens_tap(UiPage page, int16_t x, int16_t y, uint32_t now) {
	if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) { return page; }
	if (page == UiPage::Notification && y >= ui_s(224) && y < ui_s(256)) {
		for (uint8_t i = 0; i < 3; ++i) {
			int16_t left = ui_s(12 + i * 74);
			if (x < left || x >= left + ui_s(68)) { continue; }
			screens_alert_action(int8_t(i) - 1);
		}
	}
	if (page == UiPage::Music && !screens_calling()) { music_tap(x, y); }
	if (page == UiPage::Desk || page == UiPage::Games || page == UiPage::DriveHub) {
		UiPage target = context_hub_tap(page, x, y);
		if (page == UiPage::DriveHub && target != page) { last_drive_tool = target; }
		return target;
	}
	// The Drive home has no controls: a tap reopens the last tool (the tool menu at first).
	if (page == UiPage::Drive && y >= ui_s(56)) { return last_drive_tool; }
	if (page == UiPage::Dice && desk_dice_tap(x, y)) { screens_invalidate(); }
	if (page == UiPage::FlappyBird && desk_flappy_tap(x, y, now)) { screens_invalidate(); }
	if (screens_drive(page) && drive_tap(page, x, y)) { screens_invalidate(); }
	if (page == UiPage::Phone) { phone_view_tap(x, y); return page; }
	if (page == UiPage::Tris) { desk_tris_tap(x, y, now); screens_invalidate(); }
	if (page == UiPage::Pong) {
		if (y >= ui_s(82) && y < ui_s(210)) { desk_pong().move(float(y - ui_s(82)) * 160 / ui_s(128)); }
		if (y >= ui_s(236) && y < ui_s(258)) { desk_pong().reset(now); }
		screens_invalidate();
	}
	if (page == UiPage::Timer) {
		// The digits card and START/PAUSE both toggle, so the large time is a target too.
		if (y >= ui_s(56) && y < ui_s(208)) { view_timer().toggle(now); screens_invalidate(); }
		if (y >= ui_s(208) && y < ui_s(262)) {
			int16_t split = ui_s(timer_row().split);
			if (x < split) { view_timer().reset(now); }
			else { view_timer().cycle(now); }
			screens_invalidate();
		}
	}
	return page;
}

bool screens_hold(UiPage page, uint32_t now) {
	if (page == UiPage::Navigation) { view_route_temporary(-1); return true; }
	if (page == UiPage::Timer) { view_timer().reset(now); return true; }
	if (page == UiPage::Drive || drive_has_zero(page)) { drive_meter().zero(); return true; }
	return false;
}

/** @brief Dispatch vertical gestures without persisting session choices. @param page Page. @param scroll +1 up, -1 down. @return Target. */
UiPage screens_scroll(UiPage page, int8_t scroll) {
	if (!scroll) { return page; }
	if (screens_drive(page)) {
		if (page == UiPage::Drive) { return scroll < 0 ? UiPage::DriveHub : page; }
		if (page == UiPage::DriveHub) { return scroll > 0 ? UiPage::Drive : page; }
		return scroll > 0 ? UiPage::DriveHub : page;
	}
	if (page == UiPage::Phone && phone_view_scroll(scroll)) { return page; }
	if (scroll > 0) {
		if (page == UiPage::Pong || page == UiPage::Tris || page == UiPage::Dice ||
			page == UiPage::FlappyBird) { return UiPage::Games; }
		if (page == UiPage::Desk) { return UiPage::Clock; }
		if (uint8_t(page) > uint8_t(UiPage::Desk)) { return UiPage::Desk; }
		return page;
	}
	if (page == UiPage::Clock) { return UiPage::Desk; }
	if (page == UiPage::Desk) { return UiPage::Games; }
	if (page == UiPage::Games || page == UiPage::Dice) { return page; }
	if (page == UiPage::Navigation) {
		view_route_temporary(!view_route_large(settings_get().route_large));
		screens_invalidate();
		return page;
	}
	if (page == UiPage::Weather) { weather_request(); screens_invalidate(); return page; }
	return UiPage::Eyes;
}
