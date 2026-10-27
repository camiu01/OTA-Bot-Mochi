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
#include "src/eyes/imu_orientation.h"
#include "src/touch/touch.h"
#include "src/clock/clock_ui.h"
#include "src/ui/screens.h"
#include "src/ui/auto_page.h"
#include "src/ui/view_state.h"
#include "src/ui/weather_sync.h"
#include "src/sound/sound.h"
#include "src/ui/ui_theme.h"
#include "src/ui/startup.h"
#include "src/power/backlight.h"
#include "src/power/battery.h"
#include "src/power/power_button_shutdown.h"
#include "src/settings/settings.h"
#include "src/settings/settings_menu.h"
#include "src/settings/usb_commands.h"
#include "src/bus/i2c_bus.h"
#include "runtime_diagnostics.h"

ChronosESP32 watch("OTA bot");
#if MOCHI_BOARD_LCD_169
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *display = new Arduino_ST7789(bus, LCD_RST, 0, true,
	LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);
#else
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
	LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);
// CO5300: the visible area starts at RAM column 16; the panel reset is held by
// the TCA9554 expander, which init_screen() releases before the first command.
Arduino_CO5300 *display = new Arduino_CO5300(bus, -1, 0,
	LCD_WIDTH, LCD_HEIGHT, 16, 0, 0, 0);
#endif
Arduino_GFX *gfx = new Arduino_Canvas(LCD_WIDTH, LCD_HEIGHT, display);
UiPage current_screen = UiPage::Eyes;
AutoPagePolicy auto_page;
bool screen_ready = false;
bool applied_flip = false;
StartupAnimation startup;
bool starting = true;
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
#if MOCHI_BOARD_LCD_169
	// Chronos 1.9.1 has no 240x280 preset; this is the nearest rectangular metadata profile.
	watch.setScreen(CS_240x283_172_RTF);
#else
	// Chronos 1.9.1 has no 368x448 preset; nearest profile by aspect ratio and area.
	watch.setScreen(CS_410x494_200_RTF);
#endif
	watch.setConnectionCallback(connection_callback);
	watch.setNotificationCallback(notification_callback);
	watch.setConfigurationCallback(config_callback);
	watch.setRingerCallback(ringer_callback);
	watch.begin();
}

#if !MOCHI_BOARD_LCD_169
/** @brief Read one TCA9554 register. @param wire Shared bus. @param reg Register. @param value Output value. @return Success. */
bool expander_read(TwoWire &wire, uint8_t reg, uint8_t &value) {
	wire.beginTransmission(IO_EXPANDER_ADDR);
	wire.write(reg);
	if (wire.endTransmission(false) != 0) { return false; }
	if (wire.requestFrom((uint8_t)IO_EXPANDER_ADDR, (uint8_t)1) != 1) { return false; }
	value = wire.read();
	return true;
}

/** @brief Write one TCA9554 register. @param wire Shared bus. @param reg Register. @param value Value. @return Success. */
bool expander_write(TwoWire &wire, uint8_t reg, uint8_t value) {
	wire.beginTransmission(IO_EXPANDER_ADDR);
	wire.write(reg);
	wire.write(value);
	return wire.endTransmission() == 0;
}

/**
 * @brief Release the AMOLED panel reset held by the TCA9554 expander.
 *
 * @details The V2 board gates the panel reset on expander bits 0..2 at I2C
 *          0x20. Init commands sent while the panel is held in reset succeed
 *          silently and show nothing, so this must run before gfx->begin().
 *          Raw register writes keep the firmware free of an extra library.
 */
void release_panel_reset() {
	if (!i2c_bus_begin(IIC_SDA, IIC_SCL, 400000)) {
		Serial.println("Panel expander I2C initialization failed");
		return;
	}
	TwoWire &wire = i2c_bus();
	uint8_t config = 0, output = 0;
	if (!expander_read(wire, 0x03, config)) {
		Serial.println("Panel expander not responding");
		return;
	}
	if (!expander_write(wire, 0x03, uint8_t(config & ~IO_EXPANDER_RESET_PINS))) {
		Serial.println("Panel expander configuration failed");
		return;
	}
	(void)expander_read(wire, 0x01, output);
	(void)expander_write(wire, 0x01, uint8_t(output | IO_EXPANDER_RESET_PINS));
	delay(100);
	(void)expander_write(wire, 0x01, uint8_t(output & ~IO_EXPANDER_RESET_PINS));
	delay(300);
	(void)expander_write(wire, 0x01, uint8_t(output | IO_EXPANDER_RESET_PINS));
}

/** @brief Route the shared brightness policy into the AMOLED controller. @param percent Brightness percentage. */
void apply_display_brightness(uint8_t percent) {
	display->setBrightness(uint8_t(uint32_t(percent) * 255 / 100));
}
#endif

