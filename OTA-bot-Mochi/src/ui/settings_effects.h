/** @file settings_effects.h @brief Lightweight visual feedback for the AMOLED settings menu. */
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "ui_theme.h"
#include "i18n.h"
#include "save_glow.h"
#include "ui_font.h"

namespace settings_effects {
#if MOCHI_DISPLAY_CO5300 && LCD_WIDTH == 368 && LCD_HEIGHT == 448
namespace detail {
constexpr int Width = 276, Height = 66, Radius = 16;
constexpr int X = (LCD_WIDTH - Width) / 2, Y = (LCD_HEIGHT - Height) / 2;
constexpr uint16_t Scrim = 104, PanelOpacity = 230;
constexpr uint16_t Charcoal = 0x31A6, White = 0xFFFF;

/** @brief Access a pixel in the current logical orientation. */
inline uint16_t &pixel(uint16_t *pixels, bool flipped, int x, int y) {
	const int pos = y * LCD_WIDTH + x;
	return pixels[flipped ? LCD_WIDTH * LCD_HEIGHT - 1 - pos : pos];
}

/** @brief Dim the menu before adding the toast and the separate perimeter light. */
inline void dim_menu(uint16_t *pixels, uint16_t alpha) {
	for (size_t i = 0; i < size_t(LCD_WIDTH) * LCD_HEIGHT; ++i) {
		if (pixels[i]) { pixels[i] = ui_mix(pixels[i], UI_BG, alpha); }
	}
}

/** @brief Paint a nearly opaque charcoal surface with a quiet, neutral rim. */
inline void panel(uint16_t *pixels, bool flipped, uint16_t alpha) {
	const uint16_t opacity = PanelOpacity * alpha / 256;
	const uint16_t border = ui_mix(Charcoal, White, 54);
	for (int y = 0; y < Height; ++y) {
		const int inset_y = y < Height / 2 ? y : Height - 1 - y;
		for (int x = 0; x < Width; ++x) {
			const int inset_x = x < Width / 2 ? x : Width - 1 - x;
			const int dx = inset_x < Radius ? Radius - inset_x : 0;
			const int dy = inset_y < Radius ? Radius - inset_y : 0;
			const int distance = dx * dx + dy * dy;
			if (distance > Radius * Radius) { continue; }
			const bool rim = distance > (Radius - 1) * (Radius - 1);
			auto &behind = pixel(pixels, flipped, X + x, Y + y);
			behind = ui_mix(behind, rim ? border : Charcoal, opacity);
		}
	}
}
}

/** @brief Show the centered confirmation above the dimmed settings canvas. */
inline void saved_toast(Arduino_GFX *gfx, uint16_t *pixels, bool flipped, uint32_t elapsed) {
	if (elapsed >= save_glow::Duration) { return; }
	const uint16_t alpha = elapsed < 100 ? uint16_t(elapsed * 256 / 100) : save_glow::fade(elapsed);
	if (!alpha) { return; }
	detail::dim_menu(pixels, detail::Scrim * alpha / 256);
	detail::panel(pixels, flipped, alpha);
	const char *text = tr(Text::ChangesSaved);
	const int16_t width = ui_font::width(ui_ascii(text), 1);
	const uint16_t background = detail::pixel(pixels, flipped, LCD_WIDTH / 2, LCD_HEIGHT / 2);
	ui_font::label(gfx, text, (LCD_WIDTH - width) / 2, LCD_HEIGHT / 2 - ui_font::height(1) / 2,
		width, 1, ui_mix(background, detail::White, alpha), background);
}
#else
inline void saved_toast(Arduino_GFX *, uint16_t *, bool, uint32_t) {}
#endif
}
