/**
 * @file settings_menu.cpp
 * @brief Large row targets with explicit saving, no flash writes during browsing.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "settings_menu.h"
#include "settings.h"
#include "../ui/ui_theme.h"
#include "../ui/i18n.h"
#include "../sound/sound.h"

namespace {
uint8_t tab = 0;
bool save_failed = false;
bool sound_options = false;
uint8_t first_row = 0;
bool grid_open = false;
uint8_t grid_first_row = 0;

/** Expression grid geometry: three columns of large touch targets. */
constexpr uint8_t GRID_COLS = 3;
constexpr uint8_t GRID_ROWS = 6;
constexpr uint8_t GRID_ROW_PX = 21;
constexpr int16_t GRID_X0 = 8;
constexpr int16_t GRID_Y0 = 94;
constexpr int16_t GRID_W = 72;
constexpr int16_t GRID_H = 19;
constexpr uint8_t GRID_ROWS_TOTAL = (EXPRESSION_COUNT + GRID_COLS - 1) / GRID_COLS;
constexpr uint8_t GRID_MAX_FIRST = GRID_ROWS_TOTAL > GRID_ROWS ? GRID_ROWS_TOTAL - GRID_ROWS : 0;

/** @brief Number of rows in the current panel. */
uint8_t row_count() { return tab == 3 ? 2 : (tab == 4 && !sound_options ? 5 : 4); }

/** @brief Close the grid and restore the tab rows. */
void grid_close() { grid_open = false; grid_first_row = 0; first_row = 0; }

/** @brief Draw one editable row. @param gfx Canvas. @param index Row. @param label Field. @param value Choice. */
void row(Arduino_GFX *gfx, uint8_t index, const char *label, const String &value) {
	if (index < first_row || index >= first_row + 3) { return; }
	int16_t y = 90 + (index - first_row) * 44;
	ui_card(gfx, y, 40);
	ui_label(gfx, label, 24, y + 16, 84, 1, UI_MUTED);
	ui_label(gfx, value, 116, y + 16, 100, 1, UI_ACCENT);
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
		if (index == 2) {
			uint8_t focus = uint8_t(p.mood / GRID_COLS);
			grid_open = true;
			grid_first_row = focus > GRID_MAX_FIRST ? GRID_MAX_FIRST : focus;
			return;
		}
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
		if (index == 3) { sound_options = true; first_row = 0; return; }
		if (index == 4) { sound_request(SoundCue::Test, millis()); return; }
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
		ui_label(gfx, tr(Text::MotionHint), 24, 190, 192, 1, UI_MUTED);
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
		SoundStatus status = sound_status();
		Text value = status == SoundStatus::Off ? Text::Off :
			(status == SoundStatus::Night ? Text::NightMute : (status == SoundStatus::Error ? Text::Failed : Text::Test));
		row(gfx, 4, tr(Text::TestSound), tr(value));
	}
}

/** @brief Draw the expression grid overlay, replacing the tab rows. @param gfx Canvas. */
void grid_draw(Arduino_GFX *gfx) {
	const CompanionSettings &p = settings_get();
	uint8_t first = uint8_t(grid_first_row * GRID_COLS + 1);
	uint8_t last = uint8_t(first + GRID_ROWS * GRID_COLS - 1);
	if (last > EXPRESSION_COUNT) { last = EXPRESSION_COUNT; }
	gfx->fillRoundRect(2, 52, 236, 36, 8, UI_CARD);
	ui_label(gfx, tr(Text::MoodLabel), 14, 66, 110, 1, UI_ACCENT);
	ui_label(gfx, String(first) + "-" + String(last) + "/" +
		String(EXPRESSION_COUNT), 150, 66, 76, 1, UI_MUTED);
	for (uint8_t cell = 0; cell < GRID_ROWS * GRID_COLS; ++cell) {
		uint16_t index = uint16_t(grid_first_row) * GRID_COLS + cell;
		if (index >= EXPRESSION_COUNT) { break; }
		int16_t x = int16_t(GRID_X0 + (cell % GRID_COLS) * (GRID_W + 4));
		int16_t y = int16_t(GRID_Y0 + (cell / GRID_COLS) * GRID_ROW_PX);
		bool selected = index == p.mood;
		gfx->fillRoundRect(x, y, GRID_W, GRID_H, 6, selected ? UI_ACCENT : UI_CARD);
		ui_label(gfx, tr_expression(uint8_t(index)), x + 3, y + 6, GRID_W - 6, 1,
			selected ? UI_BG : UI_TEXT);
	}
	if (GRID_MAX_FIRST) {
		int16_t track = int16_t(GRID_ROWS * GRID_ROW_PX) - 2;
		gfx->fillRect(234, GRID_Y0, 3, track, UI_CARD);
		gfx->fillRect(234, int16_t(GRID_Y0 + grid_first_row * (track - 40) / GRID_MAX_FIRST),
			3, 40, UI_ACCENT);
	}
	gfx->fillRoundRect(2, 224, 116, 54, 8, UI_ACCENT);
	gfx->fillRoundRect(122, 224, 116, 54, 8, UI_CARD);
	ui_label(gfx, tr(save_failed ? Text::Failed : (settings_dirty() ? Text::Save : Text::Saved)),
		36, 247, 72, 1, UI_BG);
	ui_label(gfx, tr(Text::Back), 147, 247, 72, 1, UI_TEXT);
}

