/**
 * @file context_count.h
 * @brief Desk counters: a single tally and a two-player scoreboard, RAM-only.
 *
 * TALLY: tap the card for +1, the bottom row for -1 or RESET; hold resets.
 * SCORE: tap the left or right half for that side's +1, the bottom row for
 * -1 on the last side changed or RESET; hold resets both.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "screens.h"
#include "ui_theme.h"
#include "i18n.h"
#include "../../pin_config.h"

/** @brief Session counters shared by TALLY and SCORE. */
struct DeskCounters {
	int16_t tally = 0;
	int16_t score[2] = {0, 0};
	uint8_t last = 0;	/**< Side changed last on SCORE. */
	/** @brief Clamp a counter step to +/-9999. @param value Counter. @param step Change. */
	static void add(int16_t &value, int16_t step) {
		int32_t next = int32_t(value) + step;
		value = int16_t(next > 9999 ? 9999 : (next < -9999 ? -9999 : next));
	}
	void reset(UiPage page) { if (page == UiPage::Tally) { tally = 0; } else { score[0] = score[1] = 0; last = 0; } }
};
/** @brief Single session instance. @return Counters. */
inline DeskCounters &desk_counters() { static DeskCounters value; return value; }

/** @brief Draw TALLY or SCORE. @param gfx Canvas. @param page Tally or Score. */
inline void desk_count_draw(Arduino_GFX *gfx, UiPage page) {
	const DeskCounters &c = desk_counters();
	ui_card(gfx, 56, 148);
	if (page == UiPage::Tally) {
		ui_center(gfx, tr(Text::TallyHint), 70, 1, UI_MUTED);
		ui_center(gfx, String(c.tally), 104, 6, UI_TEXT);
	} else {
		gfx->drawFastVLine(LCD_WIDTH / 2, ui_s(70), ui_s(120), UI_LINE);
		for (uint8_t side = 0; side < 2; ++side) {
			int16_t x = side ? LCD_WIDTH / 2 : 0;
			ui_label(gfx, side ? "B" : "A", x + ui_s(14), ui_s(68), ui_s(30), 1, c.last == side ? UI_ACCENT : UI_MUTED);
			String value(c.score[side]);
			uint8_t size = value.length() > 2 ? 3 : 5;
			int16_t w = int16_t(value.length() * 6 * ui_text_size(size));
			ui_label(gfx, value, x + (LCD_WIDTH / 2 - w) / 2, ui_s(110), LCD_WIDTH / 2, size, UI_TEXT);
		}
	}
	ui_button(gfx, ui_s(2), ui_s(212), ui_s(116), ui_s(40), "-1", false);
	ui_button(gfx, ui_s(122), ui_s(212), ui_s(116), ui_s(40), tr(Text::Reset), false);
}

/** @brief Handle one tap. @param page Tally or Score. @param x Pixel X. @param y Pixel Y. @return Changed. */
inline bool desk_count_tap(UiPage page, int16_t x, int16_t y) {
	DeskCounters &c = desk_counters();
	if (y >= ui_s(56) && y < ui_s(204)) {
		if (page == UiPage::Tally) { DeskCounters::add(c.tally, 1); }
		else { c.last = x >= LCD_WIDTH / 2; DeskCounters::add(c.score[c.last], 1); }
		return true;
	}
	if (y < ui_s(212) || y >= ui_s(252)) { return false; }
	if (x >= ui_s(122)) { c.reset(page); return true; }
	DeskCounters::add(page == UiPage::Tally ? c.tally : c.score[c.last], -1);
	return true;
}
