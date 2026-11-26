/**
 * @file context_drive.h
 * @brief Car/moto dashboard: clock-style lean home plus dedicated tools under
 * DRIVE TOOLS (big lean, G-meter, lean graph, trip, BLE ELM327 OBD), with ZERO.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "screens.h"
#include "ui_state.h"
#include "ui_theme.h"
#include "ui_font.h"
#include "i18n.h"
#include "../play/drive_meter.h"
#include "../vehicle/obd_state.h"
#include "../settings/settings.h"
#include "../eyes/eyes_api.h"
#include "../clock/clock_ui.h"
#include "../../pin_config.h"
#include <ChronosESP32.h>
#include <stdio.h>

/** @brief Centered label. @param gfx Canvas. @param text Text. @param y Top (design). @param size Font scale. @param color Ink. */
inline void drive_center(Arduino_GFX *gfx, const String &text, int16_t y, uint8_t size, uint16_t color) {
	ui_center(gfx, text, y, size, color);
}
/** @brief Format a signed lean as L/R degrees. @param out Buffer. @param size Capacity. @param lean Degrees, +right. */
inline void drive_lean_text(char *out, size_t size, float lean) {
	int value = int(lroundf(lean));
	snprintf(out, size, "%c%d", value < 0 ? 'L' : (value > 0 ? 'R' : ' '), value < 0 ? -value : value);
}
/** @brief Format pitch as FWD/BACK degrees. @param out Buffer. @param size Capacity. @param pitch Degrees, negative when the front is lower. */
inline void drive_pitch_text(char *out, size_t size, float pitch) {
	int value = int(lroundf(pitch));
	snprintf(out, size, "%s %d", value ? tr(value < 0 ? Text::DriveForward : Text::DriveBack) : "", value < 0 ? -value : value);
}
/** @brief Total horizontal acceleration. @param m Meter. @return g. */
inline float drive_g(const DriveMeter &m) { return sqrtf(m.lateral * m.lateral + m.longitudinal * m.longitudinal); }
/** @brief Waiting/missing-sensor message instead of motion values. @param gfx Canvas. @param m Meter. @return True when values are usable. */
inline bool drive_ready(Arduino_GFX *gfx, const DriveMeter &m) {
	if (eyes_imu_ready() && m.tared) { return true; }
	drive_center(gfx, tr(eyes_imu_ready() ? Text::DriveHint : Text::ImuMissing), 130, 1, UI_MUTED);
	return false;
}

/** @brief Clock-style primary page: lean replaces the time; swipe down for tools. @param gfx Canvas. @param now Time. */
inline void drive_home_draw(Arduino_GFX *gfx, uint32_t now) {
	const DriveMeter &m = drive_meter();
	char text[32], pitch[20];
	ui_card(gfx, 62, 152);
	bool ready = eyes_imu_ready() && m.tared;
	drive_center(gfx, tr(ready ? Text::DriveLean : (eyes_imu_ready() ? Text::DriveHint : Text::ImuMissing)), 76, 1, UI_MUTED);
	if (ready) { drive_lean_text(text, sizeof(text), m.roll); } else { snprintf(text, sizeof(text), "--"); }
	drive_center(gfx, text, 100, 5, UI_TEXT);
	if (ready) {
		drive_pitch_text(pitch, sizeof(pitch), m.pitch);
		snprintf(text, sizeof(text), "%.2fG  %s", double(drive_g(m)), pitch);
		drive_center(gfx, text, 150, 1, drive_g(m) > 0.5f ? UI_WARM : UI_ACCENT);
		// Lean gauge: the marker slides left/right up to 60 degrees.
		int16_t left = ui_s(40), width = LCD_WIDTH - 2 * ui_s(40), mid = left + width / 2;
		gfx->fillRoundRect(left, ui_s(168), width, ui_s(4), ui_s(2), UI_LINE);
		gfx->drawFastVLine(mid, ui_s(165), ui_s(10), UI_MUTED);
		float lean = m.roll > 60 ? 60 : (m.roll < -60 ? -60 : m.roll);
		gfx->fillCircle(mid + int16_t(lean * (width / 2) / 60), ui_s(170), ui_s(5), UI_ACCENT);
		snprintf(text, sizeof(text), "MAX L%d R%d", int(m.max_left), int(m.max_right));
		drive_center(gfx, text, 180, 1, UI_MUTED);
	}
	const VehicleData obd = obd_snapshot();
	if (obd.status == ObdStatus::Live && (obd.valid & OBD_SPEED) && uint32_t(now - obd.updated) <= 3000) {
		snprintf(text, sizeof(text), "%d KM/H  %d RPM", int(obd.speed), int(obd.rpm));
		drive_center(gfx, text, 198, 1, UI_TEXT);
	}
	ui_label(gfx, tr(Text::DriveMore), ui_s(12), ui_s(238), ui_s(216), 1, UI_MUTED);
}

