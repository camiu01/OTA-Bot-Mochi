/**
 * @file context_dice.h
 * @brief Standard D&D pools with count/type controls, sum and browsable individual results.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "ui_theme.h"
#include "i18n.h"
#include "../play/dice.h"

/** @brief Keep the selection and result across page changes without saving preferences. @return Dice session. */
inline DicePool &desk_dice() { static DicePool dice; return dice; }
/** @brief Draw one bounded count/type action. @param gfx Canvas. @param x Design left. @param y Design top. @param width Design width. @param label Label. */
inline void dice_button(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t width, const char *label) {
	gfx->fillRoundRect(ui_s(x), ui_s(y), ui_s(width), ui_s(40), ui_s(6), UI_CARD);
	ui_label(gfx, label, ui_s(x + 8), ui_s(y + 16), ui_s(width - 16), 1, UI_TEXT);
}
/** @brief Render the last roll without consuming any random values during repaint. @param gfx Canvas. */
inline void desk_dice_draw(Arduino_GFX *gfx) {
	const auto &dice = desk_dice();
	dice_button(gfx, 6, 56, 44, "<"); dice_button(gfx, 190, 56, 44, ">");
	ui_label(gfx, String(dice.count()) + "D" + String(dice.sides()), ui_s(62), ui_s(68), ui_s(116), 2, UI_TEXT);
	dice_button(gfx, 6, 100, 44, "-10"); dice_button(gfx, 54, 100, 32, "-");
	dice_button(gfx, 154, 100, 32, "+"); dice_button(gfx, 190, 100, 44, "+10");
	ui_label(gfx, "x" + String(dice.count()), ui_s(92), ui_s(112), ui_s(56), 2, UI_TEXT);
	ui_card(gfx, 146, 66);
	ui_label(gfx, String(tr(Text::DiceTotal)) + ": " + (dice.rolled() ? String(dice.total()) : String("--")),
		ui_s(18), ui_s(153), ui_s(150), 1, UI_ACCENT);
	if (dice.rolled()) {
		ui_label(gfx, "<" + String(dice.page() + 1) + "/" + String(dice.pages()) + ">",
			ui_s(174), ui_s(153), ui_s(48), 1, UI_MUTED);
		for (uint8_t i = 0; i < DicePool::PER_PAGE; ++i) {
			uint8_t at = dice.page() * DicePool::PER_PAGE + i;
			if (at >= dice.count()) { break; }
			ui_label(gfx, String(dice.value(at)), ui_s(18 + (i % 6) * 34), ui_s(173 + (i / 6) * 18), ui_s(30), 1, UI_TEXT);
		}
	}
	gfx->fillRoundRect(ui_s(6), ui_s(218), ui_s(228), ui_s(40), ui_s(8), UI_ACCENT);
	ui_label(gfx, tr(Text::DiceRoll), ui_s(18), ui_s(230), ui_s(204), 2, UI_BG);
}
/** @brief Match drawn controls, retaining results while browsing their pages. @param x Pixel X. @param y Pixel Y. @return Changed. */
inline bool desk_dice_tap(int16_t x, int16_t y) {
	if (x < ui_s(6) || x >= ui_s(234)) { return false; }
	auto &dice = desk_dice();
	bool left = x < ui_s(50), right = x >= ui_s(190);
	if (y >= ui_s(56) && y < ui_s(96) && (left || right)) { dice.cycle_type(right ? 1 : -1); return true; }
	if (y >= ui_s(100) && y < ui_s(140)) {
		if (left || right) { return dice.adjust_count(right ? 10 : -10); }
		if (x >= ui_s(54) && x < ui_s(86)) { return dice.adjust_count(-1); }
		if (x >= ui_s(154) && x < ui_s(186)) { return dice.adjust_count(1); }
	}
	if (x >= ui_s(12) && x < ui_s(228) && y >= ui_s(146) && y < ui_s(212)) { return dice.move_page(x >= ui_s(120) ? 1 : -1); }
	if (y >= ui_s(218) && y < ui_s(258)) {
		dice.roll([](uint8_t limit) { return random(long(limit)); }); return true;
	}
	return false;
}
