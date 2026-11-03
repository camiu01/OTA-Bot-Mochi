/**
 * @file weather_memory.h
 * @brief Keep TLS startup away from a fragmented or nearly exhausted internal heap.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <esp_heap_caps.h>

/** @brief Require a contiguous internal 45 KiB block before allocating the HTTPS body and TLS state. @return Safe to try. */
inline bool weather_memory_ready() {
	return heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT) >= 45u * 1024u;
}
