/** @brief Observable flash operations, not real image validation or power-loss proof. */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <vector>
using esp_err_t = int;
using esp_ota_handle_t = uint32_t;
constexpr esp_err_t ESP_OK = 0;
struct esp_partition_t { uint32_t address, size; };
inline esp_partition_t test_running{0x10000, 0x640000}, test_target{0x650000, 0x640000};
inline bool test_no_slot = false;
inline int test_begin_error = 0, test_write_error = 0, test_end_error = 0, test_select_error = 0;
inline int test_begins = 0, test_aborts = 0, test_ends = 0, test_selects = 0;
inline std::vector<uint8_t> test_written;
inline const esp_partition_t *esp_ota_get_next_update_partition(const void *) { return test_no_slot ? nullptr : &test_target; }
inline const esp_partition_t *esp_ota_get_running_partition() { return &test_running; }
inline esp_err_t esp_ota_begin(const esp_partition_t *, size_t, esp_ota_handle_t *handle) {
	++test_begins; test_written.clear(); *handle = 1; return test_begin_error;
}
inline esp_err_t esp_ota_write(esp_ota_handle_t, const void *data, size_t size) {
	const auto *bytes = static_cast<const uint8_t *>(data);
	test_written.insert(test_written.end(), bytes, bytes + size); return test_write_error;
}
inline esp_err_t esp_ota_abort(esp_ota_handle_t) { ++test_aborts; return ESP_OK; }
inline esp_err_t esp_ota_end(esp_ota_handle_t) { ++test_ends; return test_end_error; }
inline esp_err_t esp_ota_set_boot_partition(const esp_partition_t *) { ++test_selects; return test_select_error; }
