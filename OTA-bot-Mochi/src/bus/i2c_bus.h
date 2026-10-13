/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * @file i2c_bus.h
 * @brief Single I2C controller shared by the touch controller and the IMU.
 *
 * @details The board routes touch, RTC and QMI8658C to the same SDA/SCL pads.
 *          The ESP32 GPIO matrix accepts one peripheral output per pad, so every
 *          driver must use this one instance instead of opening its own bus.
 */
#pragma once

#include <Wire.h>
#include <stdint.h>

/**
 * @brief Start the shared bus once; later calls keep the first configuration.
 * @param sda Data pin.
 * @param scl Clock pin.
 * @param hz Bus frequency in hertz.
 * @return True when the bus is usable.
 */
bool i2c_bus_begin(uint8_t sda, uint8_t scl, uint32_t hz);

/** @brief Access the shared bus. @return Bus instance, valid after i2c_bus_begin. */
TwoWire &i2c_bus();
