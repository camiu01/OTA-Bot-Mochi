/**
 * @file ota_screen.cpp
 * @brief Scaled OTA instructions and write progress without persisting credentials.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ota_screen.h"
#include "ota_update.h"
#include "ota_qr.h"
#include "../ui/ui_theme.h"
#include "../ui/i18n.h"
#include "../touch/touch.h"
#include "../power/backlight.h"
#include "../sound/buzzer_output.h"

namespace {
Arduino_GFX *canvas = nullptr;
uint32_t drawn = 0;
bool qr_ready = false, qr_visible = false;
char cached_url[80] = {};

/** @brief Draw one bounded text line in the existing settings design space. @param text Label. @param y Design Y. @param color Ink. */
void line(const String &text, int16_t y, uint16_t color) {
	ui_label(canvas, text, ui_s(12), ui_s(y), ui_s(216), 1, color);
}

/** @brief Draw connection instructions without scaling or tinting QR modules. @param s Session. */
void connection(const OtaStatus &s) {
	line(tr(s.lan ? Text::WebScan : Text::OtaScan), 32, UI_MUTED);
	int16_t side = ui_s(124);
	ota_qr_draw(canvas, (canvas->width() - side) / 2, ui_s(52), side);
	line(s.ssid, 183, UI_TEXT);
	line(s.lan ? tr(Text::WebSameWifi) : String(tr(Text::OtaPassword)) + " " + s.password, 201, UI_TEXT);
	line(String("http://") + s.address, 215, UI_TEXT);
}

/** @brief Draw manual fallback, progress, or recovery detail. @param s Session. */
void details(const OtaStatus &s) {
	line(tr(Text::OtaWifi), 44, UI_MUTED);
	line(s.ssid, 62, UI_TEXT);
	line(tr(s.lan ? Text::WebSameWifi : Text::OtaPassword), 88, UI_MUTED);
	line(s.lan ? tr(Text::WebAuthorize) : s.password, 106, UI_TEXT);
	line(tr(Text::OtaBrowser), 132, UI_MUTED);
	line(String("http://") + s.address, 150, UI_TEXT);
	line(tr(s.error ? Text::Failed : (s.success ? Text::OtaRestart :
		(s.receiving ? Text::OtaWriting : (s.faces ? Text::FacesReady : Text::OtaReady)))), 172, UI_TEXT);
	if (!s.error) {
		if (!s.faces) { line(String(s.total ? s.received * 100 / s.total : 0) + "%", 198, UI_ACCENT); }
		return;
	}
	// Technical recovery detail is shared with the browser's English diagnostics.
	String detail(s.error);
	uint16_t columns = ui_s(216) / (6 * ui_text_size(1));
	for (uint8_t row = 0; row < 3; ++row) {
		line(detail.substring(row * columns, (row + 1) * columns), 190 + row * 14, UI_MUTED);
	}
}

/** @brief Refresh from the same task, including during synchronous HTTP upload. */
void draw() {
	uint32_t now = millis();
	if (!canvas || uint32_t(now - drawn) < 200) { return; }
	drawn = now;
	const OtaStatus &s = ota_status();
	if (!s.active) { return; }
	backlight_update(now, false, true);
	canvas->fillScreen(UI_BG);
	line(tr(s.faces ? (s.lan ? Text::WebSettings : Text::FacesWifi) : Text::FirmwareUpdate), 12, UI_TEXT);
	bool idle = !s.receiving && !s.success && !s.error;
	if (idle && qr_visible) { connection(s); } else { details(s); }
	if (!s.receiving && !s.success) {
		canvas->fillRoundRect(ui_s(2), ui_s(232), ui_s(116), ui_s(42), ui_s(8), UI_CARD);
		line(tr(Text::Back), 247, UI_TEXT);
		if (idle && qr_ready) {
			canvas->fillRoundRect(ui_s(122), ui_s(232), ui_s(116), ui_s(42), ui_s(8), UI_CARD);
			ui_label(canvas, tr(qr_visible ? Text::OtaDetails : (s.lan ? Text::WebQr : Text::OtaQr)),
				ui_s(134), ui_s(247), ui_s(92), 1, UI_TEXT);
		}
	}
	canvas->flush();
}
}

void ota_screen_begin(Arduino_GFX *gfx, uint32_t now, bool faces) {
	canvas = gfx;
	drawn = now - 200;
	buzzer_end();
	backlight_activity(now);
	ota_begin(now, draw, faces);
	const auto &s = ota_status();
	strncpy(cached_url, s.url, sizeof(cached_url) - 1);
	qr_ready = !s.error && (s.lan ? ota_qr_prepare_url(s.url) : ota_qr_prepare(s.ssid, s.password));
	qr_visible = qr_ready;
	draw();
}

void ota_screen_poll(uint32_t now) {
	touch_poll(now);
	int16_t x = 0, y = 0;
	bool tap = touch_take_tap(x, y);
	bool receiving = ota_status().receiving;
	if (tap && y >= ui_s(232) && !receiving && !ota_status().success) {
		if (x < ui_s(120)) { ota_stop(); }
		else if (qr_ready && !ota_status().error) { qr_visible = !qr_visible; drawn = now - 200; }
	}
	if (ota_status().active) {
		ota_poll(now);
		const auto &s = ota_status();
		if (s.lan && strcmp(cached_url, s.url)) {
			strncpy(cached_url, s.url, sizeof(cached_url) - 1);
			qr_ready = ota_qr_prepare_url(s.url); qr_visible = qr_ready; drawn = now - 200;
		}
		draw();
	}
}
