/**
 * @file task.h
 * @brief Declaration-only FreeRTOS task API for the obd_ble.cpp host syntax check.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#define pdMS_TO_TICKS(ms) (ms)
#define pdPASS 1
int xTaskCreatePinnedToCore(void (*entry)(void *), const char *name, unsigned stack, void *arg,
	unsigned priority, void *handle, int core);
void vTaskDelay(unsigned ticks);
