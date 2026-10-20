/**
 * @file context_play.h
 * @brief Session-only Pong controls and synchronized analog desk clock.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../play/mini_widgets.h"
#include "../clock/clock_ui.h"
#include "ui_theme.h"
#include "i18n.h"
#include <string.h>

/** @brief Access the desk session, independent of the eye demo. @return Game. */
inline PongGame &desk_pong() { static PongGame game; return game; }

/** @brief Parse exactly HH:MM:SS, refusing invalid clock fields. @param text Input. @param seconds Output. @return Valid time. */
inline bool desk_clock_seconds(const char *text, uint32_t &seconds) {
	if (!text || strlen(text) != 8 || text[2] != ':' || text[5] != ':') { return false; }
	for (uint8_t i = 0; i < 8; ++i) {
		if (i == 2 || i == 5) { continue; }
		if (text[i] < '0' || text[i] > '9') { return false; }
	}
	uint8_t hour = uint8_t((text[0] - '0') * 10 + text[1] - '0');
	uint8_t minute = uint8_t((text[3] - '0') * 10 + text[4] - '0');
	uint8_t second = uint8_t((text[6] - '0') * 10 + text[7] - '0');
	if (hour > 23 || minute > 59 || second > 59) { return false; }
	seconds = uint32_t(hour) * 3600 + uint32_t(minute) * 60 + second;
	return true;
}

/** @brief Draw local playable Pong with bounded, tap-controlled left paddle. @param gfx Canvas. @param now Time. */
inline void desk_pong_draw(Arduino_GFX *gfx, uint32_t now) {
	PongGame &game = desk_pong();
	game.tick(now, false);
	ui_label(gfx, String(game.player) + " : " + String(game.opponent),
		ui_s(90), ui_s(56), ui_s(90), 2, UI_ACCENT);
	mini_pong_draw(gfx, game, ui_s(12), ui_s(82), ui_s(216), ui_s(128), UI_TEXT);
	ui_label(gfx, tr(Text::PongHint), ui_s(16), ui_s(216), ui_s(212), 1, UI_MUTED);
	gfx->fillRoundRect(ui_s(6), ui_s(236), ui_s(228), ui_s(22), ui_s(6), UI_CARD);
	ui_label(gfx, tr(Text::Reset), ui_s(18), ui_s(242), ui_s(204), 1, UI_TEXT);
}

/** @brief Draw actual synchronized time, never a fabricated offline time. @param gfx Canvas. @param watch Chronos clock. */
inline void desk_analog_draw(Arduino_GFX *gfx, ChronosESP32 &watch) {
	uint32_t seconds = 0;
	if (!clock_ready() || !desk_clock_seconds(watch.getTime("%H:%M:%S").c_str(), seconds)) {
		ui_empty(gfx, tr(Text::TimeUnavailable), tr(Text::OpenSync));
		return;
	}
	mini_clock_draw(gfx, ui_s(120), ui_s(146), ui_s(72), seconds, UI_TEXT, UI_ACCENT);
	ui_label(gfx, watch.getHourZ() + watch.getTime(":%M") + watch.getTime(":%S"),
		ui_s(48), ui_s(232), ui_s(156), 2, UI_TEXT);
}
