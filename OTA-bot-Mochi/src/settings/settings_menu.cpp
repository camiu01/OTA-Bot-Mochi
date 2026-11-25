/**
 * @file settings_menu.cpp
 * @brief Large row targets, read-only diagnostics and explicit saving without browsing writes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "settings_menu.h"
#include "../diagnostics/diagnostics_view.h"
#include <cstdio>
#include <cstring>
#include "settings.h"
#include "../ui/ui_theme.h"
#include "../ui/save_glow.h"
#include "../ui/settings_effects.h"
#include "../ui/ui_font.h"
#include "../ui/i18n.h"
#include "../sound/sound.h"
#include "../eyes/imu_orientation.h"
#include "../eyes/eyes_api.h"
#include "../../pin_config.h"

namespace {
uint8_t tab = 0;
bool save_failed = false;
enum class SaveGlow { Idle, Pending, Playing };
SaveGlow save_glow_state = SaveGlow::Idle;
uint32_t save_started = 0;
bool sound_options = false;
bool diagnostics_open = false;
uint8_t diagnostics_first = 0;
uint8_t first_row = 0;
bool grid_open = false;
bool filter_open = false, keep_one = false;
bool favorites_open = false, favorites_edit = false;
uint8_t grid_first_row = 0;
constexpr bool AMOLED_MENU = ui_font::Supported;
constexpr uint8_t NO_ROW = 255;
constexpr uint32_t SLIDE_MS = 220;
uint8_t painted_tab = NO_ROW, painted_first = 0, sliding_row = NO_ROW;
uint8_t previous_tab = 0;
int8_t title_direction = 0;
String painted_values[3], previous_value;
bool slide_pending = false;
uint32_t slide_started = 0, slide_elapsed = 0;

/** Fewer, finger-sized expressions per view instead of fifteen narrow cells. */
constexpr uint8_t GRID_COLS = 2;
constexpr uint8_t GRID_ROWS = 3;
constexpr uint8_t GRID_ROW_PX = 44;
constexpr int16_t GRID_X0 = 8;
constexpr int16_t GRID_Y0 = 92;
constexpr int16_t GRID_W = 108;
constexpr int16_t GRID_H = 42;
constexpr uint8_t GRID_ROWS_TOTAL = (EXPRESSION_COUNT + GRID_COLS - 1) / GRID_COLS;
constexpr uint8_t GRID_MAX_FIRST = GRID_ROWS_TOTAL > GRID_ROWS ? GRID_ROWS_TOTAL - GRID_ROWS : 0;

/** @brief Save from either menu footer and start the visual confirmation on success. */
void save_from_menu() {
	save_failed = !settings_save();
	save_glow_state = save_glow::Supported && !save_failed ? SaveGlow::Pending : SaveGlow::Idle;
}

/** @brief Map a compact favorite cell to a stable catalog ID; excluded stars stay stored. */
uint16_t grid_index(uint16_t cell) {
	if (!favorites_open || favorites_edit) { return cell; }
	for (uint16_t id = 0; id < EXPRESSION_COUNT; ++id) {
		if (settings_favorites().contains(Expression(id)) && face_filter().allows(Expression(id))) {
			if (!cell) { return id; } --cell;
		}
	}
	return EXPRESSION_COUNT;
}
/** @brief Count selectable stars or the complete catalog in editing/ordinary grids. */
uint8_t grid_count() {
	if (!favorites_open || favorites_edit) { return EXPRESSION_COUNT; }
	uint8_t count = 0;
	for (uint16_t id = 0; id < EXPRESSION_COUNT; ++id) {
		count += settings_favorites().contains(Expression(id)) && face_filter().allows(Expression(id)) ? 1 : 0;
	}
	return count;
}
/** @brief Bound scrolling to the actual compact list. */
uint8_t grid_limit() {
	uint8_t rows = (grid_count() + GRID_COLS - 1) / GRID_COLS;
	return rows > GRID_ROWS ? rows - GRID_ROWS : 0;
}
enum MenuTab : uint8_t { TAB_LOOK, TAB_LIGHT, TAB_FACES, TAB_MOTION, TAB_AUDIO, TAB_SYSTEM, TAB_COUNT };
enum SystemRow : uint8_t {
	SYS_LANGUAGE, SYS_FONT, SYS_DASHBOARD = 1 + AMOLED_MENU, SYS_ROUTE, SYS_PHONE,
	SYS_UPDATE, SYS_WEB, SYS_DIAGNOSTICS, SYS_SD
};

