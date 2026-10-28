/** @brief Observable QR encoder callback; does not implement or validate QR encoding. */
#pragma once
#include <esp_ota_ops.h>
#include <string>
using esp_qrcode_handle_t = const uint8_t *;
struct esp_qrcode_config_t {
	void (*display_func)(esp_qrcode_handle_t);
	int max_qrcode_version;
	int qrcode_ecc_level;
};
constexpr int ESP_QRCODE_ECC_MED = 1;
inline int test_qr_size = 33, test_qr_result = ESP_OK, test_qr_calls = 0;
inline int test_qr_version = 0, test_qr_ecc = 0;
inline std::string test_qr_payload;
inline esp_err_t esp_qrcode_generate(esp_qrcode_config_t *config, const char *text) {
	++test_qr_calls;
	test_qr_payload = text;
	test_qr_version = config->max_qrcode_version;
	test_qr_ecc = config->qrcode_ecc_level;
	uint8_t dummy = 0;
	if (test_qr_result == ESP_OK) { config->display_func(&dummy); }
	return test_qr_result;
}
inline int esp_qrcode_get_size(esp_qrcode_handle_t) { return test_qr_size; }
inline bool esp_qrcode_get_module(esp_qrcode_handle_t, int x, int y) { return (x + y) % 3 == 0; }
