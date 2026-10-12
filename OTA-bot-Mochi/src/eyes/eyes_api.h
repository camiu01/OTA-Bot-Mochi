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
 * @file eyes_api.h
 * @brief Public API for bloub/Mochi robot eyes.
 *
 * Init, style, expression, colors, per-frame update/draw and IMU hooks.
 */

#ifndef EYES_API_H
#define EYES_API_H

#include <Arduino_GFX_Library.h>
#include <stdint.h>
#include "eyes_types.h"

/**
 * @brief Bind display and reset config/state to defaults.
 * @param display Active Arduino_GFX instance.
 */
void eyes_init(Arduino_GFX *display);

/**
 * @brief Change rendering style.
 * @param style One of NORMAL, MOCHI, BLOUB, TRAPEZ.
 */
void eyes_set_style(EyeStyle style);

/**
 * @brief Change emotional expression.
 * @param expr One of NEUTRAL, HAPPY, SAD, ANGRY, SURPRISED, SLEEPY, LOVE, DIZZY, GLEE, WORRIED, FOCUSED, ANNOYED, SKEPTIC, FRUSTRATED, UNIMPRESSED, SUSPICIOUS, SQUINT, FURIOUS, SCARED, AWE, SLEEPY_EYES, SAD_DOWN, SAD_UP.
 */
void eyes_set_expression(Expression expr);

/**
 * @brief Set TRAPEZ corner-lift slant in pixels.
 * @param slant Corner-lift offset, positive pulls high-left corner up.
 */
void eyes_set_slant(int8_t slant);

/**
 * @brief Set eyelid line offset in pixels.
 * @param lid Lid offset, negative raises lid, positive lowers it.
 */
void eyes_set_lid(int16_t lid);

/**
 * @brief Enable or disable per-eye asymmetric geometry.
 * @param asym 0 symmetric, 1 per-eye vary.
 */
void eyes_set_asym(uint8_t asym);

/**
 * @brief Set expression by case-insensitive name string.
 * @param name One of neutral, happy, glee, worried, focused, annoyed, surprised, skeptic, frustrated, unimpressed, sleepy, suspicious, squint, angry, furious, scared, awe, love, dizzy, sad, sad_down, sad_up, sleepy_eyes.
 * @return True when name matched, false otherwise.
 */
bool eyes_set_expression_by_name(const char *name);

/**
 * @brief Override background and foreground colors.
 * @param bg Background color (default BLACK).
 * @param fg Eye body color (default WHITE).
 */
void eyes_set_colors(uint16_t bg, uint16_t fg);

/**
 * @brief Override full eye palette in one call.
 * @param bg Background color (RGB565).
 * @param fg Eye body color (RGB565).
 * @param pupil Pupil/overlay color (RGB565).
 * @param highlight Specular highlight color (RGB565).
 */
void eyes_set_eye_colors(uint16_t bg, uint16_t fg, uint16_t pupil, uint16_t highlight);

/**
 * @brief Set background clear color only.
 * @param bg Background color (RGB565).
 */
void eyes_set_bg(uint16_t bg);

/**
 * @brief Set eye body color only.
 * @param fg Eye body color (RGB565).
 */
void eyes_set_fg(uint16_t fg);

/**
 * @brief Set pupil/overlay color only.
 * @param c Pupil color (RGB565).
 */
void eyes_set_pupil(uint16_t c);

/**
 * @brief Set specular highlight color only.
 * @param c Highlight color (RGB565).
 */
void eyes_set_highlight(uint16_t c);

/**
 * @brief Get background clear color.
 * @return Background color (RGB565).
 */
uint16_t eyes_get_bg(void);

/**
 * @brief Get eye body color.
 * @return Eye body color (RGB565).
 */
uint16_t eyes_get_fg(void);

/**
 * @brief Get pupil/overlay color.
 * @return Pupil color (RGB565).
 */
uint16_t eyes_get_pupil(void);

/**
 * @brief Get specular highlight color.
 * @return Highlight color (RGB565).
 */
uint16_t eyes_get_highlight(void);

/**
 * @brief Get full eye palette in one call.
 * @param bg Output background color, nullable.
 * @param fg Output eye body color, nullable.
 * @param pupil Output pupil color, nullable.
 * @param highlight Output highlight color, nullable.
 */
void eyes_get_colors(uint16_t *bg, uint16_t *fg, uint16_t *pupil, uint16_t *highlight);

/**
 * @brief Advance animation timers and redraw.
 * @param nowMs Value of millis() at call time.
 */
void eyes_update(uint32_t nowMs);

/**
 * @brief Redraw eyes from current config/state.
 */
void eyes_draw();

/**
 * @brief Access config singleton.
 * @return Reference to global EyeConfig.
 */
EyeConfig &eyes_config();

/**
 * @brief Access state singleton.
 * @return Reference to global EyeState.
 */
EyeState &eyes_state();

/**
 * @brief Return bound display.
 * @return Display pointer or nullptr.
 */
Arduino_GFX *eyes_gfx();

/**
 * @brief Init QMI8658C on QMI_SDA/QMI_SCL.
 */
void eyes_imu_init();

/**
 * @brief Poll accelerometer, drive tilt and shake-to-dizzy.
 * @param nowMs Value of millis() at call time.
 */
void eyes_imu_poll(uint32_t nowMs);

/**
 * @brief Set pupil offset from tilt in pixels.
 * @param tx Horizontal offset.
 * @param ty Vertical offset.
 */
void eyes_imu_set_tilt(int16_t tx, int16_t ty);

/**
 * @brief Force DIZZY expression for 2s.
 * @param nowMs Value of millis() at call time.
 */
void eyes_imu_trigger_shake(uint32_t nowMs);

#endif
