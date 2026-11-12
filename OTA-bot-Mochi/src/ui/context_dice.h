/**
 * @file context_dice.h
 * @brief Standard D&D pools with count/type controls, sum and browsable individual results.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "ui_theme.h"
#include "i18n.h"
#include "../play/dice.h"
#include "../eyes/eyes_api.h"
#include "dice_shape.h"
#include "../eyes/eye_shapes.h"
#include "../../pin_config.h"
#include <stdio.h>

/** @brief Keep the selection and result across page changes without saving preferences. @return Dice session. */
inline DicePool &desk_dice() { static DicePool dice; return dice; }
/** @brief Keep the detail panel local to the dice session. */
inline bool &desk_dice_details() { static bool details = false; return details; }
/** @brief Draw one bounded count/type action. @param gfx Canvas. @param x Design left. @param y Design top. @param width Design width. @param label Label. */
inline void dice_button(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t width, const char *label) {
	gfx->fillRoundRect(ui_s(x), ui_s(y), ui_s(width), ui_s(44), ui_s(6), UI_CARD);
	ui_label(gfx, label, ui_s(x + 8), ui_s(y + 18), ui_s(width - 16), 1, UI_TEXT);
}
/** @brief Use the physical bottom strip for tall actions, leaving a four-pixel inert edge. */
inline int16_t dice_footer_bottom() { return LCD_HEIGHT - 4; }
/** @brief Large roll/detail actions share their exact drawn bounds with released taps. */
inline void dice_footer_draw(Arduino_GFX *gfx) {
	bool details = desk_dice_details();
	gfx->fillRect(0, ui_s(214), LCD_WIDTH, LCD_HEIGHT - ui_s(214), UI_BG);
	int16_t height = dice_footer_bottom() - ui_s(224);
	for (bool first : {true, false}) {
		int16_t x = ui_s(first ? 2 : 122);
		bool roll = first != details;
		gfx->fillRoundRect(x, ui_s(224), ui_s(116), height, ui_s(8), roll ? UI_ACCENT : UI_CARD);
		ui_label(gfx, tr(roll ? Text::DiceRoll : (details ? Text::Back : Text::Details)),
			x + ui_s(12), ui_s(224) + height / 2 - 4 * ui_text_size(1), ui_s(92), 1, roll ? UI_BG : UI_TEXT);
	}
	if (eyes_imu_ready()) {
		ui_label(gfx, tr(Text::DiceShakeRoll), ui_s(18), ui_s(214), ui_s(204), 1, UI_MUTED);
	}
}
/** @brief Footer gutters and the panel edge never toggle or reroll. */
inline bool dice_footer_tap(int16_t x, int16_t y) {
	if (y < ui_s(224) || y >= dice_footer_bottom()) { return false; }
	bool first = x >= ui_s(2) && x < ui_s(118), second = x >= ui_s(122) && x < ui_s(238);
	if (!first && !second) { return false; }
	if (first == desk_dice_details()) { desk_dice_details() = !desk_dice_details(); }
	else { desk_dice().roll([](uint8_t limit) { return random(long(limit)); }); }
	return true;
}
/** @brief React beside a single D20 without hiding its result or changing the saved face. */
inline bool desk_dice_reaction(Arduino_GFX *gfx) {
	const auto &dice = desk_dice();
	if (!dice.rolled() || dice.sides() != 20 || dice.count() != 1 ||
		(dice.value(0) != 1 && dice.value(0) != 20)) { return false; }
	bool success = dice.value(0) == 20;
	EyeConfig cfg{};
	cfg.style = MOCHI; cfg.expression = success ? STAR : SAD;
	cfg.bgColor = UI_BG; cfg.fgColor = UI_ACCENT;
	for (bool left : {true, false}) {
		EyeBox box{ui_s(left ? 180 : 206), ui_s(138), ui_s(20), ui_s(20), ui_s(5), left};
		eye_body(gfx, cfg, box); eye_lids(gfx, cfg, box);
	}
	ui_label(gfx, tr(success ? Text::DiceCritical : Text::DiceFumble),
		ui_s(174), ui_s(166), ui_s(64), 1, UI_ACCENT);
	return true;
}
/** @brief Draw the large solid; a pool shows die one plus its separate total. */
inline void desk_dice_solid(Arduino_GFX *gfx) {
	const auto &dice = desk_dice();
	String result = dice.rolled() ? String(dice.value(0)) : String("--");
	if (dice.sides() == 100) {
		char tens[3]; snprintf(tens, sizeof(tens), "%02u", unsigned(dice.value(0) % 100 / 10 * 10));
		dice_shape_draw(gfx, 10, 72, 150, 40, dice.rolled() ? String(tens) : String("--"));
		dice_shape_draw(gfx, 10, 168, 150, 40, dice.rolled() ? String(dice.value(0) % 10) : String("-"));
	} else { dice_shape_draw(gfx, dice.sides(), 120, 150, 50, result); }
	desk_dice_reaction(gfx);
	if (dice.count() > 1) { ui_label(gfx, tr(Text::DiceFirst), ui_s(12), ui_s(102), ui_s(70), 1, UI_MUTED); }
	ui_label(gfx, String(tr(Text::DiceTotal)) + ": " + (dice.rolled() ? String(dice.total()) : String("--")),
		ui_s(18), ui_s(202), ui_s(204), 1, UI_ACCENT);
}
/** @brief Draw editable count, sum and paginated results only in the detail panel. */
inline void desk_dice_results(Arduino_GFX *gfx) {
	const auto &dice = desk_dice();
	dice_button(gfx, 6, 100, 48, "-10"); dice_button(gfx, 66, 100, 48, "-");
	dice_button(gfx, 126, 100, 48, "+"); dice_button(gfx, 186, 100, 48, "+10");
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
}
/** @brief Render the stored roll without consuming randomness. @param gfx Canvas. */
inline void desk_dice_draw(Arduino_GFX *gfx) {
	const auto &dice = desk_dice(); bool details = desk_dice_details();
	dice_button(gfx, 6, 56, 44, "<"); dice_button(gfx, 190, 56, 44, ">");
	ui_label(gfx, String(dice.count()) + "D" + String(dice.sides()), ui_s(62), ui_s(68), ui_s(116), 2, UI_TEXT);
	if (details) { desk_dice_results(gfx); }
	else { desk_dice_solid(gfx); }
	dice_footer_draw(gfx);
}
/** @brief Match drawn controls, retaining results while browsing their pages. @param x Pixel X. @param y Pixel Y. @return Changed. */
inline bool desk_dice_tap(int16_t x, int16_t y) {
	if (y >= ui_s(224)) { return dice_footer_tap(x, y); }
	if (x < ui_s(6) || x >= ui_s(234)) { return false; }
	auto &dice = desk_dice();
	bool left = x < ui_s(50), right = x >= ui_s(190);
	if (y >= ui_s(56) && y < ui_s(100) && (left || right)) { dice.cycle_type(right ? 1 : -1); return true; }
	if (desk_dice_details() && y >= ui_s(100) && y < ui_s(144)) {
		if (x < ui_s(54)) { return dice.adjust_count(-10); }
		if (x >= ui_s(66) && x < ui_s(114)) { return dice.adjust_count(-1); }
		if (x >= ui_s(126) && x < ui_s(174)) { return dice.adjust_count(1); }
		if (x >= ui_s(186)) { return dice.adjust_count(10); }
	}
	if (desk_dice_details() && x >= ui_s(12) && x < ui_s(228) && y >= ui_s(146) && y < ui_s(212)) {
		return dice.move_page(x >= ui_s(120) ? 1 : -1);
	}
	return false;
}
