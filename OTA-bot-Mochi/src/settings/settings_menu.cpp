/**
 * @file settings_menu.cpp
 * @brief Large row targets with explicit saving, no flash writes during browsing.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "settings_menu.h"
#include "settings.h"
#include "../ui/ui_theme.h"
#include "../ui/i18n.h"

namespace {
uint8_t tab = 0;
bool save_failed = false;
bool sound_options = false;

/** @brief Draw one editable row. @param gfx Canvas. @param index Row. @param label Field. @param value Choice. */
void row(Arduino_GFX *gfx, uint8_t index, const char *label, const String &value) {
	int16_t y = 90 + index * 34;
	ui_card(gfx, y, 28);
	ui_label(gfx, label, 24, y + 10, 84, 1, UI_MUTED);
	ui_label(gfx, value, 116, y + 10, 100, 1, UI_ACCENT);
}

/** @brief Cycle to the next bounded brightness preset. @param value Current. @param night Night range. @return Next. */
uint8_t next_brightness(uint8_t value, bool night) {
	static const uint8_t day[] = {5, 10, 20, 30, 50, 70, 90};
	static const uint8_t dark[] = {1, 3, 5, 8, 12, 20};
	const uint8_t *choices = night ? dark : day;
	uint8_t count = night ? 6 : 7;
	for (uint8_t i = 0; i < count; ++i) { if (choices[i] > value) { return choices[i]; } }
	return choices[0];
}

/** @brief Apply one row tap. @param index Row index. */
void change_row(uint8_t index) {
	CompanionSettings p = settings_get();
	if (tab == 0) {
		if (index == 0) { p.style = (p.style + 1) % 4; }
		if (index == 1) { p.color = (p.color + 1) % 6; }
		if (index == 2) { p.mood = (p.mood + 1) % 5; }
		if (index == 3) { p.route_large = !p.route_large; }
	} else if (tab == 1) {
		if (index == 0) { p.brightness = next_brightness(p.brightness, false); }
		if (index == 1) { p.night = !p.night; }
		if (index == 2) { p.night_brightness = next_brightness(p.night_brightness, true); }
		if (index == 3) { p.dim = !p.dim; }
	} else if (tab == 2) {
		if (index == 0) { p.dashboard = !p.dashboard; }
		if (index == 1) { p.shake = !p.shake; }
		if (index == 2) { p.random_faces = !p.random_faces; }
		if (index == 3) { p.imu_faces = !p.imu_faces; }
	} else if (tab == 3) {
		if (index == 0) { p.animation = (p.animation + 1) % 3; }
		if (index == 1) { p.language = (p.language + 1) % 2; }
	} else if (sound_options) {
		if (index == 0) { p.sound_faces = !p.sound_faces; }
		if (index == 1) { p.sound_timer = !p.sound_timer; }
		if (index == 2) { p.sound_alerts = !p.sound_alerts; }
		if (index == 3) { p.sound_ambient = !p.sound_ambient; }
	} else {
		if (index == 0) { p.sound = !p.sound; }
		if (index == 1) { p.sound_level = p.sound_level < 5 ? 5 : (p.sound_level >= 30 ? 5 : (p.sound_level >= 25 ? 30 : p.sound_level + 5)); }
		if (index == 2) { p.sound_night_mute = !p.sound_night_mute; }
		if (index == 3) { sound_options = true; return; }
	}
	settings_update(p);
	save_failed = false;
}
/** @brief Localized toggle. @param value Toggle. @return Label. */
const char *on_off(bool value) { return tr(value ? Text::On : Text::Off); }
/** @brief Draw appearance and comfort rows. @param gfx Canvas. @param p Profile. */
void basic_rows(Arduino_GFX *gfx, const CompanionSettings &p) {
	if (tab == 0) {
		row(gfx, 0, tr(Text::EyesLabel), tr_choice(0, p.style));
		row(gfx, 1, tr(Text::ColorLabel), tr_choice(1, p.color));
		row(gfx, 2, tr(Text::MoodLabel), tr_choice(2, p.mood));
		row(gfx, 3, tr(Text::RouteDefault), tr(p.route_large ? Text::Large : Text::Details));
	} else if (tab == 1) {
		row(gfx, 0, tr(Text::DayLevel), String(p.brightness) + "%");
		row(gfx, 1, tr(Text::Night), on_off(p.night));
		row(gfx, 2, tr(Text::NightLevel), String(p.night_brightness) + "%");
		row(gfx, 3, tr(Text::IdleDim), on_off(p.dim));
	} else {
		row(gfx, 0, tr(Text::Dashboard), on_off(p.dashboard));
		row(gfx, 1, tr(Text::Shake), on_off(p.shake));
		row(gfx, 2, tr(Text::RandomMood), on_off(p.random_faces));
		row(gfx, 3, tr(Text::ImuMood), on_off(p.imu_faces));
	}
}
/** @brief Draw motion/language or audio options. @param gfx Canvas. @param p Profile. */
void extra_rows(Arduino_GFX *gfx, const CompanionSettings &p) {
	if (tab == 3) {
		row(gfx, 0, tr(Text::Animation), tr_choice(3, p.animation));
		row(gfx, 1, tr(Text::Language), p.language ? "Italiano" : "English");
		ui_label(gfx, tr(Text::MotionHint), 24, 177, 192, 1, UI_MUTED);
	} else if (sound_options) {
		row(gfx, 0, tr(Text::Faces), on_off(p.sound_faces));
		row(gfx, 1, tr(Text::TimerLabel), on_off(p.sound_timer));
		row(gfx, 2, tr(Text::CallsAlerts), on_off(p.sound_alerts));
		row(gfx, 3, tr(Text::Ambient), on_off(p.sound_ambient));
	} else {
		row(gfx, 0, tr(Text::Sound), on_off(p.sound));
		row(gfx, 1, tr(Text::Level), String(p.sound_level) + "%");
		row(gfx, 2, tr(Text::NightMute), on_off(p.sound_night_mute));
		row(gfx, 3, tr(Text::SoundOptions), tr(Text::Open));
	}
}
}

