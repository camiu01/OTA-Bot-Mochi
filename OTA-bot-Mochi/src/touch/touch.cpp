/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "touch.h"
#include "swipe.h"
#include "../../pin_config.h"
#include <Arduino.h>
#include <Wire.h>

static TwoWire s_touchBus(1);
static SwipeTracker s_swipe;
static uint8_t s_address = 0;
static uint32_t s_lastPoll = 0;

/**
 * @brief Read the contact count and first contact coordinates.
 * @param data Output five-byte register block.
 * @return True only when the complete block was received.
 */
static bool read_contact(uint8_t *data) {
	s_touchBus.beginTransmission(s_address);
	s_touchBus.write((uint8_t)0x02);
	if (s_touchBus.endTransmission(false) != 0) {
		return false;
	}
	if (s_touchBus.requestFrom(s_address, (uint8_t)5) != 5) {
		return false;
	}
	for (uint8_t i = 0; i < 5; ++i) {
		data[i] = s_touchBus.read();
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
	if (!s_touchBus.begin(TP_SDA, TP_SCL, 400000)) {
		Serial.println("Touch I2C initialization failed");
		return;
	}
	s_touchBus.setTimeOut(5);
	for (uint8_t address : {0x15, 0x38}) {
		s_touchBus.beginTransmission(address);
		if (s_touchBus.endTransmission() == 0) {
			s_address = address;
			break;
		}
	}
	if (s_address == 0) {
		Serial.println("Touch controller not found");
		return;
	}
	if (s_address == 0x15) {
		s_touchBus.beginTransmission(s_address);
		s_touchBus.write((uint8_t)0xFE);  // CST816: disable automatic sleep
		s_touchBus.write((uint8_t)0x01);
		if (s_touchBus.endTransmission() != 0) {
			Serial.println("Touch automatic-sleep configuration failed");
		}
	}
	Serial.printf("Touch controller at 0x%02X\n", s_address);
}

/** @brief Poll one contact at most every 10 ms and recognize horizontal swipes. */
int8_t touch_poll(uint32_t now) {
	if (s_address == 0 || (uint32_t)(now - s_lastPoll) < 10) {
		return 0;
	}
	s_lastPoll = now;
	uint8_t data[5];
	if (!read_contact(data)) {
		s_swipe.update(false, 0, 0, now);
		return 0;
	}
	uint8_t count = data[0] & 0x0F;
	bool touching = count == 1 && (data[1] >> 6) != 1;
	int16_t x = ((data[1] & 0x0F) << 8) | data[2];
	int16_t y = ((data[3] & 0x0F) << 8) | data[4];
	if (touching && (x >= LCD_WIDTH || y >= LCD_HEIGHT)) {
		s_swipe.update(false, 0, 0, now);
		return 0;
	}
	return s_swipe.update(touching, x, y, now);
}