/** @brief Huge lean with pitch and maxima, like Big Clock. @param gfx Canvas. @param m Meter. */
inline void drive_big_lean(Arduino_GFX *gfx, const DriveMeter &m) {
	ui_card(gfx, 62, 152);
	if (!drive_ready(gfx, m)) { return; }
	char text[24];
	drive_center(gfx, tr(Text::DriveLean), 70, 1, UI_MUTED);
	drive_lean_text(text, sizeof(text), m.roll);
	drive_center(gfx, text, 94, 7, UI_TEXT);
	drive_pitch_text(text, sizeof(text), m.pitch);
	drive_center(gfx, text, 160, 2, UI_ACCENT);
	snprintf(text, sizeof(text), "MAX L%d R%d", int(m.max_left), int(m.max_right));
	drive_center(gfx, text, 190, 1, UI_MUTED);
}
/** @brief Large friction circle with 0.5 g / 1 g rings and peaks. @param gfx Canvas. @param m Meter. */
inline void drive_g_meter(Arduino_GFX *gfx, const DriveMeter &m) {
	ui_card(gfx, 58, 164);
	if (!drive_ready(gfx, m)) { return; }
	int16_t cx = LCD_WIDTH / 2, cy = ui_s(126), r = ui_s(56);
	gfx->drawCircle(cx, cy, r, UI_MUTED);
	gfx->drawCircle(cx, cy, r / 2, UI_LINE);
	gfx->drawFastHLine(cx - r, cy, 2 * r, UI_LINE);
	gfx->drawFastVLine(cx, cy - r, 2 * r, UI_LINE);
	float x = m.lateral, y = m.longitudinal, g = drive_g(m);
	if (g > 1.1f) { x *= 1.1f / g; y *= 1.1f / g; }
	gfx->fillCircle(cx + int16_t(x * r), cy - int16_t(y * r), ui_s(7), g > 0.5f ? UI_WARM : UI_ACCENT);
	char text[32];
	snprintf(text, sizeof(text), "%.2fG", double(g));
	drive_center(gfx, text, 188, 2, UI_TEXT);
	snprintf(text, sizeof(text), "+%.2f -%.2f MAX %.1fG", double(m.max_accel), double(m.max_brake), double(m.peak_g));
	drive_center(gfx, text, 208, 1, UI_MUTED);
}
/** @brief Full-card scrolling lean (accent, +/-60 deg) and lateral G (warm, +/-1.2 g). @param gfx Canvas. @param m Meter. */
inline void drive_graph(Arduino_GFX *gfx, const DriveMeter &m) {
	ui_card(gfx, 58, 164);
	if (!drive_ready(gfx, m)) { return; }
	constexpr int32_t scale = 60;	// Both traces reach the band edge at +/-60 stored units.
	int16_t left = ui_s(18), width = ui_s(204), mid = ui_s(140), half = ui_s(58);
	ui_label(gfx, tr(Text::DriveLegend), left, ui_s(64), width, 1, UI_MUTED);
	gfx->drawFastHLine(left, mid, width, UI_LINE);
	for (bool g : {false, true}) {
		for (uint8_t i = 1; i < m.filled; ++i) {
			int32_t a = m.trace(uint8_t(i - 1), g), b = m.trace(i, g);
			a = a > scale ? scale : (a < -scale ? -scale : a);
			b = b > scale ? scale : (b < -scale ? -scale : b);
			int16_t x0 = left + int16_t(int32_t(i - 1) * width / (DriveMeter::HISTORY - 1));
			int16_t x1 = left + int16_t(int32_t(i) * width / (DriveMeter::HISTORY - 1));
			gfx->drawLine(x0, mid - int16_t(a * half / scale), x1, mid - int16_t(b * half / scale), g ? UI_WARM : UI_ACCENT);
		}
	}
	char lean[12], text[32];
	drive_lean_text(lean, sizeof(lean), m.roll);
	snprintf(text, sizeof(text), "%s  %.2fG", lean, double(m.lateral));
	drive_center(gfx, text, 206, 1, UI_TEXT);
}
/** @brief Clock, trip time, session peaks and the phone's next turn. @param gfx Canvas. @param watch Clock. @param data Snapshot. @param m Meter. @param now Time. */
inline void drive_trip(Arduino_GFX *gfx, ChronosESP32 &watch, const UiSnapshot &data, const DriveMeter &m, uint32_t now) {
	ui_card(gfx, 58, 164);
	char text[32];
	ui_label(gfx, clock_ready() ? watch.getHourZ() + watch.getTime(":%M") : String("--:--"), ui_s(24), ui_s(68), ui_s(124), 4, UI_TEXT);
	uint32_t trip = m.trip_seconds(now);
	snprintf(text, sizeof(text), "%lu:%02lu:%02lu", (unsigned long)(trip / 3600), (unsigned long)(trip / 60 % 60), (unsigned long)(trip % 60));
	ui_label(gfx, tr(Text::DriveTrip), ui_s(152), ui_s(68), ui_s(70), 1, UI_MUTED);
	ui_label(gfx, text, ui_s(152), ui_s(84), ui_s(76), 1, UI_ACCENT);
	snprintf(text, sizeof(text), "%s L%d R%d", tr(Text::DriveLean), int(m.max_left), int(m.max_right));
	ui_label(gfx, text, ui_s(24), ui_s(112), ui_s(192), 2, UI_TEXT);
	snprintf(text, sizeof(text), "+%.2fG -%.2fG %.1fG", double(m.max_accel), double(m.max_brake), double(m.peak_g));
	ui_label(gfx, text, ui_s(24), ui_s(136), ui_s(192), 1, UI_MUTED);
	const Navigation &nav = data.navigation;
	if (!nav.active) { ui_label(gfx, tr(Text::NoRoute), ui_s(24), ui_s(176), ui_s(192), 1, UI_MUTED); return; }
	ui_label(gfx, nav.distance.length() ? nav.distance : String("--"), ui_s(24), ui_s(166), ui_s(192), 2, UI_ACCENT);
	ui_label(gfx, nav.directions.length() ? nav.directions : nav.title, ui_s(24), ui_s(192), ui_s(192), 1, UI_TEXT);
}
/** @brief Format a field or "--" when the dongle has not reported it. */
inline String drive_field(const VehicleData &d, uint32_t bit, float value, const char *unit) {
	return d.valid & bit ? String(int(lroundf(value))) + unit : String("--");
}
static_assert(OBD_SLOT_CHOICES == OBD_VALUE_COUNT, "OBD slot choices follow ObdValue");
/** @brief One configurable OBD value with its short label. @param d Snapshot. @param v ObdValue (1-16). @return Text. */
inline String drive_slot_text(const VehicleData &d, uint8_t v) {
	static const char *labels[OBD_VALUE_COUNT][2] = {
		{"", ""}, {"H2O ", "ACQUA "}, {"GAS ", "GAS "}, {"FUEL ", "BENZ. "}, {"BAT ", "BAT "}, {"AIR ", "ARIA "},
		{"LOAD ", "CARICO "}, {"MAP ", "MAP "}, {"ADV ", "ANTIC. "}, {"STFT ", "STFT "}, {"LTFT ", "LTFT "},
		{"O2 ", "O2 "}, {"ECU ", "ECU "}, {"MAF ", "MAF "}, {"OUT ", "EST. "}, {"BARO ", "BARO "}, {"ON ", "ACCESO "}
	};
	static const char *units[OBD_VALUE_COUNT] = {"", "C", "%", "%", "V", "C", "%", "KPA", "", "%", "%", "V", "V", "G/S", "C", "KPA", "MIN"};
	static const uint8_t decimals[OBD_VALUE_COUNT] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 2, 1, 1, 0, 0, 0};
	String text = labels[v][settings_get().language == 1 ? 1 : 0];
	if (!(d.valid & obd_bit(v))) { return text + "--"; }
	char number[16];
	snprintf(number, sizeof(number), "%.*f%s", int(decimals[v]), double(d.value[v]), units[v]);
	return text + number;
}
/**
 * @brief Tachometer in the selected style: bar, shift-light segments, dot dial beside the speed, or none.
 * @param gfx Canvas. @param style Settings obd_gauge. @param permille RPM share of the configured maximum. @param now Time.
 */
