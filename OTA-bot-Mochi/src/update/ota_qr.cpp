/**
 * @file ota_qr.cpp
 * @brief Bounded RAM-only QR cache; credentials never reach the console callback.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ota_qr.h"
#include <qrcode.h>
#include <stdio.h>
#include <string.h>

namespace {
constexpr uint8_t MAX_SIDE = 33; // Version four fits the generated Wi-Fi payload.
uint8_t modules[(MAX_SIDE * MAX_SIDE + 7) / 8] = {};
uint8_t width = 0;

/** @brief Copy the temporary encoder result before its callback returns. @param qr Encoder-owned matrix. */
void capture(esp_qrcode_handle_t qr) {
	int size = esp_qrcode_get_size(qr);
	if (size < 21 || size > MAX_SIDE) { return; }
	width = uint8_t(size);
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			if (!esp_qrcode_get_module(qr, x, y)) { continue; }
			size_t bit = size_t(y * size + x);
			modules[bit / 8] |= uint8_t(1u << (bit % 8));
		}
	}
}

/** @brief Only generated ASCII credentials are supported; reject QR delimiters. @param text Credential. @param limit Maximum length. @return Safe, nonempty value. */
bool safe(const char *text, size_t limit) {
	if (!text || !*text || strlen(text) > limit) { return false; }
	for (; *text; ++text) {
		if (!(*text >= 'A' && *text <= 'Z') && !(*text >= 'a' && *text <= 'z') &&
			!(*text >= '0' && *text <= '9') && *text != '-') { return false; }
	}
	return true;
}
}

bool ota_qr_prepare(const char *ssid, const char *password) {
	width = 0;
	memset(modules, 0, sizeof(modules));
	if (!safe(ssid, 23) || !safe(password, 8) || strlen(password) != 8) { return false; }
	char payload[64];
	int used = snprintf(payload, sizeof(payload), "WIFI:T:WPA;S:%s;P:%s;;", ssid, password);
	if (used < 0 || size_t(used) >= sizeof(payload)) { return false; }
	esp_qrcode_config_t config = {capture, 4, ESP_QRCODE_ECC_MED};
	bool encoded = esp_qrcode_generate(&config, payload) == ESP_OK;
	memset(payload, 0, sizeof(payload));
	if (!encoded) { width = 0; }
	return width != 0;
}

bool ota_qr_prepare_url(const char *url) {
	width = 0; memset(modules, 0, sizeof(modules));
	if (!url || strlen(url) > 63 || strncmp(url, "http://", 7)) { return false; }
	const char *at = url + 7;
	for (int part = 0; part < 4; ++part) {
		int value = 0, digits = 0;
		while (*at >= '0' && *at <= '9') { value = value * 10 + *at++ - '0'; if (++digits > 3) { return false; } }
		if (!digits || value > 255 || (part < 3 && *at++ != '.')) { return false; }
	}
	if (strncmp(at, "/#", 2) || strlen(at + 2) != 32) { return false; }
	for (at += 2; *at; ++at) { if (!(*at >= '0' && *at <= '9') && !(*at >= 'a' && *at <= 'f')) { return false; } }
	esp_qrcode_config_t config = {capture, 4, ESP_QRCODE_ECC_MED};
	if (esp_qrcode_generate(&config, url) != ESP_OK) { width = 0; }
	return width != 0;
}

bool ota_qr_draw(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t side) {
	if (!gfx || !width || side < width + 8 || x < 0 || y < 0 ||
		x + side > gfx->width() || y + side > gfx->height()) { return false; }
	int16_t scale = side / (width + 8);
	int16_t pixels = (width + 8) * scale;
	x += (side - pixels) / 2; y += (side - pixels) / 2;
	gfx->fillRect(x, y, pixels, pixels, 0xffff);
	for (uint8_t row = 0; row < width; ++row) {
		for (uint8_t col = 0; col < width; ++col) {
			size_t bit = size_t(row) * width + col;
			if (modules[bit / 8] & (1u << (bit % 8))) {
				gfx->fillRect(x + (col + 4) * scale, y + (row + 4) * scale, scale, scale, 0);
			}
		}
	}
	return true;
}
