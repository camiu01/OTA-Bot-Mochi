/**
 * @file context_hubs.h
 * @brief Shared desk/game tiles with explicit destinations and identical drawing/tap geometry.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "screens.h"
#include "ui_theme.h"
#include "i18n.h"
#include "text_layout.h"
#include "../settings/settings.h"

struct ContextTile { UiPage page; Text label; };
struct ContextHub { const ContextTile *tiles; uint8_t count; };
/** @brief Choose the visible tiles; phone tools are Android-only. @param games Game hub. @return Stable tile list. */
inline ContextHub context_hub(bool games) {
	static constexpr ContextTile desk[] = {
		{UiPage::BigClock, Text::BigClock}, {UiPage::Overview, Text::Overview},
		{UiPage::Timer, Text::TimerMenu}, {UiPage::Device, Text::Device},
		{UiPage::AnalogClock, Text::AnalogClock}, {UiPage::Phone, Text::CompanionTitle}
	};
	static constexpr ContextTile play[] = {
		{UiPage::Dice, Text::Dice}, {UiPage::Pong, Text::Pong}, {UiPage::Tris, Text::Tris}
	};
	return games ? ContextHub{play, 3} : ContextHub{desk, uint8_t(settings_get().phone_ios ? 5 : 6)};
}
/** @brief Render scaled tiles and the adjacent layer's navigation hint. @param gfx Canvas. @param games Game hub. */
inline void context_hub_draw(Arduino_GFX *gfx, bool games) {
	const auto hub = context_hub(games);
	uint8_t columns = uint8_t((ui_s(88) - 4) / (6 * ui_text_size(1)));
	for (uint8_t i = 0; i < hub.count; ++i) {
		int16_t x = ui_s(i % 2 ? 126 : 6), y = ui_s(58 + (i / 2) * 50);
		gfx->fillRoundRect(x, y, ui_s(108), ui_s(44), ui_s(8), UI_CARD);
		TextLines lines = wrap_text(tr(hub.tiles[i].label), columns);
		int16_t top = y + ui_s(24) - int16_t(lines.count ? lines.count : 1) * ui_s(8);
		for (uint8_t l = 0; l < lines.count && l < 2; ++l) {
			ui_label(gfx, lines.line[l], x + ui_s(10), top + l * ui_s(16), ui_s(88), 1, UI_TEXT);
		}
	}
	ui_label(gfx, tr(games ? Text::BackDesk : Text::GamesHint), ui_s(12), ui_s(238), ui_s(216), 1, UI_MUTED);
}
/** @brief Ignore card gaps, hidden entries and the footer hint. @param games Game hub. @param x Pixel X. @param y Pixel Y. @return Selected page or hub. */
inline UiPage context_hub_tap(bool games, int16_t x, int16_t y) {
	const auto hub = context_hub(games);
	for (uint8_t i = 0; i < hub.count; ++i) {
		int16_t left = ui_s(i % 2 ? 126 : 6), top = ui_s(58 + (i / 2) * 50);
		if (x >= left && x < left + ui_s(108) && y >= top && y < top + ui_s(44)) { return hub.tiles[i].page; }
	}
	return games ? UiPage::Games : UiPage::Desk;
}
