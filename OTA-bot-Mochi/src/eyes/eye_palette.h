/**
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 *
 * @file eye_palette.h
 * @brief RGB565 packing for explicit animated scene effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef EYE_PALETTE_H
#define EYE_PALETTE_H

#include <stdint.h>

/**
 * @brief Pack 8-bit channels into an RGB565 pixel.
 * @param r Red 0..255. @param g Green 0..255. @param b Blue 0..255.
 * @return RGB565 value.
 */
constexpr uint16_t eye_rgb(uint8_t r, uint8_t g, uint8_t b) {
	return uint16_t(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

#endif