/** SD sounds only appears on the board with an SD slot and codec. */
#if MOCHI_HAS_SD_AUDIO
constexpr bool HAS_SD_ROW = true;
#else
constexpr bool HAS_SD_ROW = false;
#endif
/** Row indices in the main SOUND list, shifted on boards without SD sounds. */
constexpr uint8_t SOUND_OPTIONS_ROW = HAS_SD_ROW ? 4 : 3;
constexpr uint8_t SOUND_TEST_ROW = HAS_SD_ROW ? 5 : 4;

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

/** @brief Number of rows in the current panel. */
uint8_t row_count() {
	static const uint8_t counts[] = {3, 7, 6, 5, 5};
	if (tab == TAB_SYSTEM) { return SYS_SD + HAS_SD_ROW; }
	if (tab != TAB_AUDIO) { return counts[tab]; }
	return sound_options ? 5 : (HAS_SD_ROW ? 6 : 5);
}

/** @brief Close the grid and restore the tab rows. */
void grid_close() {
	grid_open = filter_open = keep_one = favorites_open = favorites_edit = false;
	grid_first_row = 0; first_row = 0;
}

/** @brief Draw a chevron using the current accent color. */
void chevron(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t size, int8_t direction) {
	const int16_t tail = x - direction * size / 2;
	const int16_t tip = tail + direction * size;
	gfx->drawLine(tail, y - size, tip, y, UI_ACCENT);
	gfx->drawLine(tip, y, tail, y + size, UI_ACCENT);
}

/** @brief Center a label vertically and horizontally, reducing its size if needed. */
void centered_label(Arduino_GFX *gfx, const char *text, int16_t x, int16_t width,
	int16_t mid_y, uint16_t color, uint16_t background) {
	const String ascii = ui_ascii(text);
	const uint8_t size = ui_font::width(ascii, 2) <= width ? 2 : 1;
	const int16_t text_width = ui_font::width(ascii, size);
	const int16_t offset = text_width < width ? (width - text_width) / 2 : 0;
	ui_font::label(gfx, text, x + offset, mid_y - ui_font::height(size) / 2,
		width - offset, size, color, background);
}

/** @brief Draw a compact category pill on the AMOLED panel. */
void category_pill(Arduino_GFX *gfx, const char *label) {
	const int16_t y = ui_s(52), height = ui_s(36), mid = y + height / 2;
	gfx->fillRoundRect(ui_s(20), y, LCD_WIDTH - 2 * ui_s(20), height, height / 2, UI_CARD);
	chevron(gfx, ui_s(44), mid, ui_s(6), -1);
	chevron(gfx, LCD_WIDTH - ui_s(44), mid, ui_s(6), 1);
	const int16_t left = ui_s(72), width = LCD_WIDTH - 2 * left;
	if (!title_direction || slide_pending) { centered_label(gfx, label, left, width, mid, UI_ACCENT, UI_CARD); return; }
	const uint32_t half = SLIDE_MS / 2;
	const bool leaving = slide_elapsed < half;
	const uint32_t motion = leaving ? slide_elapsed : SLIDE_MS - slide_elapsed;
	const int16_t shift = int16_t(ui_s(12) * motion / half) * title_direction * (leaving ? -1 : 1);
	const uint16_t opacity = uint16_t((half - motion) * 256 / half);
	if (opacity) { centered_label(gfx, leaving ? tr_settings_tab(previous_tab) : label,
		left + shift, width, mid, ui_mix(UI_CARD, UI_ACCENT, opacity), UI_CARD); }
}

