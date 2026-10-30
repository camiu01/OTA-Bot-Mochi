/**
 * @file esp_log.h
 * @brief Capture structured warnings from controlled native transport tests.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
inline unsigned test_log_warnings = 0;
#define ESP_LOGW(tag, message) do { (void)(tag); (void)(message); ++test_log_warnings; } while (0)
