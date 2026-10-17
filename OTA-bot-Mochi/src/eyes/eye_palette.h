/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * @file eye_palette.h
 * @brief Fixed semantic accent colours for decorated eye expressions.
 *
 * @purpose Give expressions whose decoration reads by colour a palette that is
 * independent of the user's eye colours: blue tears, a rose heart, an orange
 * flame, and so on.
 */

#ifndef EYE_PALETTE_H
#define EYE_PALETTE_H

#include <stdint.h>
#include "eyes_types.h"

/**
 * @brief Pack 8-bit channels into an RGB565 pixel.
 * @param r Red 0..255. @param g Green 0..255. @param b Blue 0..255.
 * @return RGB565 value.
 */
constexpr uint16_t eye_rgb(uint8_t r, uint8_t g, uint8_t b) {
	return uint16_t(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

/**
 * @brief Fixed accent for an expression whose decoration reads by colour.
 *
 * The accent replaces the configured ink only for that expression's glyph, and
 * the petting heart marker always takes the LOVE accent, so every other face
 * still follows the user's eye colours. Accents are plain constants with no
 * settings and no persistence; none of them equals black, white, or the pure
 * red and green values the native suites count as pupil and highlight colours,
 * which keeps the solid-style exclusion proofs honest. An accent equal to the
 * configured body colour would hide that mark, exactly like an engraved glyph
 * whose background matches the body.
 *
 * @param expr Expression value.
 * @return RGB565 accent, or 0 when the expression keeps the configured colours.
 */
constexpr uint16_t eye_accent(Expression expr) {
	switch (expr) {
		case CRYING: return eye_rgb(77, 190, 255);   /**< Sky-blue tear. */
		case LOVE: return eye_rgb(255, 92, 130);     /**< Rose heart. */
		case FIRE: return eye_rgb(255, 140, 48);     /**< Orange flame. */
		case STAR: return eye_rgb(255, 235, 80);     /**< Golden star. */
		case HALO: return eye_rgb(255, 206, 88);     /**< Halo gold. */
		case DEVIL: return eye_rgb(230, 55, 55);     /**< Horn red. */
		case MOON: return eye_rgb(255, 190, 60);     /**< Amber moon. */
		case SPARKLE: return eye_rgb(140, 232, 255); /**< Ice-cyan sparkle. */
		case BOLT: return eye_rgb(255, 242, 110);    /**< Electric yellow. */
		case DIAMOND: return eye_rgb(90, 227, 240);  /**< Aqua stone. */
		case MONEY: return eye_rgb(97, 224, 138);    /**< Coin green. */
		case LOW_BATT: return eye_rgb(255, 84, 74);  /**< Battery red. */
		case IDEA: return eye_rgb(255, 202, 86);     /**< Idea amber. */
		case MUSIC: return eye_rgb(186, 130, 250);   /**< Note violet. */
		default: return 0;
	}
}

#endif
