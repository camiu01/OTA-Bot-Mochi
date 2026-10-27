/**
 * @brief Diagnostic interval maxima, unchanged draws and timestamp rollover.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/diagnostics/render_metrics.h"
#include <assert.h>

int main() {
	RenderMetrics m;
	m.record(100, 200, 500);
	assert(m.samples == 1 && m.drawMaxUs == 100 && m.flushMaxUs == 300);
	assert(m.combinedMaxUs == 400 && m.overBudget == 0);
	m.record(UINT32_MAX - 9, 20, 50);
	assert(m.samples == 2 && m.drawMaxUs == 100 && m.flushMaxUs == 300);
	m.record(1000, 34000, 34000);
	assert(m.drawMaxUs == 33000 && m.overBudget == 0);
	m.record(0, 20000, 40000);
	assert(m.combinedMaxUs == 40000 && m.overBudget == 1);
	m = {};
	assert(m.samples == 0 && m.drawMaxUs == 0 && m.flushMaxUs == 0);
	m.record(UINT32_MAX - 9, 20, 50);
	assert(m.drawMaxUs == 30 && m.flushMaxUs == 30 && m.combinedMaxUs == 60);
}
