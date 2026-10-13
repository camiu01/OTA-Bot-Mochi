/**
 * @file semphr.h
 * @brief Native mutex substitutes used only by host tests.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "FreeRTOS.h"
inline SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t *storage) { return storage; }
inline void xSemaphoreTake(SemaphoreHandle_t lock, uint32_t) { lock->mutex.lock(); }
inline void xSemaphoreGive(SemaphoreHandle_t lock) { lock->mutex.unlock(); }
