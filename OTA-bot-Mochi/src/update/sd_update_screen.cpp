/**
 * @file sd_update_screen.cpp
 * @brief Existing settings primitives for a manually confirmed SD firmware install.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "sd_update_screen.h"
#include "sd_update.h"
#include "../../pin_config.h"
#include "../ui/ui_theme.h"
#include "../ui/i18n.h"
#include "../touch/touch.h"
#include "../power/backlight.h"

namespace {
Arduino_GFX *canvas = nullptr;
uint32_t drawn = 0;
SdUpdatePhase previous = SdUpdatePhase::Idle;
/** @brief Draw bounded text in the shared design space. @param text Label. @param y Design row. @param color Ink. */
void line(const String &text, int16_t y, uint16_t color) {
	ui_label(canvas, text, ui_s(12), ui_s(y), ui_s(216), 1, color);
}
/** @brief Draw progress without exposing network settings or enabling automatic installation. @param now Milliseconds. */
void draw(uint32_t now) {
	const auto &s = sd_update_status();
	if (!canvas || !s.active || (s.phase == previous && uint32_t(now - drawn) < 200)) { return; }
	previous = s.phase; drawn = now;
	backlight_update(now, false, true);
	canvas->fillScreen(UI_BG);
	line(tr(Text::SdFirmware), 12, UI_TEXT);
	line("/mochi/update/", 48, UI_MUTED);
	line(MOCHI_OTA_FILENAME, 66, UI_TEXT);
	Text label = Text::SdChecking;
	if (s.phase == SdUpdatePhase::Ready) { label = Text::SdConfirm; }
	if (s.phase == SdUpdatePhase::Writing) { label = Text::OtaWriting; }
	if (s.phase == SdUpdatePhase::Success) { label = Text::OtaRestart; }
	if (s.phase == SdUpdatePhase::Failed) { label = Text::Failed; }
	line(tr(label), 102, UI_TEXT);
	size_t progress = s.phase == SdUpdatePhase::Writing ? s.written : s.checked;
	line(String(s.total ? progress * 100 / s.total : 0) + "%", 124, UI_ACCENT);
	if (s.error) {
		String detail(s.error);
		uint16_t columns = ui_s(216) / (6 * ui_text_size(1));
		for (uint8_t row = 0; row < 4; ++row) {
			line(detail.substring(row * columns, (row + 1) * columns), 150 + row * 14, UI_MUTED);
		}
	} else { line(tr(Text::SdKeepPower), 166, UI_MUTED); }
	if (s.phase != SdUpdatePhase::Success) {
		canvas->fillRoundRect(ui_s(2), ui_s(232), ui_s(116), ui_s(42), ui_s(8), UI_CARD);
		line(tr(Text::Back), 247, UI_TEXT);
	}
	if (s.phase == SdUpdatePhase::Ready) {
		canvas->fillRoundRect(ui_s(122), ui_s(232), ui_s(116), ui_s(42), ui_s(8), UI_ACCENT);
		ui_label(canvas, tr(Text::Start), ui_s(134), ui_s(247), ui_s(92), 1, UI_BG);
	}
	canvas->flush();
}
}

void sd_update_screen_begin(Arduino_GFX *gfx, uint32_t now) {
	canvas = gfx; drawn = now - 200; previous = SdUpdatePhase::Idle;
	backlight_activity(now);
	sd_update_begin(now);
	draw(now);
}
void sd_update_screen_poll(uint32_t now) {
	touch_poll(now);
	int16_t x = 0, y = 0;
	if (touch_take_tap(x, y) && x >= 0 && x < LCD_WIDTH && y >= ui_s(232) && y < ui_s(274)) {
		if (x < ui_s(120)) { sd_update_stop(); }
		else if (sd_update_confirm()) { drawn = now - 200; }
	}
	sd_update_poll(now);
	draw(now);
}
