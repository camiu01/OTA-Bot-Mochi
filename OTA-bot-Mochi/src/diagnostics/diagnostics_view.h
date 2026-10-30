/**
 * @file diagnostics_view.h
 * @brief Read-only, paged device diagnostics using existing scaled settings geometry.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "device_status.h"
#include "../ui/ui_theme.h"
#include "../ui/i18n.h"
#include "../ui/ui_state.h"
#include "../settings/settings.h"
#include "../eyes/eyes_api.h"
#include "../sound/sound.h"
#include "../../pin_config.h"

namespace diagnostics_view {
inline constexpr uint8_t ROWS = 11;

/** @brief Describe the last worker observation, never implying a card probe when disabled. @return Localized state. */
inline const char *sd_label() {
#if MOCHI_BOARD_LCD_169
	return tr(Text::DiagUnsupported);
#else
	if (!settings_get().sound_sd) { return tr(Text::Off); }
	switch (device_status::sd.load()) {
		case device_status::SdState::Ready: return tr(Text::DiagReady);
		case device_status::SdState::Missing: return tr(Text::DiagMissing);
		case device_status::SdState::Error: return tr(Text::Failed);
		default: return tr(Text::DiagUnchecked);
	}
#endif
}

/** @brief Format one live value without including phone content or network credentials. @param index Row. @param state Platform snapshot. @param connected BLE link. @return Bounded value. */
inline String value(uint8_t index, const device_status::Snapshot &state, bool connected) {
	switch (index) {
		case 0: return MOCHI_BOARD_LCD_169 ? "LCD 1.69 OLD" : "AMOLED 1.8 V2";
		case 1: return tr(connected ? Text::Connected : Text::Offline);
		case 2: return state.measured ? tr(state.wifi ? Text::Connected : Text::Offline) : "--";
		case 3: return tr(eyes_imu_ready() ? Text::ImuReady : Text::ImuMissing);
		case 4: return sd_label();
		case 5: {
			static const Text states[] = {Text::Off, Text::NightMute, Text::DiagArmed, Text::DiagReady, Text::Failed};
			return tr(states[uint8_t(sound_status())]);
		}
		case 6: return state.measured ? String(state.heap / 1024) + " KiB" : "--";
		case 7: return state.measured ? String(state.minimum_heap / 1024) + " KiB" : "--";
		case 8: return state.measured ? String(state.psram / 1024) + " KiB" : "--";
		case 9: return String(state.uptime / 3600000) + "h " + String(state.uptime / 60000 % 60) + "m";
		case 10: return state.reset;
		default: return "--";
	}
}

/** @brief Draw three read-only rows and a full-width return target. @param gfx Canvas. @param first First row. */
inline void draw(Arduino_GFX *gfx, uint8_t first) {
	static const Text labels[] = {Text::DiagBoard, Text::DiagPhone, Text::DiagWifi,
		Text::DiagImu, Text::DiagSd, Text::Sound, Text::DiagHeap, Text::DiagMinimum,
		Text::DiagPsram, Text::DiagUptime, Text::DiagReset};
	const auto state = device_status::snapshot();
	const bool connected = ui_snapshot().connected;
	gfx->fillRoundRect(ui_s(2), ui_s(52), ui_s(236), ui_s(36), ui_s(8), UI_CARD);
	ui_label(gfx, tr(Text::Diagnostics), ui_s(14), ui_s(60), ui_s(212), 1, UI_ACCENT);
	ui_label(gfx, String(first + 1) + "-" + String(first + 3) + "/" + String(ROWS), ui_s(14), ui_s(75), ui_s(212), 1, UI_MUTED);
	for (uint8_t slot = 0; slot < 3 && first + slot < ROWS; ++slot) {
		uint8_t index = first + slot;
		int16_t y = 90 + slot * 44;
		ui_card(gfx, y, 40);
		ui_label(gfx, tr(labels[index]), ui_s(24), ui_s(y + 5), ui_s(192), 1, UI_MUTED);
		ui_label(gfx, value(index, state, connected), ui_s(24), ui_s(y + 23), ui_s(192), 1, UI_TEXT);
	}
	gfx->fillRect(ui_s(232), ui_s(92), ui_s(3), ui_s(128), UI_CARD);
	gfx->fillRect(ui_s(232), ui_s(92 + first * 88 / (ROWS - 3)), ui_s(3), ui_s(40), UI_ACCENT);
	gfx->fillRoundRect(ui_s(2), ui_s(224), ui_s(236), ui_s(54), ui_s(8), UI_CARD);
	ui_label(gfx, tr(Text::Back), ui_s(24), ui_s(247), ui_s(192), 1, UI_TEXT);
}
}