/** @brief Draw compact appearance, lighting, and car-mode tabs. @param gfx Canvas. */
void settings_menu_draw(Arduino_GFX *gfx) {
	const CompanionSettings &p = settings_get();
	static const Text tabs[] = {Text::LookTab, Text::LightTab, Text::DriveTab, Text::MoveTab, Text::AudioTab};
	for (uint8_t i = 0; i < 5; ++i) {
		gfx->fillRoundRect(12 + i * 44, 57, 40, 25, 8, i == tab ? UI_ACCENT : UI_CARD);
		ui_label(gfx, tr(tabs[i]), 16 + i * 44, 66, 32, 1, i == tab ? UI_BG : UI_TEXT);
	}
	if (tab < 3) { basic_rows(gfx, p); } else { extra_rows(gfx, p); }
	gfx->fillRoundRect(12, 230, 102, 24, 8, UI_ACCENT);
	gfx->fillRoundRect(126, 230, 102, 24, 8, UI_CARD);
	ui_label(gfx, tr(save_failed ? Text::Failed : (settings_dirty() ? Text::Save : Text::Saved)),
		36, 238, 72, 1, UI_BG);
	ui_label(gfx, tr(tab == 4 && sound_options ? Text::Back : Text::EyesButton), 147, 238, 72, 1, UI_TEXT);
}

/** @brief Interpret tap coordinates without conflating swipes/holds. @param x X. @param y Y. @return Action. */
MenuAction settings_menu_tap(int16_t x, int16_t y) {
	if (y >= 57 && y < 82 && x >= 12 && x < 228) {
		uint8_t next = (x - 12) / 44;
		if (next < 5 && (x - 12) % 44 < 40) { tab = next; sound_options = false; return MenuAction::Changed; }
	}
	if (y >= 230 && y < 254) {
		if (x >= 126 && x < 228) {
			if (tab == 4 && sound_options) { sound_options = false; return MenuAction::Changed; }
			return MenuAction::Exit;
		}
		if (x >= 12 && x < 114) {
			save_failed = !settings_save();
			return MenuAction::Changed;
		}
	}
	if (x >= 12 && x < 228 && y >= 90 && y < 220) {
		uint8_t index = (y - 90) / 34;
		uint8_t count = tab == 3 ? 2 : 4;
		if (index < count && (y - 90) % 34 < 28) { change_row(index); return MenuAction::Changed; }
	}
	return MenuAction::None;
}
