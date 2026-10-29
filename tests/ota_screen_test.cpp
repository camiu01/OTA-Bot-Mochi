/**
 * @file ota_screen_test.cpp
 * @brief Maintenance QR/details controls and text bounds with fake session/encoder.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/update/ota_screen.h"
#include "../OTA-bot-Mochi/src/update/ota_update.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/ui/i18n.h"
#include "../OTA-bot-Mochi/src/ui/ui_theme.h"
#include "../OTA-bot-Mochi/src/power/backlight.h"
#include <qrcode.h>
#include <algorithm>
#include <cassert>
#include <iostream>

namespace {
OtaStatus session;
bool tapped = false;
int16_t tap_x = 0, tap_y = 0;
}

void ota_begin(uint32_t, void (*)(), bool faces) {
	session = OtaStatus{};
	session.active = true;
	session.faces = faces;
	strcpy(session.ssid, "Mochi-Update-abcdef");
	strcpy(session.password, "ABCD2345");
}
void ota_poll(uint32_t) {}
void ota_stop() { session = OtaStatus{}; }
const OtaStatus &ota_status() { return session; }
void backlight_activity(uint32_t) {}
void backlight_update(uint32_t, bool touching, bool keep_awake) { assert(!touching && keep_awake); }
int8_t touch_poll(uint32_t) { return 0; }
bool touch_take_tap(int16_t &x, int16_t &y) {
	x = tap_x; y = tap_y;
	bool result = tapped; tapped = false; return result;
}

/** @brief Refresh at a controlled timestamp with optional released footer tap. */
static void refresh(Arduino_GFX &gfx, int16_t x = -1) {
	gfx.text.clear();
	test_now += 201;
	if (x >= 0) { tapped = true; tap_x = ui_s(x); tap_y = ui_s(248); }
	ota_screen_poll(test_now);
}

/** @brief Find a complete drawn label without assuming draw call order. */
static bool contains(const Arduino_GFX &gfx, const char *text) {
	return std::find(gfx.text.begin(), gfx.text.end(), text) != gfx.text.end();
}

int main() {
	assert(settings_init());
	eyes_init(nullptr);
	for (uint8_t language = 0; language < 2; ++language) {
		CompanionSettings profile = settings_get();
		profile.language = language;
		assert(settings_update(profile));
		Arduino_GFX gfx;
		test_now += 1000;
		ota_screen_begin(&gfx, test_now);
		assert(contains(gfx, tr(Text::OtaScan)));
		assert(contains(gfx, tr(Text::OtaDetails)));
		assert(contains(gfx, "Mochi-Update-abcdef"));
		assert(contains(gfx, "http://192.168.4.1"));
		int encoded = test_qr_calls;
		refresh(gfx, 180);
		assert(session.active && contains(gfx, tr(Text::OtaBrowser)));
		assert(contains(gfx, "ABCD2345") && contains(gfx, tr(Text::OtaQr)));
		refresh(gfx, 180);
		assert(contains(gfx, tr(Text::OtaScan)) && test_qr_calls == encoded);
		session.receiving = true; session.total = 100; session.received = 25;
		refresh(gfx);
		assert(contains(gfx, tr(Text::OtaWriting)) && contains(gfx, "25%"));
		assert(!contains(gfx, tr(Text::OtaScan)));
		refresh(gfx, 20); assert(session.active);
		session.receiving = false; session.error = "Upload interrupted. Reopen update mode.";
		refresh(gfx, 180);
		assert(contains(gfx, tr(Text::Failed)) && !contains(gfx, tr(Text::OtaScan)));
		refresh(gfx, 20); assert(!session.active);
		gfx.text.clear(); ota_screen_begin(&gfx, test_now, true);
		session.lan = true; strcpy(session.address, "192.168.1.42");
		strcpy(session.url, "http://192.168.1.42/#0123456789abcdef0123456789abcdef");
		refresh(gfx);
		assert(contains(gfx, tr(Text::WebScan)) && contains(gfx, "http://192.168.1.42"));
		assert(test_qr_payload == session.url);
		refresh(gfx, 180);
		assert(contains(gfx, tr(Text::WebAuthorize)) && contains(gfx, tr(Text::WebQr)));
		strcpy(session.address, "192.168.1.43"); strcpy(session.url, "http://192.168.1.43/#abcdef0123456789abcdef0123456789");
		refresh(gfx);
		assert(contains(gfx, tr(Text::WebScan)) && contains(gfx, "http://192.168.1.43") && test_qr_payload == session.url);
		refresh(gfx, 20); assert(!session.active);
		test_qr_result = 1; gfx.text.clear();
		ota_screen_begin(&gfx, test_now);
		assert(contains(gfx, tr(Text::OtaBrowser)));
		assert(contains(gfx, "ABCD2345") && !contains(gfx, tr(Text::OtaScan)));
		test_qr_result = ESP_OK;
		ota_stop();
		gfx.text.clear(); ota_screen_begin(&gfx, test_now, true);
		assert(session.faces && contains(gfx, tr(Text::FacesWifi)));
		refresh(gfx, 180);
		assert(contains(gfx, tr(Text::FacesReady)) && !contains(gfx, "0%"));
		refresh(gfx, 20); assert(!session.active);
	}
	std::cout << "OTA QR/details, password fallback, progress and EN/IT panel bounds passed\n";
}
