/**
 * @file companion_panel_test.cpp
 * @brief Bilingual phone controls, QR quiet zones, contextual navigation and iPhone-only visibility guards.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/companion/companion_state.h"
#include "../OTA-bot-Mochi/src/companion/phone_view.h"
#include "../OTA-bot-Mochi/src/companion/phone_qr.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/ui/screens.h"
#include "../OTA-bot-Mochi/src/ui/ui_theme.h"
#include "../OTA-bot-Mochi/src/ui/i18n.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include <qrcode.h>
#include <cassert>
#include <iostream>

/** @brief Seed only validated completed phone content. @param watch Phone fixture. */
static void seed(ChronosESP32 &watch) {
	companion_connection(true);
	companion_config(watch, CF_PBAT, 1, 72);
	companion_config(watch, CF_ALARM, 0, (uint32_t(7) << 24) | (uint32_t(30) << 16) | 0x7f01);
	watch.contact_count = 1; watch.contacts[0] = Contact{"Contact with a long name to clip safely", "+39123456789012345678"};
	companion_config(watch, CF_CONTACT, 1, 1);
	watch.qr_links[0] = "https://example.com/contact";
	companion_config(watch, CF_QR, 0, 0);
	companion_config(watch, CF_LANG, 0, 12);
	uint8_t packet[] = {0xea, 0, 0, 0xfe, 0x7e, 2, 1, 0, 0x10, 21, 0, 12, 80, 2};
	companion_data(packet, sizeof(packet), 0);
}
/** @brief Render every phone subview in each locale with real layout and fake pixels. @param gfx Canvas. */
static void panels(Arduino_GFX &gfx) {
	for (uint8_t language = 0; language < 2; ++language) {
		CompanionSettings value = settings_get(); value.language = language; assert(settings_update(value));
		phone_view_draw(&gfx, 0);
		for (uint8_t i = 2; i < 8; ++i) {
			phone_view_tap(ui_s(i % 2 ? 170 : 50), ui_s(88 + (i / 2) * 41));
			gfx.fillScreen(UI_BG); phone_view_draw(&gfx, 0);
			assert(phone_view_scroll(1));
		}
	}
}
/** @brief Exact QR payloads are cached; malformed payloads and encoder failures do not draw. @param gfx Canvas. */
static void qr_checks(Arduino_GFX &gfx) {
	test_qr_size = 41;
	assert(phone_qr_draw(&gfx, "https://example.com/raster"));
	assert(test_qr_version == 6 && test_qr_ecc == ESP_QRCODE_ECC_MED);
	int calls = test_qr_calls;
	assert(phone_qr_draw(&gfx, "https://example.com/raster") && test_qr_calls == calls);
	int scale = ui_s(136) / 49, side = scale * 49, left = (gfx.w - side) / 2, top = ui_s(78);
	for (int y = 0; y < side; ++y) {
		for (int x = 0; x < side; ++x) {
			if (x < 4 * scale || x >= side - 4 * scale || y < 4 * scale || y >= side - 4 * scale) {
				assert(gfx.pixels[(top + y) * gfx.w + left + x] == 0xffff);
			}
		}
	}
	assert(!phone_qr_draw(&gfx, std::string(97, 'a')));
	assert(!phone_qr_draw(&gfx, std::string("a\0b", 3)));
	test_qr_result = 1;
	assert(!phone_qr_draw(&gfx, "different"));
	test_qr_result = ESP_OK; test_qr_size = 33;
}
/** @brief Open the first available record rather than an empty slot zero. @param gfx Canvas. @param watch Phone fixture. */
static void sparse_qr_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	watch.qr_links[0] = ""; companion_config(watch, CF_QR, 0, 0);
	watch.qr_links[2] = "https://example.com/second"; companion_config(watch, CF_QR, 0, 2);
	phone_view_tap(ui_s(170), ui_s(170));
	phone_view_draw(&gfx, 0);
	assert(test_qr_payload == "https://example.com/second");
	assert(phone_view_scroll(1));
}
/** @brief Chronos-only tools disappear in iPhone mode while the main carousel remains unchanged. @param gfx Canvas. @param watch Clock. */
static void ios_checks(Arduino_GFX &gfx, ChronosESP32 &watch) {
	assert(screens_parent(UiPage::Phone) == UiPage::Clock);
	assert(screens_tap(UiPage::Desk, ui_s(170), ui_s(180), 0) == UiPage::Phone);
	CompanionSettings value = settings_get(); value.phone_ios = true; assert(settings_update(value));
	assert(screens_tap(UiPage::Desk, ui_s(170), ui_s(180), 0) == UiPage::Desk);
	for (UiPage page : {UiPage::Desk, UiPage::Phone, UiPage::Clock, UiPage::Weather, UiPage::BigClock, UiPage::AnalogClock}) {
		gfx.text.clear(); screens_invalidate(); assert(screens_draw(&gfx, watch, page, 30000));
		for (const std::string &text : gfx.text) {
			assert(text.find("CHRONOS") == std::string::npos && text.find("Chronos") == std::string::npos);
			assert(text != tr(Text::CompanionTitle));
		}
	}
}
/** @brief Execute the panel-size-specific phone UI proof. @return Success. */
int main() {
	assert(ui_state_init() && settings_init() && companion_init());
	Arduino_GFX gfx; ChronosESP32 watch;
	eyes_init(&gfx); settings_apply(); seed(watch);
	panels(gfx); qr_checks(gfx); sparse_qr_checks(gfx, watch); ios_checks(gfx, watch);
	std::cout << "Phone tools, bilingual panels, QR bounds and iPhone visibility passed on " << gfx.w << "x" << gfx.h << "\n";
	return 0;
}
