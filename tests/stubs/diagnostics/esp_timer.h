/** @file esp_timer.h
 * @brief Controlled 64-bit monotonic uptime for diagnostics tests.
 */
#pragma once
#include <cstdint>
inline int64_t test_uptime_us = 0;
/** @brief Read the controlled monotonic timer. @return Microseconds. */
inline int64_t esp_timer_get_time() { return test_uptime_us; }
