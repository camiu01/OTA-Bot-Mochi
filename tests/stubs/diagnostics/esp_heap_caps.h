/** @file esp_heap_caps.h
 * @brief Controlled capability pools for platform diagnostics tests.
 */
#pragma once
#include <cstddef>
#include <cstdint>
constexpr uint32_t MALLOC_CAP_INTERNAL = 1, MALLOC_CAP_8BIT = 2, MALLOC_CAP_SPIRAM = 4;
inline size_t test_internal_free = 123456, test_internal_min = 34567, test_psram_free = 7654321;
/** @brief Return a controlled pool. @param caps Capability mask. @return Free bytes. */
inline size_t heap_caps_get_free_size(uint32_t caps) {
	return caps == (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT) ? test_internal_free : test_psram_free;
}
/** @brief Return the controlled internal low-water mark. @param caps Capability mask. @return Minimum bytes. */
inline size_t heap_caps_get_minimum_free_size(uint32_t caps) {
	return caps == (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT) ? test_internal_min : 0;
}
