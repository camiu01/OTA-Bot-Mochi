/** @brief Deterministic native-only entropy for session checks. */
#pragma once
#include <stddef.h>
#include <stdint.h>
inline void esp_fill_random(void *buffer, size_t size) {
	static uint8_t value = 0;
	auto *out = static_cast<uint8_t *>(buffer);
	for (size_t i = 0; i < size; ++i) { out[i] = ++value; }
}
