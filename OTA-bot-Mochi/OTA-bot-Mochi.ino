/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed without any warranty. See the GNU General
 * Public License for more details. A copy is included in LICENSE.
 */
/**
 * @file OTA-bot-Mochi.ino
 * @brief Buffered companion UI with task-safe Chronos events and live eyes.
 */
#include <Arduino_GFX_Library.h>
#include <ChronosESP32.h>
#include "pin_config.h"
#include "src/eyes/eyes_api.h"
#include "src/eyes/eyes_faces.h"
#include "src/touch/touch.h"
#include "src/clock/clock_ui.h"
#include "src/ui/screens.h"
#include "src/ui/auto_page.h"
#include "src/ui/view_state.h"
#include "src/ui/weather_sync.h"
#include "src/sound/sound.h"
#include "src/ui/ui_theme.h"
#include "src/power/backlight.h"
#include "src/settings/settings.h"
#include "src/settings/settings_menu.h"
#include "src/settings/usb_commands.h"

ChronosESP32 watch("OTA bot");
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *display = new Arduino_ST7789(bus, LCD_RST, 0, true,
	LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);
Arduino_GFX *gfx = new Arduino_Canvas(LCD_WIDTH, LCD_HEIGHT, display);
UiPage current_screen = UiPage::Eyes;
AutoPagePolicy auto_page;
bool screen_ready = false;
uint32_t last_frame_ms = 0;
uint32_t activity_revision = 0;
constexpr uint32_t FRAME_INTERVAL_MS = 33;

/**
 * @brief Retain a notification without logging personal content.
 * @param notification Received phone alert.
 */
void notification_callback(Notification notification) {
	ui_notification(notification);
}

/** @brief Update the call overlay. @param caller Caller ID. @param active Call state. */
void ringer_callback(String caller, bool active) {
	ui_ringer(caller, active);
}

/** @brief Cache connection changes. @param connected Bluetooth state. */
void connection_callback(bool connected) {
	ui_connection(connected);
}

/**
 * @brief Snapshot received data after Chronos updates its own state.
 * @param config Event type.
 * @param a Event parameter.
 * @param b Event parameter.
 */
void config_callback(Config config, uint32_t a, uint32_t b) {
	if (config == CF_TIME && a == 1) { clock_time_synced(); }
	ui_config(watch, config, a, b);
}

/** @brief Install callbacks after the UI lock is ready and start BLE. */
void init_chronos() {
	// Chronos 1.9.1 has no 240x280 preset; this is the nearest rectangular metadata profile.
	watch.setScreen(CS_240x283_172_RTF);
	watch.setConnectionCallback(connection_callback);
	watch.setNotificationCallback(notification_callback);
	watch.setConfigurationCallback(config_callback);
	watch.setRingerCallback(ringer_callback);
	watch.begin();
}

/** @brief Initialize the buffered canvas; never draw after an allocation failure. */
void init_screen() {
	if (!gfx->begin()) {
		Serial.println("Display/framebuffer initialization failed");
		return;
	}
	screen_ready = true;
	gfx->fillScreen(UI_BG);
	gfx->flush();
	backlight_init(millis());
}

/**
 * @brief Switch pages and invalidate the canvas.
 * @param page Next page.
 * @param now Millisecond timestamp.
 */
void switch_screen(UiPage page, uint32_t now) {
	current_screen = page;
	gfx->fillScreen(UI_BG);
	screens_invalidate();
	eyes_state().dirtyInit = false;
	last_frame_ms = now - FRAME_INTERVAL_MS;
}

/**
 * @brief Handle parked-use taps, a hold-to-settings shortcut, and page swipes.
 * @param now Millisecond timestamp.
 */
void handle_touch(uint32_t now) {
	int8_t step = touch_poll(now);
	bool hold = touch_take_hold();
	int16_t x, y;
	bool tap = touch_take_tap(x, y);
	int8_t scroll = touch_take_scroll();
	if (scroll && current_screen == UiPage::Settings && !screens_calling() &&
		settings_menu_scroll(scroll)) { screens_invalidate(); }
	if (hold && current_screen == UiPage::Eyes && !screens_calling()) {
		switch_screen(UiPage::Settings, now);
		return;
	}
	if (hold && current_screen == UiPage::Navigation && !screens_calling()) {
		view_route_temporary(-1);
		screens_invalidate();
		return;
	}
	if (tap && current_screen == UiPage::Settings && !screens_calling()) {
		MenuAction action = settings_menu_tap(x, y);
		if (action == MenuAction::Exit) { switch_screen(UiPage::Eyes, now); }
		else if (action == MenuAction::Changed) { screens_invalidate(); }
	} else if (tap && !screens_calling()) {
		if (current_screen == UiPage::Eyes) { eyes_faces_tap(now); }
		else {
			UiPage target = screens_tap(current_screen, x, y, now);
			if (target != current_screen) { switch_screen(target, now); }
		}
	}
	if (step == 0) { return; }
	switch_screen(screens_swipe(current_screen, step), now);
}

/** @brief Follow new Chronos events without changing screens from BLE callbacks. @param now Timestamp. */
void handle_auto_page(uint32_t now) {
	UiPage target = auto_page.update(current_screen, ui_page_events(), touch_pressed());
	if (target != current_screen) {
		backlight_activity(now);
		switch_screen(target, now);
	}
}

/** @brief Initialize display, snapshot lock, BLE, eyes, and touch. */
void setup() {
	Serial.begin(115200);
	if (!settings_init()) { Serial.println("Settings storage unavailable; using RAM defaults"); }
	init_screen();
	if (!ui_state_init()) {
		Serial.println("UI state initialization failed; BLE not started");
		screen_ready = false;
		return;
	}
	init_chronos();
	if (!screen_ready) { return; }
	eyes_init(gfx);
	settings_apply();
	eyes_imu_init();
	Serial.println(eyes_imu_ready() ? "IMU ready" : "IMU unavailable; using idle eyes");
	touch_init();
	sound_init(millis());
	last_frame_ms = millis() - FRAME_INTERVAL_MS;
	Serial.printf("UI ready, free heap: %u bytes\n", ESP.getFreeHeap());
}

/**
 * @brief Wake the backlight on received data and update its touch policy.
 * @param now Millisecond timestamp.
 */
void update_backlight(uint32_t now) {
	uint32_t revision = ui_revision();
	if (revision != activity_revision) {
		activity_revision = revision;
		if (!settings_get().dashboard || current_screen == UiPage::Notification ||
			current_screen == UiPage::Navigation) { backlight_activity(now); }
	}
	backlight_update(now, touch_pressed(), screens_keep_awake());
}

/** @brief Service BLE and touch continuously, rendering only complete changed frames. */
void loop() {
	settings_usb_poll();
	watch.loop();
	delay(1);
	if (!screen_ready) { return; }
	uint32_t now = millis();
	handle_touch(now);
	handle_auto_page(now);
	if (weather_sync_due(now, watch.isConnected(), watch.isSubscribed())) { watch.syncRequest(); }
	view_timer().tick(now);
	sound_poll(now, ui_page_events(), current_screen == UiPage::Eyes && !screens_calling(),
		view_timer().resting, view_timer().finished);
	update_backlight(now);
	if ((uint32_t)(now - last_frame_ms) < FRAME_INTERVAL_MS) { return; }
	last_frame_ms = now;
	bool changed = screens_draw(gfx, watch, current_screen, now);
	if (current_screen == UiPage::Eyes && !screens_calling()) {
		if (changed) { eyes_state().dirtyInit = false; }
		eyes_update(now);
		changed = true;
	}
	if (changed) { gfx->flush(); }
}
