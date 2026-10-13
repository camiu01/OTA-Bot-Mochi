/**
 * @file auto_page.h
 * @brief Edge-triggered page selection, without repeated jumps on route refresh.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "screens.h"

struct AutoPagePolicy {
	uint32_t notifications = 0;
	uint32_t route_starts = 0;

	/**
	 * @brief Select a page on new alerts or actual navigation starts.
	 * @param current Current page.
	 * @param events Coherent event counters.
	 * @param touching Postpone switching until a held finger is released.
	 * @return Target page; a simultaneous route start wins over a notification.
	 */
	UiPage update(UiPage current, const UiPageEvents &events, bool touching) {
		if (touching) { return current; }
		bool notification = events.notifications != notifications;
		bool route_start = events.route_starts != route_starts && events.route_active;
		notifications = events.notifications;
		route_starts = events.route_starts;
		if (route_start) { return UiPage::Navigation; }
		if (notification) { return UiPage::Notification; }
		return current;
	}
};
