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
	UiPage return_page = UiPage::Eyes;
	uint32_t notification_started = 0;
	bool returning = false;

	/**
	 * @brief Select a page on new alerts or actual navigation starts.
	 * @param current Current page.
	 * @param events Coherent event counters.
	 * @param touching Postpone switching until a held finger is released.
	 * @param now Millisecond timestamp.
	 * @return Target page; a simultaneous route start wins over a notification.
	 */
	UiPage update(UiPage current, const UiPageEvents &events, bool touching, uint32_t now = 0) {
		if (current != UiPage::Notification) { returning = false; }
		if (touching) { return current; }
		bool notification = events.notifications != notifications;
		bool route_start = events.route_starts != route_starts && events.route_active;
		notifications = events.notifications;
		route_starts = events.route_starts;
		if (route_start) { returning = false; return UiPage::Navigation; }
		if (notification) {
			if (current != UiPage::Notification) { return_page = current; returning = true; }
			notification_started = now;
			return UiPage::Notification;
		}
		if (returning && uint32_t(now - notification_started) >= 60000) {
			returning = false;
			return return_page;
		}
		return current;
	}
};