/** @brief Draw the footer shared by the visible tabs and expression grids. */
void menu_footer(Arduino_GFX *gfx, Text right) {
	const int16_t top = ui_s(224), bottom = bottom_band_bottom(), mid = (top + bottom) / 2;
	const Text left = save_failed ? Text::Failed : (AMOLED_MENU || settings_dirty() ? Text::Save : Text::Saved);
	if (AMOLED_MENU) {
		const int16_t height = ui_s(44), side = ui_s(12), gap = ui_s(5);
		const int16_t width = LCD_WIDTH / 2 - gap - side;
		const int16_t right_x = LCD_WIDTH - side - width;
		gfx->fillRoundRect(side, mid - height / 2, width, height, ui_s(10), UI_ACCENT);
		gfx->fillRoundRect(right_x, mid - height / 2, width, height, ui_s(10), UI_CARD);
		centered_label(gfx, tr(left), side + ui_s(6), width - 2 * ui_s(6), mid, UI_BG, UI_ACCENT);
		centered_label(gfx, tr(right), right_x + ui_s(6), width - 2 * ui_s(6), mid, UI_TEXT, UI_CARD);
		return;
	}
	gfx->fillRoundRect(ui_s(2), top, ui_s(116), bottom - top, ui_s(8), UI_ACCENT);
	gfx->fillRoundRect(ui_s(122), top, ui_s(116), bottom - top, ui_s(8), UI_CARD);
	ui_label(gfx, tr(left), ui_s(36), mid - ui_s(4), ui_s(72), 1, UI_BG);
	ui_label(gfx, tr(right), ui_s(147), mid - ui_s(4), ui_s(72), 1, UI_TEXT);
}

/** @brief Fit a setting label and right-aligned value on one line with a trailing chevron. */
void row_inline(Arduino_GFX *gfx, uint8_t index, int16_t y, const char *label, const String &value) {
	const int16_t left = ui_s(24), right = LCD_WIDTH - ui_s(33);
	const int16_t unit = ui_font::active() ? 1 : 6 * ui_text_size(1);
	const int16_t mid = y + ui_s(44) / 2, top = mid - ui_font::height(1) / 2;
	const int16_t travel = ui_s(6);
	const int16_t units = (right - left - ui_s(6) - travel) / unit;
	const String next = ui_ascii(value);
	int16_t value_units = units - ui_font::width(ui_ascii(label), 1) / unit;
	if (value_units < units / 3) { value_units = units / 3; }
	const bool sliding = !slide_pending && sliding_row == index &&
		std::strcmp(previous_value.c_str(), value.c_str()) != 0;
	const String old = sliding ? ui_font::fit(ui_ascii(previous_value), value_units * unit, 1) : String();
	const String current = ui_font::fit(next, value_units * unit, 1);
	const int16_t old_width = ui_font::width(old, 1), new_width = ui_font::width(current, 1);
	const int16_t largest = old_width > new_width ? old_width : new_width;
	ui_font::label(gfx, label, left, top, units * unit - largest, 1, UI_MUTED, UI_CARD);
	const uint32_t half = SLIDE_MS / 2;
	const bool leaving = sliding && slide_elapsed < half;
	const int16_t width = leaving ? old_width : new_width;
	const uint32_t motion = leaving ? slide_elapsed : (sliding ? SLIDE_MS - slide_elapsed : 0);
	const uint16_t opacity = uint16_t((half - motion) * 256 / half);
	const int16_t shift = int16_t(travel * motion / half) * (leaving ? -1 : 1);
	if (opacity) { ui_font::label(gfx, leaving ? old : current, right - width + shift, top, width, 1,
		ui_mix(UI_CARD, UI_ACCENT, opacity), UI_CARD); }
	chevron(gfx, LCD_WIDTH - ui_s(23), mid, ui_s(4), 1);
	painted_values[index - first_row] = value;
}

