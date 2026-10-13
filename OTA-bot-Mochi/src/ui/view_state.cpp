/**
 * @file view_state.cpp
 * @brief Session overrides remain separate from persistent preferences.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "view_state.h"
static int8_t route_mode = -1;
static DeskTimer timer;
static uint32_t revision = 0;
/** @brief Resolve presentation. @param preference Default. @return Effective layout. */
bool view_route_large(bool preference) { return route_mode < 0 ? preference : route_mode != 0; }
/** @brief Assign valid override only. @param mode Default/details/large. */
void view_route_temporary(int8_t mode) {
	if (mode >= -1 && mode <= 1 && route_mode != mode) { route_mode = mode; ++revision; }
}
/** @brief Reset session state independently of flash. */
void view_reset() { route_mode = -1; timer = DeskTimer{}; ++revision; }
/** @brief Expose session view changes. @return Revision. */
uint32_t view_revision() { return revision; }
/** @brief Access local timer. @return Timer. */
DeskTimer &view_timer() { return timer; }
