/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "touch.h"
#include "swipe.h"
#include "touch_actions.h"
#include "../bus/i2c_bus.h"
#include "../../pin_config.h"
#include <Arduino.h>
#include <Wire.h>

static SwipeTracker s_swipe;
static uint8_t s_address = 0;
static uint32_t s_lastPoll = 0;
static bool s_pressed = false;
static bool s_flipped = false;
static TouchActions s_actions;

/**
 * @brief Read the contact count and first contact coordinates.
 * @param data Output five-byte register block.
 * @return True only when the complete block was received.
 */
static bool read_contact(uint8_t *data) {
	TwoWire &bus = i2c_bus();
	bus.beginTransmission(s_address);
	bus.write((uint8_t)0x02);
	if (bus.endTransmission(false) != 0) {
		return false;
	}
	if (bus.requestFrom(s_address, (uint8_t)5) != 5) {
		return false;
	}
	for (uint8_t i = 0; i < 5; ++i) {
		data[i] = bus.read();
	}
	return true;
}

/** @brief Reset and detect CST816 or FT6X36 touch controllers. */
void touch_init() {
	pinMode(TP_INT, INPUT_PULLUP);
	pinMode(TP_RST, OUTPUT);
	digitalWrite(TP_RST, LOW);
	delay(10);
	digitalWrite(TP_RST, HIGH);
	delay(50);
	if (!i2c_bus_begin(TP_SDA, TP_SCL, 400000)) {
		Serial.println("Touch I2C initialization failed");
		return;
	}
	TwoWire &bus = i2c_bus();
	for (uint8_t address : {0x15, 0x38}) {
		bus.beginTransmission(address);
		if (bus.endTransmission() == 0) {
			s_address = address;
			break;
		}
	}
	if (s_address == 0) {
		Serial.println("Touch controller not found");
		return;
	}
	if (s_address == 0x15) {
		bus.beginTransmission(s_address);
		bus.write((uint8_t)0xFE);  // CST816: disable automatic sleep
		bus.write((uint8_t)0x01);
		if (bus.endTransmission() != 0) {
			Serial.println("Touch automatic-sleep configuration failed");
		}
	}
	Serial.printf("Touch controller at 0x%02X\n", s_address);
}

/** @brief Select the coordinate mapping for the current mounting orientation. @param flipped Invert X/Y. */
void touch_set_flipped(bool flipped) { s_flipped = flipped; }

/** @brief Poll one contact at most every 10 ms and recognize horizontal swipes. */
int8_t touch_poll(uint32_t now) {
	if (s_address == 0 || (uint32_t)(now - s_lastPoll) < 10) {
		return 0;
	}
	s_lastPoll = now;
	uint8_t data[5];
	if (!read_contact(data)) {
		s_pressed = false;
		s_actions.cancel();
		s_swipe.update(false, 0, 0, now);
		return 0;
	}
	uint8_t count = data[0] & 0x0F;
	bool touching = count == 1 && (data[1] >> 6) != 1;
	int16_t x = ((data[1] & 0x0F) << 8) | data[2];
	int16_t y = ((data[3] & 0x0F) << 8) | data[4];
	if (touching && (x >= LCD_WIDTH || y >= LCD_HEIGHT)) {
		s_pressed = false;
		s_actions.cancel();
		s_swipe.update(false, 0, 0, now);
		return 0;
	}
	if (s_flipped) {
		x = int16_t(LCD_WIDTH - 1 - x);
		y = int16_t(LCD_HEIGHT - 1 - y);
	}
	s_pressed = touching;
	s_actions.update(touching, x, y, now);
	if (s_actions.vertical) {
		s_swipe.update(false, 0, 0, now);
		return 0;
	}
	return s_swipe.update(touching, x, y, now);
}

int8_t touch_take_scroll() {
	int8_t scroll = s_actions.scroll;
	s_actions.scroll = 0;
	return scroll;
}

/** @brief Return the most recent valid touch state. @return Finger contact state. */
bool touch_pressed() { return s_pressed; }

/** @brief Consume a short tap. @param x Output X. @param y Output Y. @return Pending tap. */
bool touch_take_tap(int16_t &x, int16_t &y) {
	if (!s_actions.tap) { return false; }
	s_actions.tap = false;
	x = s_actions.x; y = s_actions.y;
	return true;
}

/** @brief Consume a double tap. @return Pending double tap. */
bool touch_take_double() { return s_actions.take_double(); }

/** @brief Consume a stationary hold. @return Pending hold. */
bool touch_take_hold() {
	bool hold = s_actions.hold;
	s_actions.hold = false;
	return hold;
}

/** @brief Consume a vertical petting stroke. @return Pending pet. */
bool touch_take_pet() { return s_actions.take_pet(); }

/** @brief Consume a vertical petting stroke with the contact position. @param x Output X. @param y Output Y. @return Pending pet. */
bool touch_take_pet(int16_t &x, int16_t &y) { return s_actions.take_pet(x, y); }