/** @brief Initialize the buffered canvas; never draw after an allocation failure. */
void init_screen() {
#if !MOCHI_BOARD_LCD_169
	release_panel_reset();
	backlight_bind_display(apply_display_brightness);
#endif
	if (!gfx->begin()) {
		Serial.println("Display/framebuffer initialization failed");
		return;
	}
	screen_ready = true;
	applied_flip = settings_get().flipped;
	imu_orientation().applied = applied_flip;
	gfx->setRotation(applied_flip ? 2 : 0);
	touch_set_flipped(applied_flip);
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
 * @brief Apply the saved 180-degree mounting orientation and repaint the whole canvas.
 * @param flip True when the device is mounted upside down.
 * @param now Millisecond timestamp.
 */
void apply_display_flip(bool flip, uint32_t now) {
	applied_flip = flip;
	imu_orientation().applied = flip;
	gfx->setRotation(flip ? 2 : 0);
	touch_set_flipped(flip);
	switch_screen(current_screen, now);
}

/**
 * @brief Toggle master sound from the status-bar strip; RAM-only until SAVE.
 * @param now Millisecond timestamp.
 */
void toggle_quick_sound(uint32_t now) {
	CompanionSettings value = settings_get();
	value.sound = !value.sound;
	if (!settings_update(value)) { return; }
	if (value.sound) { sound_request(SoundCue::Tap, now); }
}

/**
 * @brief Handle parked-use taps, a utility-page mute toggle, double taps, petting
 * strokes, a hold-to-settings shortcut, contextual vertical gestures, and page swipes.
 * @param now Millisecond timestamp.
 */
void handle_touch(uint32_t now) {
	int8_t step = touch_poll(now);
	bool hold = touch_take_hold();
	int16_t x, y;
	bool tap = touch_take_tap(x, y);
	bool dbl = touch_take_double();
	int16_t pet_x = 0, pet_y = 0;
	bool pet = touch_take_pet(pet_x, pet_y);
	int8_t scroll = touch_take_scroll();
	if (starting) { return; }
	if (tap && y < 50 && !screens_calling() && current_screen != UiPage::Eyes &&
		current_screen != UiPage::Settings) {
		toggle_quick_sound(now);
		return;
	}
	if (scroll && current_screen != UiPage::Eyes && !screens_calling()) {
		if (current_screen == UiPage::Settings) {
			if (settings_menu_scroll(scroll)) { screens_invalidate(); }
			return;
		}
		UiPage target = screens_scroll(current_screen, scroll);
		if (target != current_screen) { switch_screen(target, now); }
		return;
	}
	if (hold && current_screen == UiPage::Eyes && !screens_calling()) {
		switch_screen(UiPage::Settings, now);
		return;
	}
	if (hold && current_screen == UiPage::Navigation && !screens_calling()) {
		view_route_temporary(-1);
		screens_invalidate();
		return;
	}
	if (pet && current_screen == UiPage::Eyes && !screens_calling()) { eyes_faces_pet(now, pet_x, pet_y); }
	if (tap && current_screen == UiPage::Settings && !screens_calling()) {
		MenuAction action = settings_menu_tap(x, y);
		if (action == MenuAction::Exit) { switch_screen(UiPage::Eyes, now); }
		else if (action == MenuAction::Changed) { screens_invalidate(); }
	} else if (tap && !screens_calling()) {
		if (current_screen == UiPage::Eyes) {
			if (dbl) { eyes_faces_double(now, x, y); }
			else { eyes_faces_tap(now, x, y); }
		} else {
			UiPage target = screens_tap(current_screen, x, y, now);
			if (target != current_screen) { switch_screen(target, now); }
		}
	}
	if (step == 0) { return; }
	switch_screen(screens_swipe(current_screen, step), now);
}

/** @brief Follow new Chronos events without changing screens from BLE callbacks. @param now Timestamp. */
void handle_auto_page(uint32_t now) {
	if (starting) { return; }
	UiPage target = auto_page.update(current_screen, ui_page_events(), touch_pressed());
	if (target != current_screen) {
		backlight_activity(now);
		switch_screen(target, now);
	}
}

/** @brief Initialize display, snapshot lock, BLE, eyes, and touch. */
void setup() {
	bool power_ready = power_button_init();
	Serial.begin(115200);
	if (!settings_init()) { Serial.println("Settings storage unavailable; using RAM defaults"); }
	if (!power_ready) { Serial.println("PWR button configuration unavailable"); }
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
	startup.begin(millis());
	last_frame_ms = millis() - FRAME_INTERVAL_MS;
	Serial.printf("UI ready, free heap: %lu bytes\n", static_cast<unsigned long>(ESP.getFreeHeap()));
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
	handle_power_button(watch, display, screen_ready, millis());
	settings_usb_poll();
	watch.loop();
	delay(1);
	if (!screen_ready) { return; }
	uint32_t now = millis();
#if MOCHI_RUNTIME_DIAGNOSTICS
	runtime_diagnostics::poll(now);
#endif
	handle_touch(now);
	eyes_imu_poll(now);
	bool flip = imu_orientation().target(settings_get().flipped);
	if (!touch_pressed() && flip != applied_flip) {
		apply_display_flip(flip, now);
	}
	handle_auto_page(now);
	if (weather_sync_due(now, watch.isConnected(), watch.isSubscribed())) { watch.syncRequest(); }
	battery_sample(now);
	view_timer().tick(now);
	sound_poll(now, ui_page_events(), current_screen == UiPage::Eyes && !screens_calling(),
		view_timer().resting, view_timer().finished);
	update_backlight(now);
	if ((uint32_t)(now - last_frame_ms) < FRAME_INTERVAL_MS) { return; }
	last_frame_ms = now;
	if (starting) {
		if (startup.draw(gfx, now, UI_TEXT)) { gfx->flush(); return; }
		starting = false;
		eyes_faces_greet(now);
		switch_screen(UiPage::Eyes, now);
	}
#if MOCHI_RUNTIME_DIAGNOSTICS
	uint32_t draw_started = micros();
#endif
	bool changed = screens_draw(gfx, watch, current_screen, now);
	if (current_screen == UiPage::Eyes && !screens_calling()) {
		if (changed) { eyes_state().dirtyInit = false; }
		eyes_update(now);
		changed = true;
	}
#if MOCHI_RUNTIME_DIAGNOSTICS
	uint32_t draw_finished = micros();
#endif
	if (changed) { gfx->flush(); }
#if MOCHI_RUNTIME_DIAGNOSTICS
	runtime_diagnostics::metrics.record(draw_started, draw_finished, changed ? micros() : draw_finished);
#endif
}