/**
 * @brief Apply a tapped grid cell, ignoring the gutters between touch targets.
 * @param x X. @param y Y. @return True when a real cell was hit.
 */
bool grid_cell(int16_t x, int16_t y) {
	if (x < GRID_X0 || y < GRID_Y0) { return false; }
	int16_t local_x = int16_t(x - GRID_X0), local_y = int16_t(y - GRID_Y0);
	uint8_t col = uint8_t(local_x / (GRID_W + 4));
	uint8_t row = uint8_t(local_y / GRID_ROW_PX);
	if (col >= GRID_COLS || row >= GRID_ROWS) { return false; }
	if (local_x % (GRID_W + 4) >= GRID_W || local_y % GRID_ROW_PX >= GRID_H) { return false; }
	uint16_t index = uint16_t(grid_first_row + row) * GRID_COLS + col;
	if (index >= EXPRESSION_COUNT) { return false; }
	CompanionSettings p = settings_get();
	p.mood = uint8_t(index);
	save_failed = false;
	settings_update(p);
	return true;
}
}

/** @brief Draw compact appearance, lighting, and car-mode tabs. @param gfx Canvas. */
void settings_menu_draw(Arduino_GFX *gfx) {
	if (grid_open) { grid_draw(gfx); return; }
	const CompanionSettings &p = settings_get();
	static const Text tabs[] = {Text::LookTab, Text::LightTab, Text::DriveTab, Text::MoveTab, Text::AudioTab};
	gfx->fillRoundRect(2, 52, 68, 36, 8, UI_CARD);
	gfx->fillRoundRect(170, 52, 68, 36, 8, UI_CARD);
	ui_label(gfx, "<", 30, 62, 24, 2, UI_TEXT);
	ui_label(gfx, ">", 198, 62, 24, 2, UI_TEXT);
	ui_label(gfx, tr(tabs[tab]), 84, 58, 72, 1, UI_ACCENT);
	ui_label(gfx, String(tab + 1) + "/5", 108, 74, 24, 1, UI_MUTED);
	if (tab < 3) { basic_rows(gfx, p); } else { extra_rows(gfx, p); }
	if (tab != 3) {
		uint8_t max_scroll = row_count() - 3;
		gfx->fillRect(232, 92, 3, 128, UI_CARD);
		gfx->fillRect(232, 92 + first_row * (128 - 60) / max_scroll, 3, 60, UI_ACCENT);
	}
	gfx->fillRoundRect(2, 224, 116, 54, 8, UI_ACCENT);
	gfx->fillRoundRect(122, 224, 116, 54, 8, UI_CARD);
	ui_label(gfx, tr(save_failed ? Text::Failed : (settings_dirty() ? Text::Save : Text::Saved)),
		36, 247, 72, 1, UI_BG);
	ui_label(gfx, tr(tab == 4 && sound_options ? Text::Back : Text::EyesButton), 147, 247, 72, 1, UI_TEXT);
}

/** @brief Interpret tap coordinates without conflating swipes/holds. @param x X. @param y Y. @return Action. */
MenuAction settings_menu_tap(int16_t x, int16_t y) {
	if (x < 0 || x >= 240 || y < 0 || y >= 280) { return MenuAction::None; }
	if (grid_open) {
		if (y >= 224 && x < 120) { save_failed = !settings_save(); return MenuAction::Changed; }
		if (y >= 224) { grid_close(); return MenuAction::Changed; }
		return grid_cell(x, y) ? MenuAction::Changed : MenuAction::None;
	}
	if (y >= 52 && y < 90) {
		if (x >= 72 && x < 168) { return MenuAction::None; }
		tab = (tab + (x < 72 ? 4 : 1)) % 5;
		sound_options = false; first_row = 0;
		return MenuAction::Changed;
	}
	if (y >= 222) {
		if (x >= 120) {
			if (tab == 4 && sound_options) { sound_options = false; first_row = 0; return MenuAction::Changed; }
			grid_close();
			return MenuAction::Exit;
		}
		if (x < 120) {
			save_failed = !settings_save();
			return MenuAction::Changed;
		}
	}
	if (y >= 90 && y < 222) {
		uint8_t index = first_row + (y - 90) / 44;
		uint8_t count = row_count();
		if (index < count) { change_row(index); return MenuAction::Changed; }
	}
	return MenuAction::None;
}

bool settings_menu_scroll(int8_t direction) {
	if (grid_open) {
		uint8_t next = grid_first_row;
		if (direction > 0 && next < GRID_MAX_FIRST) { ++next; }
		if (direction < 0 && next > 0) { --next; }
		if (next == grid_first_row) { return false; }
		grid_first_row = next;
		return true;
	}
	uint8_t limit = row_count() > 3 ? row_count() - 3 : 0;
	uint8_t next = first_row;
	if (direction > 0 && next < limit) { ++next; }
	if (direction < 0 && next > 0) { --next; }
	if (next == first_row) { return false; }
	first_row = next;
	return true;
}

/** @brief Report whether the expression grid overlay currently replaces the tabs. @return Grid visible. */
bool settings_menu_grid_open() { return grid_open; }
