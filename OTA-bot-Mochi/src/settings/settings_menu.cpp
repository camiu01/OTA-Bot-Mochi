/**
 * @file settings_menu.cpp
 * @brief Large row targets, read-only diagnostics and explicit saving without browsing writes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "settings_menu.h"
#include "../diagnostics/diagnostics_view.h"
#include <cstdio>
#include "settings.h"
#include "../ui/ui_theme.h"
#include "../ui/i18n.h"
#include "../sound/sound.h"
#include "../eyes/imu_orientation.h"
#include "../eyes/eyes_api.h"
#include "../../pin_config.h"

namespace {
uint8_t tab = 0;
bool save_failed = false;
bool sound_options = false;
bool diagnostics_open = false;
uint8_t diagnostics_first = 0;
uint8_t first_row = 0;
enum MenuTab : uint8_t { TAB_LOOK, TAB_LIGHT, TAB_FACES, TAB_MOTION, TAB_AUDIO, TAB_SYSTEM, TAB_COUNT };

/** SD sounds only appears on the board with an SD slot and codec. */
#if MOCHI_HAS_SD_AUDIO
constexpr bool HAS_SD_ROW = true;
#else
constexpr bool HAS_SD_ROW = false;
#endif
/** Row indices in the main SOUND list, shifted on boards without SD sounds. */
constexpr uint8_t SOUND_OPTIONS_ROW = HAS_SD_ROW ? 4 : 3;
constexpr uint8_t SOUND_TEST_ROW = HAS_SD_ROW ? 5 : 4;
/** Last SYSTEM row: the BLE OBD dongle scan toggle. */
constexpr uint8_t OBD_ROW = HAS_SD_ROW ? 8 : 7;

/**
 * @brief Bottom edge of the SAVE/Back band.
 *
 * The band normally ends at the 240x280 design field, but taller panels scale
 * that field short of the physical edge and leave a dead strip. Reaching to the
 * real panel edge (keeping a 4-pixel inert margin) makes the bottom buttons tall
 * enough to hit without turning the whole strip into a target.
 * @return Panel pixel row just past the band.
 */
int16_t bottom_band_bottom() {
	const int16_t design = ui_s(280);
	const int16_t panel = int16_t(LCD_HEIGHT - 4);
	return panel > design ? panel : design;
}

/**
 * @brief Draw the SAVE (primary, or muted once saved) and right-hand action buttons.
 * @param gfx Canvas. @param right Label of the right button.
 */
void save_band(Arduino_GFX *gfx, const char *right) {
	int16_t h = int16_t(bottom_band_bottom() - ui_s(224));
	gfx->fillRect(0, ui_s(222), LCD_WIDTH, int16_t(LCD_HEIGHT - ui_s(222)), UI_BG);	// Hide carousel dots in the button gap.
	bool dirty = settings_dirty();
	ui_button(gfx, ui_s(2), ui_s(224), ui_s(116), h,
		tr(save_failed ? Text::Failed : (dirty ? Text::Save : Text::Saved)), dirty || save_failed);
	ui_button(gfx, ui_s(122), ui_s(224), ui_s(116), h, right);
}

/** @brief Number of rows in the current panel. */
uint8_t row_count() {
	static const uint8_t counts[] = {3, 7, 6, 5, 5, OBD_ROW + 1};
	if (tab != TAB_AUDIO) { return counts[tab]; }
	return sound_options ? 5 : (HAS_SD_ROW ? 6 : 5);
}

}

#include "settings_grid.h"

