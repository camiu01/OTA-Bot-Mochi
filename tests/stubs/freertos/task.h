/**
 * @file task.h
 * @brief Capture the PCM worker for deterministic native execution.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <driver/i2s_std.h>
#define pdMS_TO_TICKS(ms) (ms)
#define pdPASS 1
inline bool test_task_failure = false;
inline unsigned test_audio_tasks = 0;
inline void (*test_audio_task)(void *) = nullptr;
inline int xTaskCreate(void (*entry)(void *), const char *, unsigned, void *, unsigned, void *) {
	if (test_task_failure) { return 0; }
	test_audio_task = entry;
	++test_audio_tasks;
	return pdPASS;
}
inline void vTaskDelay(unsigned) { throw TestAudioYield{}; }
