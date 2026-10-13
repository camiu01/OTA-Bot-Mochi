/**
 * @file FreeRTOS.h
 * @brief Native test definitions for the UI snapshot mutex.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <mutex>
#define portMAX_DELAY 0xFFFFFFFF
struct StaticSemaphore_t { std::mutex mutex; };
using SemaphoreHandle_t = StaticSemaphore_t *;
