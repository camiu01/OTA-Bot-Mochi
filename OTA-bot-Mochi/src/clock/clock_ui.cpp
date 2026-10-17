/**
 * @file clock_ui.cpp
 * @brief Quiet clock face with independent time, date, and status regions.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "clock_ui.h"
#include "../ui/ui_theme.h"
#include "../ui/i18n.h"
#include "../../pin_config.h"
#include <atomic>

namespace {
std::atomic<bool> synced{false};
bool invalidated = true;
bool previous_ready = false;
String previous[4];

/**
 * @brief Replace only a changed field on the clock card.
 * @param gfx Canvas.
 * @param text New text.
 * @param index Cached field index.
 * @param y Top edge in design space.
 * @param size Design font scale.
 * @param color Foreground.
 * @return True if painted.
 */
bool field(Arduino_GFX *gfx, const String &text, uint8_t index, int16_t y,
	uint8_t size, uint16_t color) {
	if (!invalidated && previous[index] == text) { return false; }
	uint8_t eff = ui_text_size(size);
	int16_t width = int16_t(text.length()) * 6 * eff;
	gfx->fillRect(ui_s(24), ui_s(y), LCD_WIDTH - ui_s(48), ui_s(int16_t(8 * eff + 4)), UI_CARD);
	ui_label(gfx, text, (LCD_WIDTH - width) / 2, ui_s(y), width, size, color);
	previous[index] = text;
	return true;
}
}

/** @brief Mark time valid only after Chronos has set its clock. */
void clock_time_synced() { synced.store(true); }
/** @brief Expose synchronization without inventing an offline clock. @return Ready. */
bool clock_ready() { return synced.load(); }

/** @brief Force repaint after entering the clock screen. */
void clock_invalidate() { invalidated = true; }

/**
 * @brief Draw synchronized time while retaining it across disconnections.
 * @param gfx Canvas.
 * @param watch Chronos time source.
 * @param connected Bluetooth flag.
 * @return Whether the canvas changed.
 */
bool clock_draw(Arduino_GFX *gfx, ChronosESP32 &watch, bool connected) {
	bool ready = synced.load();
	if (ready != previous_ready) { invalidated = true; }
	previous_ready = ready;
	bool changed = invalidated;
	if (invalidated) {
		ui_card(gfx, 62, 152);
		ui_label(gfx, tr(ready ? Text::LocalTime : Text::PhoneSync), ui_s(28), ui_s(78), ui_s(180), 1, UI_ACCENT);
	}
	String time = ready ? watch.getHourZ() + watch.getTime(":%M") : "--:--";
	String seconds = ready ? watch.getTime(":%S") + " " + watch.getAmPmC() : tr(Text::Waiting);
	String date = ready ? watch.getTime("%d / %m / %Y") : tr(Text::OpenChronos);
	String status = tr(connected ? Text::Connected : Text::Offline);
	changed |= field(gfx, time, 0, 108, 4, UI_TEXT);
	changed |= field(gfx, seconds, 1, 150, 1, UI_ACCENT);
	changed |= field(gfx, date, 2, 172, 1, UI_TEXT);
	changed |= field(gfx, status, 3, 196, 1, UI_MUTED);
	invalidated = false;
	return changed;
}
