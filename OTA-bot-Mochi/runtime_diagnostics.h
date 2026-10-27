/**
 * @file runtime_diagnostics.h
 * @brief Opt-in USB summaries, owned only by the main task.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#ifndef MOCHI_RUNTIME_DIAGNOSTICS
#define MOCHI_RUNTIME_DIAGNOSTICS 0
#endif
#if MOCHI_RUNTIME_DIAGNOSTICS
#include <esp_heap_caps.h>
#include <esp_system.h>
#include "src/diagnostics/render_metrics.h"

namespace runtime_diagnostics {
inline RenderMetrics metrics;
inline uint32_t lastReport = 0;

/** @brief Report heap capability pools and reset reason, at most once per minute.
 * @param now Millisecond uptime; summaries never contain phone payloads.
 */
inline void poll(uint32_t now) {
	if (uint32_t(now - lastReport) < 60000) { return; }
	lastReport = now;
	if (Serial) {
		const uint32_t internal = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
		const uint32_t external = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;
		Serial.printf("diag uptime_ms=%lu reset=%d internal_free=%u internal_min=%u psram_free=%u psram_min=%u\n",
			(unsigned long)now, int(esp_reset_reason()),
			unsigned(heap_caps_get_free_size(internal)), unsigned(heap_caps_get_minimum_free_size(internal)),
			unsigned(heap_caps_get_free_size(external)), unsigned(heap_caps_get_minimum_free_size(external)));
		Serial.printf("diag samples=%lu draw_max_us=%lu flush_call_max_us=%lu draw_flush_max_us=%lu over_33ms=%lu\n",
			(unsigned long)metrics.samples, (unsigned long)metrics.drawMaxUs,
			(unsigned long)metrics.flushMaxUs, (unsigned long)metrics.combinedMaxUs,
			(unsigned long)metrics.overBudget);
	}
	metrics = {};
}
}
#endif
