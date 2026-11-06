/** @file phone_qr.cpp @brief Independent version-six QR cache; phone content never reaches logs or a network. */
#include "phone_qr.h"
#include "../ui/ui_theme.h"
#include <qrcode.h>
#include <string.h>
namespace {
uint8_t modules[(41 * 41 + 7) / 8] = {}, width = 0;
String cached;
/** @brief Copy encoder-owned modules before the callback returns. @param qr QR matrix. */
void capture(esp_qrcode_handle_t qr) {
	int size = esp_qrcode_get_size(qr);
	if (size < 21 || size > 41) { return; }
	width = uint8_t(size);
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			if (esp_qrcode_get_module(qr, x, y)) {
				size_t bit = size_t(y * size + x); modules[bit / 8] |= uint8_t(1U << (bit % 8));
			}
		}
	}
}
}
bool phone_qr_draw(Arduino_GFX *gfx, const String &value) {
	if (!gfx || !value.length() || value.length() > 96 || strlen(value.c_str()) != value.length()) { return false; }
	if (!(value == cached)) {
		width = 0; memset(modules, 0, sizeof(modules)); cached = value;
		esp_qrcode_config_t config = {capture, 6, ESP_QRCODE_ECC_MED};
		if (esp_qrcode_generate(&config, value.c_str()) != ESP_OK) { width = 0; }
	}
	if (!width) { return false; }
	int side = ui_s(136), scale = side / (width + 8);
	if (!scale) { return false; }
	int pixels = scale * (width + 8), left = (gfx->width() - pixels) / 2, top = ui_s(78);
	gfx->fillRect(left, top, pixels, pixels, 0xffff);
	for (uint8_t y = 0; y < width; ++y) {
		for (uint8_t x = 0; x < width; ++x) {
			size_t bit = size_t(y) * width + x;
			if (modules[bit / 8] & (1U << (bit % 8))) {
				gfx->fillRect(left + (x + 4) * scale, top + (y + 4) * scale, scale, scale, 0);
			}
		}
	}
	return true;
}
