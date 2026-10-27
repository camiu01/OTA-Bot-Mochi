/**
 * @file render_metrics.h
 * @brief Bounded main-task render-call timing, without allocations.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

/** @brief Interval maxima; samples include unchanged utility-page draw calls. */
struct RenderMetrics {
	uint32_t samples = 0, drawMaxUs = 0, flushMaxUs = 0, combinedMaxUs = 0;
	uint32_t overBudget = 0;

	/** @brief Record unsigned microsecond intervals, including clock rollover.
	 * @param start Before drawing. @param drawn After drawing.
	 * @param flushed After flush, or drawn when unchanged.
	 */
	void record(uint32_t start, uint32_t drawn, uint32_t flushed) {
		const uint32_t draw = drawn - start, flush = flushed - drawn;
		const uint32_t combined = flushed - start;
		++samples;
		if (draw > drawMaxUs) { drawMaxUs = draw; }
		if (flush > flushMaxUs) { flushMaxUs = flush; }
		if (combined > combinedMaxUs) { combinedMaxUs = combined; }
		if (combined > 33000) { ++overBudget; }
	}
};