/** @brief Draw one editable row. @param gfx Canvas. @param index Row. @param label Field. @param value Choice. */
void row(Arduino_GFX *gfx, uint8_t index, const char *label, const String &value) {
	if (index < first_row || index >= first_row + 3) { return; }
	int16_t design_y = int16_t(90 + (index - first_row) * 44);
	int16_t y = ui_s(design_y);
	if (AMOLED_MENU) {
		const int16_t side = ui_s(12), gutter = ui_s(4);
		const int16_t bottom = ui_s(design_y + 44);
		gfx->fillRoundRect(side, y + gutter / 2, LCD_WIDTH - 2 * side,
			bottom - y - gutter, ui_s(14), UI_CARD);
		row_inline(gfx, index, y, label, value);
		return;
	}
	ui_card(gfx, design_y, 44);
	ui_label(gfx, label, ui_s(24), y + ui_s(5), ui_s(192), 1, UI_MUTED);
	ui_label(gfx, value, ui_s(24), y + ui_s(23), ui_s(192), 1, UI_ACCENT);
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
		if (index == SYS_LANGUAGE) { p.language = (p.language + 1) % 2; }
		if (AMOLED_MENU && index == SYS_FONT) { p.font = ui_font::next(p.font); }
		if (index == SYS_DASHBOARD) { p.dashboard = !p.dashboard; }
		if (index == SYS_ROUTE) { p.route_large = !p.route_large; }
		if (index == SYS_PHONE) { p.phone_ios = !p.phone_ios; }
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
		row(gfx, SYS_LANGUAGE, tr(Text::Language), p.language ? "Italiano" : "English");
		if (AMOLED_MENU) { row(gfx, SYS_FONT, tr(Text::Font), ui_font::name(p.font)); }
		row(gfx, SYS_DASHBOARD, tr(Text::Dashboard), on_off(p.dashboard));
		row(gfx, SYS_ROUTE, tr(Text::RouteDefault), tr(p.route_large ? Text::Large : Text::Details));
		row(gfx, SYS_PHONE, tr(Text::PhoneMode), tr(p.phone_ios ? Text::IphoneMode : Text::AndroidMode));
		row(gfx, SYS_UPDATE, tr(Text::FirmwareUpdate), tr(Text::Open));
		row(gfx, SYS_WEB, tr(Text::WebSettings), tr(Text::Open));
		row(gfx, SYS_DIAGNOSTICS, tr(Text::Diagnostics), tr(Text::Open));
#if MOCHI_HAS_SD_AUDIO
		row(gfx, SYS_SD, tr(Text::SdFirmware), tr(Text::Open));
#endif
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

/** @brief Draw the selection/library header and the optional Wi-Fi action. */
void grid_header(Arduino_GFX *gfx) {
	uint8_t first = grid_count() ? uint8_t(grid_first_row * GRID_COLS + 1) : 0;
	uint8_t last = uint8_t(first + GRID_ROWS * GRID_COLS - 1);
	if (last > grid_count()) { last = grid_count(); }
	gfx->fillRoundRect(ui_s(2), ui_s(52), ui_s(236), ui_s(36), ui_s(8), UI_CARD);
	if (filter_open) {
		ui_font::label(gfx, tr(keep_one ? Text::KeepOneFace : Text::FaceLibrary), ui_s(14), ui_s(58), ui_s(146), 1, UI_ACCENT, UI_CARD);
		ui_font::label(gfx, String(face_filter().count()) + "/" + String(EXPRESSION_COUNT) + " " + tr(Text::Enabled),
			ui_s(14), ui_s(74), ui_s(146), 1, UI_MUTED, UI_CARD);
		gfx->fillRoundRect(ui_s(170), ui_s(52), ui_s(68), ui_s(36), ui_s(8), UI_ACCENT);
		ui_font::label(gfx, "Wi-Fi", ui_s(182), ui_s(66), ui_s(50), 1, UI_BG, UI_ACCENT);
		return;
	}
	ui_font::label(gfx, tr(favorites_open ? (favorites_edit ? Text::EditFavorites : Text::Favorites) : Text::MoodLabel),
		ui_s(14), ui_s(66), ui_s(130), 1, UI_ACCENT, UI_CARD);
	ui_font::label(gfx, String(first) + "-" + String(last) + "/" +
		String(grid_count()), ui_s(150), ui_s(66), ui_s(76), 1, UI_MUTED, UI_CARD);
}
/** @brief Draw an enabled checkbox or a muted excluded entry without relying on color alone. */
void grid_entry(Arduino_GFX *gfx, uint8_t index, int16_t x, int16_t y) {
	bool allowed = face_filter().allows(Expression(index));
	bool selected = !filter_open && !favorites_edit && allowed && index == settings_get().mood;
	gfx->fillRoundRect(x, y, ui_s(GRID_W), ui_s(GRID_H), ui_s(6), selected ? UI_ACCENT : UI_CARD);
	if (filter_open) {
		gfx->drawRect(x + ui_s(3), y + ui_s(17), ui_s(8), ui_s(8), UI_TEXT);
		if (allowed) { gfx->fillRect(x + ui_s(5), y + ui_s(19), ui_s(4), ui_s(4), UI_ACCENT); }
	}
	if (favorites_edit) {
		ui_label(gfx, settings_favorites().contains(Expression(index)) ? "*" : "-",
			x + ui_s(3), y + ui_s(17), ui_s(8), 1, UI_ACCENT);
	}
	int16_t pad = filter_open || favorites_edit ? 14 : 3;
	ui_font::label(gfx, tr_expression(index), x + ui_s(pad), y + ui_s(17), ui_s(GRID_W - pad - 3), 1,
		selected ? UI_BG : (allowed ? UI_TEXT : UI_MUTED), selected ? UI_ACCENT : UI_CARD);
	if (!filter_open && !allowed) {
		gfx->drawLine(x + ui_s(3), y + ui_s(4), x + ui_s(10), y + ui_s(20), UI_MUTED);
	}
}
/** @brief Draw the expression grid overlay, replacing the tab rows. @param gfx Canvas. */
void grid_draw(Arduino_GFX *gfx) {
	if (grid_first_row > grid_limit()) { grid_first_row = grid_limit(); }
	grid_header(gfx);
	if (!grid_count()) { ui_font::label(gfx, tr(Text::NoFavorites), ui_s(14), ui_s(140), ui_s(212), 1, UI_MUTED, UI_BG); }
	for (uint8_t cell = 0; cell < GRID_ROWS * GRID_COLS; ++cell) {
		uint16_t index = grid_index(uint16_t(grid_first_row) * GRID_COLS + cell);
		if (index >= EXPRESSION_COUNT) { break; }
		int16_t x = ui_s(int16_t(GRID_X0 + (cell % GRID_COLS) * (GRID_W + 4)));
		int16_t y = ui_s(int16_t(GRID_Y0 + (cell / GRID_COLS) * GRID_ROW_PX));
		grid_entry(gfx, uint8_t(index), x, y);
	}
	if (grid_limit()) {
		int16_t track = int16_t(GRID_ROWS * GRID_ROW_PX) - 2;
		gfx->fillRect(ui_s(234), ui_s(GRID_Y0), ui_s(3), ui_s(track), UI_CARD);
		gfx->fillRect(ui_s(234), ui_s(int16_t(GRID_Y0 + grid_first_row * (track - 40) / grid_limit())),
			ui_s(3), ui_s(40), UI_ACCENT);
	}
	menu_footer(gfx, Text::Back);
}

/**
 * @brief Apply a tapped grid cell; rows are contiguous full-height targets
 * and only the thin column gutters stay inert.
 * @param x X. @param y Y. @return True when a real cell was hit.
 */
bool grid_cell(int16_t x, int16_t y) {
	for (uint8_t row = 0; row < GRID_ROWS; ++row) {
		int16_t cell_y = ui_s(int16_t(GRID_Y0 + row * GRID_ROW_PX));
		if (y < cell_y || y >= ui_s(int16_t(GRID_Y0 + (row + 1) * GRID_ROW_PX))) { continue; }
		for (uint8_t col = 0; col < GRID_COLS; ++col) {
			int16_t cell_x = ui_s(int16_t(GRID_X0 + col * (GRID_W + 4)));
			if (x < cell_x || x >= cell_x + ui_s(GRID_W)) { continue; }
			if (grid_first_row > grid_limit()) { grid_first_row = grid_limit(); }
			uint16_t index = grid_index(uint16_t(grid_first_row + row) * GRID_COLS + col);
			if (index >= EXPRESSION_COUNT) { return false; }
			if (favorites_edit) {
				FaceFavorites next = settings_favorites();
				next.set(Expression(index), !next.contains(Expression(index)));
				settings_favorites_update(next); save_failed = false; return true;
			}
			if (filter_open) {
				FaceFilter next = face_filter();
				keep_one = !next.set(Expression(index), !next.allows(Expression(index)));
				if (!keep_one) { settings_faces_update(next); }
				save_failed = false;
				return true;
			}
			if (!face_filter().allows(Expression(index))) { return false; }
			CompanionSettings p = settings_get();
			p.mood = uint8_t(index);
			save_failed = false;
			settings_update(p);
			return true;
		}
		return false;
	}
	return false;
}
}

