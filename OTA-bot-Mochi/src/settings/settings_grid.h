/**
 * @file settings_grid.h
 * @brief Expression grid, face library and favorites overlay of the settings menu.
 *
 * Private to settings_menu.cpp: it relies on that file's save_failed, first_row and
 * bottom_band_bottom(), which are declared before this header is included.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

namespace {
bool grid_open = false;
bool filter_open = false, keep_one = false;
bool favorites_open = false, favorites_edit = false;
uint8_t grid_first_row = 0;

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

/** @brief Close the grid and restore the tab rows. */
void grid_close() {
	grid_open = filter_open = keep_one = favorites_open = favorites_edit = false;
	grid_first_row = 0; first_row = 0;
}

/** @brief Draw the selection/library header and the optional Wi-Fi action. */
void grid_header(Arduino_GFX *gfx) {
	uint8_t first = grid_count() ? uint8_t(grid_first_row * GRID_COLS + 1) : 0;
	uint8_t last = uint8_t(first + GRID_ROWS * GRID_COLS - 1);
	if (last > grid_count()) { last = grid_count(); }
	ui_box(gfx, ui_s(2), ui_s(52), ui_s(236), ui_s(36), UI_CARD, true);
	if (filter_open) {
		ui_label(gfx, tr(keep_one ? Text::KeepOneFace : Text::FaceLibrary), ui_s(14), ui_s(58), ui_s(146), 1, UI_ACCENT);
		ui_label(gfx, String(face_filter().count()) + "/" + String(EXPRESSION_COUNT) + " " + tr(Text::Enabled),
			ui_s(14), ui_s(74), ui_s(146), 1, UI_MUTED);
		ui_button(gfx, ui_s(170), ui_s(52), ui_s(68), ui_s(36), "WI-FI", true);
		return;
	}
	ui_label(gfx, tr(favorites_open ? (favorites_edit ? Text::EditFavorites : Text::Favorites) : Text::MoodLabel),
		ui_s(14), ui_s(66), ui_s(130), 1, UI_ACCENT);
	ui_label(gfx, String(first) + "-" + String(last) + "/" +
		String(grid_count()), ui_s(150), ui_s(66), ui_s(76), 1, UI_MUTED);
}
/** @brief Draw an enabled checkbox or a muted excluded entry without relying on color alone. */
void grid_entry(Arduino_GFX *gfx, uint8_t index, int16_t x, int16_t y) {
	bool allowed = face_filter().allows(Expression(index));
	bool selected = !filter_open && !favorites_edit && allowed && index == settings_get().mood;
	ui_box(gfx, x, y, ui_s(GRID_W), ui_s(GRID_H), selected ? UI_ACCENT : UI_CARD, !selected);
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
	if (!grid_count()) { ui_label(gfx, tr(Text::NoFavorites), ui_s(14), ui_s(140), ui_s(212), 1, UI_MUTED); }
	for (uint8_t cell = 0; cell < GRID_ROWS * GRID_COLS; ++cell) {
		uint16_t index = grid_index(uint16_t(grid_first_row) * GRID_COLS + cell);
		if (index >= EXPRESSION_COUNT) { break; }
		int16_t x = ui_s(int16_t(GRID_X0 + (cell % GRID_COLS) * (GRID_W + 4)));
		int16_t y = ui_s(int16_t(GRID_Y0 + (cell / GRID_COLS) * GRID_ROW_PX));
		grid_entry(gfx, uint8_t(index), x, y);
	}
	if (grid_limit()) {
		int16_t track = int16_t(GRID_ROWS * GRID_ROW_PX) - 2;
		gfx->fillRoundRect(ui_s(234), ui_s(GRID_Y0), ui_s(3), ui_s(track), ui_s(1), UI_LINE);
		gfx->fillRect(ui_s(234), ui_s(int16_t(GRID_Y0 + grid_first_row * (track - 40) / grid_limit())),
			ui_s(3), ui_s(40), UI_ACCENT);
	}
	save_band(gfx, tr(Text::Back));
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
