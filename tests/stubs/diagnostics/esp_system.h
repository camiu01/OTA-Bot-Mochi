/** @file esp_system.h
 * @brief Controlled reset reasons for platform diagnostics tests.
 */
#pragma once
enum esp_reset_reason_t {
	ESP_RST_UNKNOWN, ESP_RST_POWERON, ESP_RST_EXT, ESP_RST_SW, ESP_RST_PANIC,
	ESP_RST_INT_WDT, ESP_RST_TASK_WDT, ESP_RST_WDT, ESP_RST_DEEPSLEEP, ESP_RST_BROWNOUT
};
inline esp_reset_reason_t test_reset_reason = ESP_RST_UNKNOWN;
/** @brief Read the controlled reset reason. @return Reset reason. */
inline esp_reset_reason_t esp_reset_reason() { return test_reset_reason; }