/** @brief Draw the diagnostic overlay or compact settings tabs. @param gfx Canvas. */
void settings_menu_draw(Arduino_GFX *gfx) {
	painted_tab = NO_ROW;
	if (diagnostics_open) { diagnostics_view::draw(gfx, diagnostics_first); return; }
	if (grid_open) { grid_draw(gfx); return; }
	painted_tab = tab;
	painted_first = first_row;
	const CompanionSettings &p = settings_get();
	if (AMOLED_MENU) {
		category_pill(gfx, tr_settings_tab(tab));
	} else {
		gfx->fillRoundRect(ui_s(2), ui_s(52), ui_s(68), ui_s(36), ui_s(8), UI_CARD);
		gfx->fillRoundRect(ui_s(170), ui_s(52), ui_s(68), ui_s(36), ui_s(8), UI_CARD);
		ui_label(gfx, "<", ui_s(30), ui_s(62), ui_s(24), 2, UI_TEXT);
		ui_label(gfx, ">", ui_s(198), ui_s(62), ui_s(24), 2, UI_TEXT);
		ui_label(gfx, tr_settings_tab(tab), ui_s(78), ui_s(58), ui_s(84), 1, UI_ACCENT);
		ui_label(gfx, String(tab + 1) + "/" + String(uint8_t(TAB_COUNT)), ui_s(108), ui_s(74), ui_s(24), 1, UI_MUTED);
	}
	if (tab < 3) { basic_rows(gfx, p); } else { extra_rows(gfx, p); }
	if (row_count() > 3) {
		uint8_t max_scroll = row_count() - 3;
		gfx->fillRect(ui_s(232), ui_s(92), ui_s(3), ui_s(128), UI_CARD);
		gfx->fillRect(ui_s(232), ui_s(int16_t(92 + first_row * (128 - 60) / max_scroll)), ui_s(3), ui_s(60), UI_ACCENT);
	}
	menu_footer(gfx, tab == 4 && sound_options ? Text::Back : Text::EyesButton);
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
			if (x < ui_s(120)) { save_from_menu(); } else { grid_close(); }
			return MenuAction::Changed;
		}
		return grid_cell(x, y) ? MenuAction::Changed : MenuAction::None;
	}
	if (y >= ui_s(52) && y < ui_s(90)) {
		if (x >= ui_s(72) && x < ui_s(168)) { return MenuAction::None; }
		sliding_row = NO_ROW;
		const int8_t direction = x < ui_s(72) ? -1 : 1;
		previous_tab = tab;
		tab = (tab + TAB_COUNT + direction) % TAB_COUNT;
		if (AMOLED_MENU) { title_direction = direction; slide_pending = true; }
		sound_options = false; first_row = 0;
		return MenuAction::Changed;
	}
	if (y >= ui_s(224) && y < bottom_band_bottom()) {
		if (x >= ui_s(120)) {
			if (tab == 4 && sound_options) { sound_options = false; first_row = 0; return MenuAction::Changed; }
			grid_close();
			return MenuAction::Exit;
		}
		save_from_menu();
		return MenuAction::Changed;
	}
	if (y >= ui_s(90) && y < ui_s(222)) {
		uint8_t visible = 0;
		while (visible < 2 && y >= ui_s(int16_t(90 + (visible + 1) * 44))) { ++visible; }
		uint8_t index = uint8_t(first_row + visible);
		uint8_t count = row_count();
		if (tab == TAB_SYSTEM && HAS_SD_ROW && index == SYS_SD) { return MenuAction::SdUpdate; }
		if (tab == TAB_SYSTEM && index == SYS_UPDATE) { return MenuAction::Update; }
		if (tab == TAB_SYSTEM && index == SYS_WEB) { return MenuAction::FacesWeb; }
		if (tab == TAB_SYSTEM && index == SYS_DIAGNOSTICS) {
			diagnostics_open = true; diagnostics_first = 0; return MenuAction::Changed;
		}
		if (index < count) {
			const bool was_sound_options = sound_options;
			change_row(index);
			if (AMOLED_MENU && painted_tab == tab && painted_first == first_row &&
				!grid_open && was_sound_options == sound_options) {
				previous_value = painted_values[visible];
				sliding_row = index;
				title_direction = 0;
				slide_pending = true;
			}
			return MenuAction::Changed;
		}
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

uint32_t settings_menu_save_elapsed(uint32_t now, bool visible) {
	if (!visible) { save_glow_state = SaveGlow::Idle; }
	if (save_glow_state == SaveGlow::Pending) {
		save_started = now;
		save_glow_state = SaveGlow::Playing;
	}
	const uint32_t elapsed = now - save_started;
	if (elapsed >= save_glow::Duration) { save_glow_state = SaveGlow::Idle; }
	return save_glow_state == SaveGlow::Playing ? elapsed : save_glow::Duration;
}

bool settings_menu_animate(uint32_t now, bool visible) {
	if (!visible || grid_open || diagnostics_open) { sliding_row = NO_ROW; title_direction = 0; }
	if (sliding_row == NO_ROW && !title_direction) { return false; }
	if (slide_pending) { slide_started = now; slide_pending = false; }
	slide_elapsed = now - slide_started;
	if (slide_elapsed >= SLIDE_MS) { sliding_row = NO_ROW; title_direction = 0; }
	return true;
}