namespace {

/** @brief Draw one editable row. @param gfx Canvas. @param index Row. @param label Field. @param value Choice. */
void row(Arduino_GFX *gfx, uint8_t index, const char *label, const String &value) {
	if (index < first_row || index >= first_row + 3) { return; }
	int16_t design_y = int16_t(90 + (index - first_row) * 44);
	ui_card(gfx, design_y + 2, 40);
	int16_t y = ui_s(design_y);
	bool on = value == tr(Text::On), off = value == tr(Text::Off);
	ui_label(gfx, label, ui_s(24), y + ui_s(7), on || off ? ui_s(156) : ui_s(192), 1, UI_MUTED);
	if (!on && !off) { ui_label(gfx, value, ui_s(24), y + ui_s(23), ui_s(192), 1, UI_ACCENT); return; }
	// Toggle rows show a switch instead of a yes/no word.
	int16_t sx = ui_s(184), sy = y + ui_s(14), sw = ui_s(32), sh = ui_s(16);
	gfx->fillRoundRect(sx, sy, sw, sh, sh / 2, on ? UI_ACCENT : UI_LINE);
	gfx->fillCircle(on ? sx + sw - sh / 2 : sx + sh / 2, sy + sh / 2, sh / 2 - ui_s(3), on ? UI_BG : UI_MUTED);
	ui_label(gfx, value, ui_s(24), y + ui_s(23), ui_s(120), 1, on ? UI_TEXT : UI_MUTED);
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

/** @brief Cycle a schedule endpoint without creating a zero-length interval. @param current Hour. @param other Other endpoint. @return Next hour. */
uint8_t next_hour(uint8_t current, uint8_t other) {
	uint8_t next = (current + 1) % 24;
	return next == other ? (next + 1) % 24 : next;
}
/** @brief Edit display comfort and schedule fields in RAM. @param p Candidate. @param index Row. */
void change_light(CompanionSettings &p, uint8_t index) {
	if (index == 0) { p.brightness = next_brightness(p.brightness, false); }
	if (index == 1) { p.night = !p.night; }
	if (index == 2) { p.night_brightness = next_brightness(p.night_brightness, true); }
	if (index == 3) { p.dim = !p.dim; }
	if (index == 4) { p.night_auto = !p.night_auto; }
	if (index == 5) { p.night_start = next_hour(p.night_start, p.night_end); }
	if (index == 6) { p.night_end = next_hour(p.night_end, p.night_start); }
}
/** @brief Format a schedule endpoint with room for the full parameter range. @param hour 24-hour value. @return HH:00. */
String schedule_hour(uint8_t hour) {
	char text[7]; snprintf(text, sizeof(text), "%02u:00", unsigned(hour));
	return String(text);
}

/** @brief Apply one row tap. @param index Row index. */
void change_row(uint8_t index) {
	CompanionSettings p = settings_get();
	if (tab == 0) {
		if (index == 0) { p.style = (p.style + 1) % EYE_STYLE_COUNT; }
		if (index == 1) { p.color = (p.color + 1) % 6; }
		if (index == 2) {
			uint8_t focus = uint8_t(p.mood / GRID_COLS);
			grid_open = true;
			filter_open = false;
			grid_first_row = focus > GRID_MAX_FIRST ? GRID_MAX_FIRST : focus;
			return;
		}
	} else if (tab == 1) {
		change_light(p, index);
	} else if (tab == 2) {
		if (index == 0) { p.animation = (p.animation + 1) % 3; }
		if (index == 1) { p.random_faces = !p.random_faces; }
		if (index == 2) { p.face_flow = (p.face_flow + 1) % FACE_FLOW_COUNT; }
		if (index == 3) { grid_open = filter_open = true; keep_one = false; grid_first_row = 0; return; }
		if (index == 4 || index == 5) {
			grid_open = favorites_open = true; favorites_edit = index == 5;
			filter_open = false; grid_first_row = 0; return;
		}
	} else if (tab == 3) {
		if (index == 0) { p.imu_faces = !p.imu_faces; }
		if (index == 1) { p.shake = !p.shake; }
		if (index == 2) { if (!eyes_imu_calibrate()) { imu_orientation().status = ImuCalibration::Failed; } return; }
		if (index == 3) { imu_orientation().enabled = !imu_orientation().enabled; return; }
		if (index == 4) { p.flipped = !p.flipped; }
	} else if (tab == TAB_SYSTEM) {
		if (index == 0) { p.language = (p.language + 1) % 2; }
		if (index == 1) { p.dashboard = !p.dashboard; }
		if (index == 2) { p.route_large = !p.route_large; }
		if (index == 3) { p.phone_ios = !p.phone_ios; }
		if (index == OBD_ROW) { p.obd_scan = !p.obd_scan; }
	} else if (sound_options) {
		if (index == 0) { p.sound_faces = !p.sound_faces; }
		if (index == 1) { p.sound_timer = !p.sound_timer; }
		if (index == 2) { p.sound_alerts = !p.sound_alerts; }
		if (index == 3) { p.sound_ambient = !p.sound_ambient; }
		if (index == 4) { p.sound_charging = !p.sound_charging; }
	} else {
		if (index == 0) { p.sound = !p.sound; }
		if (index == 1) { p.sound_level = p.sound_level < 5 ? 5 : (p.sound_level >= 30 ? 5 : (p.sound_level >= 25 ? 30 : p.sound_level + 5)); }
		if (index == 2) { p.sound_night_mute = !p.sound_night_mute; }
#if MOCHI_HAS_SD_AUDIO
		if (index == 3) { p.sound_sd = !p.sound_sd; }
#endif
		if (index == SOUND_OPTIONS_ROW) { sound_options = true; first_row = 0; return; }
		if (index == SOUND_TEST_ROW) { sound_request(SoundCue::Test, millis()); return; }
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
	} else if (tab == 1) {
		row(gfx, 0, tr(Text::DayLevel), String(p.brightness) + "%");
		row(gfx, 1, tr(Text::NightManual), on_off(p.night));
		row(gfx, 2, tr(Text::NightLevel), String(p.night_brightness) + "%");
		row(gfx, 3, tr(Text::IdleDim), on_off(p.dim));
		row(gfx, 4, tr(Text::NightAuto), on_off(p.night_auto));
		row(gfx, 5, tr(Text::NightStart), schedule_hour(p.night_start));
		row(gfx, 6, tr(Text::NightEnd), schedule_hour(p.night_end));
	} else {
		row(gfx, 0, tr(Text::Animation), tr_choice(3, p.animation));
		row(gfx, 1, tr(Text::RandomMood), on_off(p.random_faces));
		row(gfx, 2, tr(Text::FaceFlow), tr_choice(4, p.face_flow));
		row(gfx, 3, tr(Text::FaceLibrary), tr(Text::Open));
		row(gfx, 4, tr(Text::Favorites), tr(Text::Open));
		row(gfx, 5, tr(Text::EditFavorites), String(settings_favorites().count()));
	}
}
/** @brief Draw motion/language or audio options. @param gfx Canvas. @param p Profile. */
void extra_rows(Arduino_GFX *gfx, const CompanionSettings &p) {
	if (tab == 3) {
		row(gfx, 0, tr(Text::ImuMood), on_off(p.imu_faces));
		row(gfx, 1, tr(Text::Shake), on_off(p.shake));
		ImuCalibration status = imu_orientation().status;
		row(gfx, 2, tr(Text::CalibrateImu), tr(status == ImuCalibration::Sampling ? Text::Calibrating :
			(status == ImuCalibration::Ready ? (imu_orientation().save_failed ? Text::Failed :
				(imu_orientation().persisted ? Text::Saved : Text::Calibrated)) :
				(status == ImuCalibration::Failed ? Text::Failed : Text::Calibrate))));
		row(gfx, 3, tr(Text::AutoRotate), on_off(imu_orientation().enabled));
		row(gfx, 4, tr(Text::ScreenFlip), tr(p.flipped ? Text::Flipped : Text::Normal));
	} else if (tab == TAB_SYSTEM) {
		row(gfx, 0, tr(Text::Language), p.language ? "ITALIANO" : "ENGLISH");
		row(gfx, 1, tr(Text::Dashboard), on_off(p.dashboard));
		row(gfx, 2, tr(Text::RouteDefault), tr(p.route_large ? Text::Large : Text::Details));
			row(gfx, 3, tr(Text::PhoneMode), tr(p.phone_ios ? Text::IphoneMode : Text::AndroidMode));
			row(gfx, 4, tr(Text::FirmwareUpdate), tr(Text::Open));
			row(gfx, 5, tr(Text::WebSettings), tr(Text::Open));
		row(gfx, 6, tr(Text::Diagnostics), tr(Text::Open));
#if MOCHI_HAS_SD_AUDIO
		row(gfx, 7, tr(Text::SdFirmware), tr(Text::Open));
#endif
		row(gfx, OBD_ROW, tr(Text::ObdScan), on_off(p.obd_scan));
	} else if (sound_options) {
		row(gfx, 0, tr(Text::Faces), on_off(p.sound_faces));
		row(gfx, 1, tr(Text::TimerLabel), on_off(p.sound_timer));
		row(gfx, 2, tr(Text::CallsAlerts), on_off(p.sound_alerts));
		row(gfx, 3, tr(Text::Ambient), on_off(p.sound_ambient));
		row(gfx, 4, tr(Text::Charging), on_off(p.sound_charging));
	} else {
		row(gfx, 0, tr(Text::Sound), on_off(p.sound));
		row(gfx, 1, tr(Text::Level), String(p.sound_level) + "%");
		row(gfx, 2, tr(Text::NightMute), on_off(p.sound_night_mute));
#if MOCHI_HAS_SD_AUDIO
		row(gfx, 3, tr(Text::SoundSd), on_off(p.sound_sd));
#endif
		row(gfx, SOUND_OPTIONS_ROW, tr(Text::SoundOptions), tr(Text::Open));
		SoundStatus status = sound_status();
		Text value = status == SoundStatus::Off ? Text::Off :
			(status == SoundStatus::Night ? Text::NightMute : (status == SoundStatus::Error ? Text::Failed : Text::Test));
		row(gfx, SOUND_TEST_ROW, tr(Text::TestSound), tr(value));
	}
}

}

/** @brief Draw the diagnostic overlay or compact settings tabs. @param gfx Canvas. */
void settings_menu_draw(Arduino_GFX *gfx) {
	if (diagnostics_open) { diagnostics_view::draw(gfx, diagnostics_first); return; }
	if (grid_open) { grid_draw(gfx); return; }
	const CompanionSettings &p = settings_get();
	ui_box(gfx, ui_s(2), ui_s(52), ui_s(68), ui_s(36), UI_CARD, true);
	ui_box(gfx, ui_s(170), ui_s(52), ui_s(68), ui_s(36), UI_CARD, true);
	ui_label(gfx, "<", ui_s(30), ui_s(62), ui_s(24), 2, UI_ACCENT);
	ui_label(gfx, ">", ui_s(198), ui_s(62), ui_s(24), 2, UI_ACCENT);
	ui_label(gfx, tr_settings_tab(tab), ui_s(78), ui_s(58), ui_s(84), 1, UI_ACCENT);
	ui_label(gfx, String(tab + 1) + "/" + String(uint8_t(TAB_COUNT)), ui_s(108), ui_s(74), ui_s(24), 1, UI_MUTED);
	if (tab < 3) { basic_rows(gfx, p); } else { extra_rows(gfx, p); }
	if (row_count() > 3) {
		uint8_t max_scroll = row_count() - 3;
		gfx->fillRoundRect(ui_s(232), ui_s(92), ui_s(3), ui_s(128), ui_s(1), UI_LINE);
		gfx->fillRoundRect(ui_s(232), ui_s(int16_t(92 + first_row * (128 - 60) / max_scroll)), ui_s(3), ui_s(60), ui_s(1), UI_ACCENT);
	}
	save_band(gfx, tr(tab == 4 && sound_options ? Text::Back : Text::EyesButton));
}

/** @brief Interpret tap coordinates without conflating swipes/holds. @param x X. @param y Y. @return Action. */
MenuAction settings_menu_tap(int16_t x, int16_t y) {
	if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) { return MenuAction::None; }
	if (diagnostics_open) {
		if (y < ui_s(224) || y >= bottom_band_bottom()) { return MenuAction::None; }
		diagnostics_open = false; return MenuAction::Changed;
	}
	if (grid_open) {
		if (filter_open && x >= ui_s(170) && y >= ui_s(52) && y < ui_s(88)) { return MenuAction::FacesWeb; }
		if (y >= ui_s(224) && y < bottom_band_bottom()) {
			if (x < ui_s(120)) { save_failed = !settings_save(); } else { grid_close(); }
			return MenuAction::Changed;
		}
		return grid_cell(x, y) ? MenuAction::Changed : MenuAction::None;
	}
	if (y >= ui_s(52) && y < ui_s(90)) {
		if (x >= ui_s(72) && x < ui_s(168)) { return MenuAction::None; }
		tab = (tab + (x < ui_s(72) ? TAB_COUNT - 1 : 1)) % TAB_COUNT;
		sound_options = false; first_row = 0;
		return MenuAction::Changed;
	}
	if (y >= ui_s(224) && y < bottom_band_bottom()) {
		if (x >= ui_s(120)) {
			if (tab == 4 && sound_options) { sound_options = false; first_row = 0; return MenuAction::Changed; }
			grid_close();
			return MenuAction::Exit;
		}
		save_failed = !settings_save();
		return MenuAction::Changed;
	}
	if (y >= ui_s(90) && y < ui_s(222)) {
		uint8_t visible = 0;
		while (visible < 2 && y >= ui_s(int16_t(90 + (visible + 1) * 44))) { ++visible; }
		uint8_t index = uint8_t(first_row + visible);
		uint8_t count = row_count();
		if (tab == TAB_SYSTEM && HAS_SD_ROW && index == 7) { return MenuAction::SdUpdate; }
			if (tab == TAB_SYSTEM && index == 4) { return MenuAction::Update; }
			if (tab == TAB_SYSTEM && index == 5) { return MenuAction::FacesWeb; }
		if (tab == TAB_SYSTEM && index == 6) {
			diagnostics_open = true; diagnostics_first = 0; return MenuAction::Changed;
		}
		if (index < count) { change_row(index); return MenuAction::Changed; }
	}
	return MenuAction::None;
}

bool settings_menu_scroll(int8_t direction) {
	if (diagnostics_open) {
		uint8_t next = diagnostics_first;
		if (direction > 0 && next < diagnostics_view::ROWS - 3) { ++next; }
		if (direction < 0 && next > 0) { --next; }
		if (next == diagnostics_first) { return false; }
		diagnostics_first = next; return true;
	}
	if (grid_open) {
		uint8_t next = grid_first_row;
		if (next > grid_limit()) { next = grid_limit(); }
		if (direction > 0 && next < grid_limit()) { ++next; }
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
