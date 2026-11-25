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
/** @brief Choose the visible tiles; phone tools are Android-only. @param page Desk, Games or DriveHub. @return Stable tile list. */
inline ContextHub context_hub(UiPage page) {
	static constexpr ContextTile desk[] = {
		{UiPage::BigClock, Text::BigClock}, {UiPage::Overview, Text::Overview},
		{UiPage::Timer, Text::TimerMenu}, {UiPage::Device, Text::Device},
		{UiPage::AnalogClock, Text::AnalogClock}, {UiPage::Phone, Text::CompanionTitle}
	};
	static constexpr ContextTile play[] = {
		{UiPage::Dice, Text::Dice}, {UiPage::Pong, Text::Pong}, {UiPage::Tris, Text::Tris},
		{UiPage::FlappyBird, Text::FlappyBird}
	};
	static constexpr ContextTile drive[] = {
		{UiPage::DriveLean, Text::DriveBigLean}, {UiPage::DriveG, Text::DriveGMeter},
		{UiPage::DriveGraph, Text::DriveGraphTitle}, {UiPage::DriveTrip, Text::DriveTrip},
		{UiPage::DriveObd, Text::DriveObd}
	};
	if (page == UiPage::DriveHub) { return ContextHub{drive, 5}; }
	return page == UiPage::Games ? ContextHub{play, 4} : ContextHub{desk, uint8_t(settings_get().phone_ios ? 5 : 6)};
}
/** @brief Render scaled tiles and the adjacent layer's navigation hint. @param gfx Canvas. @param page Hub page. */
inline void context_hub_draw(Arduino_GFX *gfx, UiPage page) {
	const auto hub = context_hub(page);
	uint8_t columns = uint8_t((ui_s(88) - 4) / (6 * ui_text_size(1)));
	for (uint8_t i = 0; i < hub.count; ++i) {
		int16_t x = ui_s(i % 2 ? 126 : 6), y = ui_s(58 + (i / 2) * 50);
		ui_box(gfx, x, y, ui_s(108), ui_s(44), UI_CARD, true);
		gfx->fillRoundRect(x + ui_s(6), y + ui_s(14), ui_s(3), ui_s(16), ui_s(1), UI_ACCENT);
		TextLines lines = wrap_text(tr(hub.tiles[i].label), columns, ui_s(88) - 4);
		int16_t top = y + ui_s(24) - int16_t(lines.count ? lines.count : 1) * ui_s(8);
		for (uint8_t l = 0; l < lines.count && l < 2; ++l) {
			ui_label(gfx, lines.line[l], x + ui_s(15), top + l * ui_s(16), ui_s(86), 1, UI_TEXT);
		}
	}
	ui_label(gfx, tr(page == UiPage::DriveHub ? Text::DriveBack2 : (page == UiPage::Games ? Text::BackDesk : Text::GamesHint)), ui_s(12), ui_s(238), ui_s(216), 1, UI_MUTED);
}
/** @brief Ignore card gaps, hidden entries and the footer hint. @param page Hub page. @param x Pixel X. @param y Pixel Y. @return Selected page or hub. */
inline UiPage context_hub_tap(UiPage page, int16_t x, int16_t y) {
	const auto hub = context_hub(page);
	for (uint8_t i = 0; i < hub.count; ++i) {
		int16_t left = ui_s(i % 2 ? 126 : 6), top = ui_s(58 + (i / 2) * 50);
		if (x >= left && x < left + ui_s(108) && y >= top && y < top + ui_s(44)) { return hub.tiles[i].page; }
	}
	return page;
}
