/**
 * @file ota_qr_test.cpp
 * @brief Wi-Fi payload, cache reset, integer scaling and quiet-zone checks, not scan proof.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/update/ota_qr.h"
#include <qrcode.h>
#include <cassert>
#include <iostream>

/** @brief Compare every painted module and the complete four-module quiet zone. */
static void raster_check(int16_t side) {
	Arduino_GFX gfx(LCD_WIDTH, LCD_HEIGHT);
	gfx.fillScreen(0x1234);
	int16_t x = (gfx.width() - side) / 2, y = 52;
	int calls = test_qr_calls;
	assert(ota_qr_draw(&gfx, x, y, side));
	assert(test_qr_calls == calls); // Rendering reuses the cached matrix.
	int scale = side / 41, pixels = scale * 41;
	int left = x + (side - pixels) / 2, top = y + (side - pixels) / 2;
	for (int row = 0; row < gfx.height(); ++row) {
		for (int col = 0; col < gfx.width(); ++col) {
			uint16_t expected = 0x1234;
			if (col >= left && col < left + pixels && row >= top && row < top + pixels) {
				int mx = (col - left) / scale - 4, my = (row - top) / scale - 4;
				expected = mx >= 0 && mx < 33 && my >= 0 && my < 33 && (mx + my) % 3 == 0 ? 0 : 0xffff;
			}
			assert(gfx.pixels[size_t(row) * gfx.width() + col] == expected);
		}
	}
}

int main() {
	Arduino_GFX gfx;
	assert(!ota_qr_draw(&gfx, 0, 0, 124));
	assert(ota_qr_prepare("Mochi-Update-abcdef", "ABCD2345"));
	assert(test_qr_payload == "WIFI:T:WPA;S:Mochi-Update-abcdef;P:ABCD2345;;");
	assert(test_qr_version == 4 && test_qr_ecc == ESP_QRCODE_ECC_MED);
	raster_check(124);
	raster_check(189);
	assert(!ota_qr_draw(nullptr, 0, 0, 124));
	assert(!ota_qr_draw(&gfx, 0, 0, 40));
	assert(!ota_qr_draw(&gfx, -1, 0, 124));
	assert(!ota_qr_draw(&gfx, gfx.width() - 10, 0, 124));
	for (const char *ssid : {"", "bad;ssid", "bad:ssid", "bad\\ssid", "bad\"ssid", "123456789012345678901234"}) {
		assert(!ota_qr_prepare(ssid, "ABCD2345"));
		assert(!ota_qr_draw(&gfx, 0, 0, 124));
	}
	assert(!ota_qr_prepare(nullptr, "ABCD2345"));
	assert(!ota_qr_prepare("Mochi", nullptr));
	assert(!ota_qr_prepare("Mochi", "short"));
	assert(!ota_qr_prepare("Mochi", "ABCD23456"));
	assert(!ota_qr_prepare("Mochi", "ABCD;345"));
	assert(ota_qr_prepare("Mochi-Update-123456", "Z8765432"));
	assert(test_qr_payload.find("abcdef") == std::string::npos);
	test_qr_result = 1;
	assert(!ota_qr_prepare("Mochi", "ABCD2345") && !ota_qr_draw(&gfx, 0, 0, 124));
	test_qr_result = ESP_OK; test_qr_size = 37;
	assert(!ota_qr_prepare("Mochi", "ABCD2345") && !ota_qr_draw(&gfx, 0, 0, 124));
	std::cout << "OTA Wi-Fi payload, QR cache, module scale and quiet zone passed\n";
}
