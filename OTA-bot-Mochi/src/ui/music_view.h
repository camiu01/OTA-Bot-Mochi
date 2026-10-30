/**
 * @file music_view.h
 * @brief Shared-palette media page with bounded rotating text and touch controls.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "ui_theme.h"
#include "ui_state.h"
#include "i18n.h"

/** @brief Render long metadata in readable overlapping windows. @param gfx Canvas. @param text Metadata. @param y Top. @param size Font size. @param now Time. */
inline void music_line(Arduino_GFX *gfx, const String &text, int16_t y, uint8_t size, uint32_t now) {
	String safe = ui_ascii(text);
	unsigned int columns = ui_s(184) / (6 * ui_text_size(size));
	unsigned int offset = 0;
	if (safe.length() > columns) { offset = ((now / 2000) % ((safe.length() - 1) / columns + 2)) * columns; }
	if (offset >= safe.length()) { offset = 0; }
	ui_label(gfx, safe.substring(offset, offset + columns), ui_s(28), ui_s(y), ui_s(184), size, UI_TEXT);
}

/** @brief Draw familiar transport symbols without relying on Unicode fonts. @param gfx Canvas. @param index Previous/toggle/next. @param playing Playback state. @param color Ink. */
inline void music_symbol(Arduino_GFX *gfx, uint8_t index, bool playing, uint16_t color) {
	int16_t cx = 46 + index * 74;
	if (index == 1 && playing) {
		gfx->fillRect(ui_s(cx - 8), ui_s(220), ui_s(5), ui_s(20), color);
		gfx->fillRect(ui_s(cx + 3), ui_s(220), ui_s(5), ui_s(20), color);
		return;
	}
	int16_t direction = index == 0 ? -1 : 1;
	gfx->fillTriangle(ui_s(cx - direction * 7), ui_s(220), ui_s(cx - direction * 7),
		ui_s(240), ui_s(cx + direction * 7), ui_s(230), color);
	if (index != 1) { gfx->fillRect(ui_s(cx + direction * 10 - 1), ui_s(220), ui_s(3), ui_s(20), color); }
}

/** @brief Draw phone-volume controls separately from Mochi's local sound level. @param gfx Canvas. @param music Snapshot. */
inline void music_volume(Arduino_GFX *gfx, const UiMusic &music) {
	for (uint8_t i = 0; i < 2; ++i) {
		int16_t x = i ? 160 : 12;
		MusicAction action = i ? MusicAction::VolumeUp : MusicAction::VolumeDown;
		uint16_t ink = music.commands & (1U << uint8_t(action)) ? UI_TEXT : UI_MUTED;
		gfx->fillRoundRect(ui_s(x), ui_s(162), ui_s(68), ui_s(38), ui_s(8), UI_CARD);
		gfx->fillRect(ui_s(x + 24), ui_s(180), ui_s(20), ui_s(3), ink);
		if (i) { gfx->fillRect(ui_s(x + 33), ui_s(172), ui_s(3), ui_s(20), ink); }
	}
	String label = music.volume <= 100 ? String("VOL ") + String(music.volume) + "%" : tr(Text::MusicVolume);
	ui_label(gfx, label, ui_s(86), ui_s(177), ui_s(68), 1, UI_MUTED);
}

/** @brief Render media content without modifying the user's eye configuration. @param gfx Canvas. @param data Snapshot. @param now Time. */
inline void music_draw(Arduino_GFX *gfx, const UiSnapshot &data, uint32_t now) {
	if (!data.connected) { ui_empty(gfx, tr(Text::Offline), tr(Text::MusicConnect)); return; }
	const UiMusic &music = data.music;
	if (!music.available) { ui_empty(gfx, tr(Text::MusicWaiting), tr(Text::MusicOpen)); return; }
	ui_card(gfx, 62, 92);
	ui_label(gfx, tr(music.playing ? Text::MusicPlaying : Text::Pause), ui_s(28), ui_s(76), ui_s(184), 1, UI_MUTED);
	if (music.title.length()) { music_line(gfx, music.title, 98, 2, now); }
	else { ui_label(gfx, tr(Text::MusicEmpty), ui_s(28), ui_s(98), ui_s(184), 1, UI_TEXT); }
	music_line(gfx, music.artist, 132, 1, now);
	music_volume(gfx, music);
	const MusicAction actions[] = {MusicAction::Previous, MusicAction::Toggle, MusicAction::Next};
	for (uint8_t i = 0; i < 3; ++i) {
		bool enabled = music.commands & (1U << uint8_t(actions[i]));
		gfx->fillRoundRect(ui_s(12 + i * 74), ui_s(206), ui_s(68), ui_s(48), ui_s(8),
			i == 1 && enabled ? UI_ACCENT : UI_CARD);
		music_symbol(gfx, i, music.playing, !enabled ? UI_MUTED : (i == 1 ? UI_BG : UI_TEXT));
	}
}

/** @brief Route only released taps inside media buttons. @param x Panel X. @param y Panel Y. */
inline void music_tap(int16_t x, int16_t y) {
	if (y >= ui_s(162) && y < ui_s(200)) {
		if (x >= ui_s(12) && x < ui_s(80)) { ui_music_request(MusicAction::VolumeDown); }
		if (x >= ui_s(160) && x < ui_s(228)) { ui_music_request(MusicAction::VolumeUp); }
		return;
	}
	if (y < ui_s(206) || y >= ui_s(254)) { return; }
	const MusicAction actions[] = {MusicAction::Previous, MusicAction::Toggle, MusicAction::Next};
	for (uint8_t i = 0; i < 3; ++i) {
		int16_t left = ui_s(12 + i * 74);
		if (x >= left && x < left + ui_s(68)) { ui_music_request(actions[i]); return; }
	}
}