inline void drive_tacho(Arduino_GFX *gfx, uint8_t style, int32_t permille, uint32_t now) {
	permille = permille < 0 ? 0 : (permille > 1000 ? 1000 : permille);
	uint16_t hot = permille > 850 ? UI_WARM : UI_ACCENT;
	if (style == 0) { ui_bar(gfx, ui_s(24), ui_s(170), ui_s(192), ui_s(8), permille, hot); return; }
	if (style == 1) {	// Shift light: ten blocks, the top two warm; all blink past 95 %.
		bool blink = permille > 950 && (now / 125) % 2;
		for (int16_t i = 0; i < 10; ++i) {
			bool lit = permille >= (i + 1) * 100 - 50 && !blink;
			gfx->fillRoundRect(ui_s(24 + i * 20), ui_s(168), ui_s(16), ui_s(12), ui_s(3), lit ? (i >= 8 ? UI_WARM : UI_ACCENT) : UI_LINE);
		}
		return;
	}
	if (style != 2) { return; }
	// Dial: 21 dots on a 270 degree sweep, centered right of the speed digits.
	int16_t cx = ui_s(184), cy = ui_s(112), r = ui_s(26), dot = ui_s(2) < 2 ? 2 : ui_s(2);
	for (int16_t i = 0; i <= 20; ++i) {
		float a = (135.0f + i * 13.5f) * 3.14159265f / 180.0f;
		bool lit = permille >= i * 50;
		gfx->fillCircle(int16_t(cx + r * cosf(a)), int16_t(cy + r * sinf(a)), dot, lit ? (i >= 17 ? UI_WARM : UI_ACCENT) : UI_LINE);
	}
	ui_label(gfx, "KM/H", cx - ui_s(12), cy + ui_s(16), ui_s(30), 1, UI_MUTED);
}
/** @brief Engine data from a BLE ELM327 dongle, with link status. @param gfx Canvas. @param now Time. */
inline void drive_obd(Arduino_GFX *gfx, uint32_t now) {
	ui_card(gfx, 58, 196);
	const VehicleData d = obd_snapshot();
	const CompanionSettings &p = settings_get();
	if (!p.obd_scan) { drive_center(gfx, tr(Text::ObdOff), 130, 1, UI_MUTED); return; }
	static const Text status[] = {Text::ObdIdle, Text::ObdScanning, Text::ObdConnecting, Text::ObdStarting, Text::ObdLive, Text::ObdNoEcu};
	bool stale = d.status == ObdStatus::Live && uint32_t(now - d.updated) > 3000;
	String head = d.status == ObdStatus::Live && d.name[0] ? String(d.name) : String(tr(status[uint8_t(d.status)]));
	ui_label(gfx, head, ui_s(24), ui_s(68), ui_s(192), 1, stale ? UI_WARM : UI_MUTED);
	uint16_t ink = d.status == ObdStatus::Live && !stale ? UI_TEXT : UI_MUTED;
	ui_label(gfx, drive_field(d, OBD_SPEED, d.speed, ""), ui_s(24), ui_s(88), ui_s(120), 6, ink);
	if (p.obd_gauge != 2) { ui_label(gfx, "KM/H", ui_s(150), ui_s(118), ui_s(60), 1, UI_MUTED); }
	ui_label(gfx, drive_field(d, OBD_RPM, d.rpm, " RPM"), ui_s(24), ui_s(148), ui_s(192), 2, UI_ACCENT);
	drive_tacho(gfx, p.obd_gauge, d.valid & OBD_RPM ? int32_t(d.rpm * 1000 / (int32_t(p.obd_rpm_max) * 100)) : 0, now);
	gfx->drawFastHLine(ui_s(24), ui_s(186), ui_s(192), UI_LINE);
	const uint8_t slots[OBD_SLOTS] = {p.obd_slot1, p.obd_slot2, p.obd_slot3, p.obd_slot4};
	for (uint8_t i = 0; i < OBD_SLOTS; ++i) {
		if (!slots[i] || slots[i] >= OBD_SLOT_CHOICES) { continue; }
		ui_label(gfx, drive_slot_text(d, slots[i]), ui_s(i % 2 ? 124 : 24), ui_s(i < 2 ? 194 : 220), ui_s(96), 1, ink);
	}
	if (!(d.valid & OBD_MIL)) { return; }
	ui_label(gfx, d.mil || d.dtc ? tr(Text::ObdMil) + String(d.dtc) + " DTC" : String(tr(Text::ObdNoFaults)),
		ui_s(24), ui_s(240), ui_s(192), 1, d.mil ? UI_WARM : UI_MUTED);
}
/** @brief Motion tools carry a ZERO button; OBD does not. @param page Tool. @return Button shown. */
inline bool drive_has_zero(UiPage page) {
	return page == UiPage::DriveLean || page == UiPage::DriveG || page == UiPage::DriveGraph || page == UiPage::DriveTrip;
}
/** @brief Render one DRIVE TOOLS page. @param gfx Canvas. @param watch Clock. @param data Snapshot. @param page Tool. @param now Time. */
inline void drive_tool_draw(Arduino_GFX *gfx, ChronosESP32 &watch, const UiSnapshot &data, UiPage page, uint32_t now) {
	const DriveMeter &m = drive_meter();
	if (page == UiPage::DriveLean) { drive_big_lean(gfx, m); }
	else if (page == UiPage::DriveG) { drive_g_meter(gfx, m); }
	else if (page == UiPage::DriveGraph) { drive_graph(gfx, m); }
	else if (page == UiPage::DriveTrip) { drive_trip(gfx, watch, data, m, now); }
	else if (page == UiPage::DriveObd) { drive_obd(gfx, now); }
	if (!drive_has_zero(page)) { return; }
	ui_button(gfx, ui_s(6), ui_s(226), ui_s(228), ui_s(30), tr(Text::DriveZero), true);
}
/** @brief ZERO is the only control; gauges are read-only. @param page Tool. @param x Pixel X. @param y Pixel Y. @return Meter reset. */
inline bool drive_tap(UiPage page, int16_t x, int16_t y) {
	if (!drive_has_zero(page) || y < ui_s(226) || y >= ui_s(256) || x < ui_s(6) || x >= ui_s(234)) { return false; }
	drive_meter().zero();
	return true;
}
