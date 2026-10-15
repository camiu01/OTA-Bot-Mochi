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
 * @file eyes_anim.cpp
 * @brief Eye animation timers and public state API.
 */
#include "eyes_api.h"
#include "eyes_faces.h"
#include "eye_life.h"
#include "../settings/settings.h"
#include "../../pin_config.h"
#include <Arduino.h>
#include <math.h>

static Arduino_GFX *s_gfx = nullptr;
static EyeConfig s_cfg;
static EyeState s_state;

/**
 * @brief Schedule the next random saccade.
 * @param now Current timestamp in milliseconds.
 * @param minMs Minimum delay before the next saccade.
 * @param spanMs Random additional delay span.
 */
static void anim_arm_saccade(uint32_t now, uint32_t minMs, uint32_t spanMs) {
	s_state.nextSaccadeMs = now + minMs + (uint32_t)(random((long)spanMs));
}

/**
 * @brief Schedule the next blink.
 * @param now Current timestamp in milliseconds.
 * @param minMs Minimum delay before the next blink.
 * @param spanMs Random additional delay span.
 */
static void anim_arm_blink(uint32_t now, uint32_t minMs, uint32_t spanMs) {
	s_state.nextBlinkMs = now + minMs + (uint32_t)(random((long)spanMs));
}

/**
 * @brief Advance idle bobbing phase and vertical offset.
 * @param dtMs Elapsed time since last update, clamped by caller.
 */
static void anim_step_float(uint32_t dtMs) {
	const float speed = settings_get().dashboard ? 0.002f : 0.004f;
	s_state.floatPhase += (float)dtMs * speed;
	if (s_state.floatPhase > 6.2831853f) {
		s_state.floatPhase -= 6.2831853f;
	}
	float amplitude = settings_get().dashboard ? 1.5f : 4.0f;
	s_state.floatY = (int16_t)(sinf(s_state.floatPhase) * amplitude);
}

/**
 * @brief Trigger a random gaze jump when due, holding a tap-directed glance instead.
 * @param now Current timestamp in milliseconds.
 */
static void anim_step_saccade(uint32_t now) {
	if (s_state.glanceUntilMs != 0) {
		if ((int32_t)(now - s_state.glanceUntilMs) < 0) { return; }
		s_state.glanceUntilMs = 0;
	}
	if ((int32_t)(now - s_state.nextSaccadeMs) < 0) {
		return;
	}
	bool calm = settings_get().dashboard;
	s_state.saccadeX = (int16_t)(random(calm ? -6 : -12, calm ? 7 : 13));
	s_state.saccadeY = (int16_t)(random(calm ? -3 : -8, calm ? 4 : 9));
	anim_arm_saccade(now, calm ? 2500 : 1500, 2500);
}

/**
 * @brief Open or close eyelids according to blink schedule.
 * @param now Current timestamp in milliseconds.
 */
static void anim_step_blink(uint32_t now) {
	if (!s_state.blinking) {
		if ((int32_t)(now - s_state.nextBlinkMs) >= 0) {
			s_state.blinking = true;
			s_state.blinkStartMs = now;
			s_state.blinkDurMs = 160 + (uint32_t)(random(80));
		}
		return;
	}
	if ((uint32_t)(now - s_state.blinkStartMs) >= s_state.blinkDurMs) {
		s_state.blinking = false;
		anim_arm_blink(now, 2000, 4000);
	}
}

/**
 * @brief Expire the DIZZY override and restore saved expression.
 * @param now Current timestamp in milliseconds.
 */
static void anim_step_dizzy(uint32_t now) {
	if (s_state.dizzyUntilMs == 0) {
		return;
	}
	if ((int32_t)(now - s_state.dizzyUntilMs) >= 0) {
		s_state.dizzyUntilMs = 0;
		s_cfg.expression = s_state.savedExpr;
	}
}

/**
 * @brief Bind display and reset config/state to defaults.
 * @param display Active Arduino_GFX instance.
 */
void eyes_init(Arduino_GFX *display) {
	s_gfx = display;
	s_cfg.screenW = LCD_WIDTH;
	s_cfg.screenH = LCD_HEIGHT;
	if (display) {
		s_cfg.screenW = uint16_t(display->width());
		s_cfg.screenH = uint16_t(display->height());
	}
	eyes_life_reset();
	s_cfg.style = BLOUB;
	s_cfg.expression = NEUTRAL;
	eyes_faces_base(NEUTRAL);
	s_cfg.bgColor = RGB565_BLACK;
	s_cfg.fgColor = RGB565_WHITE;
	s_cfg.pupilColor = RGB565_BLACK;
	s_cfg.highlightColor = RGB565_WHITE;
	eyes_set_style(BLOUB);
	s_cfg.lidTop = 0;
	s_state.saccadeX = 0;
	s_state.saccadeY = 0;
	s_state.tiltX = 0;
	s_state.tiltY = 0;
	s_state.floatY = 0;
	s_state.floatPhase = 0.0f;
	s_state.lastMs = millis();
	s_state.blinking = false;
	s_state.blinkDurMs = 150;
	s_state.dizzyUntilMs = 0;
	s_state.glanceUntilMs = 0;
	s_state.heartUntilMs = 0;
	s_state.savedExpr = NEUTRAL;
	s_state.dirtyInit = false;
	anim_arm_saccade(s_state.lastMs, 1500, 1000);
	anim_arm_blink(s_state.lastMs, 2000, 1500);
	if (s_gfx != nullptr) {
		s_gfx->fillScreen(s_cfg.bgColor);
	}
}

/**
 * @brief Change rendering style.
 * @param style One of NORMAL, MOCHI, BLOUB, TRAPEZ.
 */
void eyes_set_style(EyeStyle style) {
	s_cfg.style = style;
	s_cfg.eyeGap = 28;
	s_cfg.asym = 1;
	if (style == NORMAL) {
		s_cfg.eyeW = 56;
		s_cfg.eyeH = 84;
		s_cfg.radius = 0;
		s_cfg.slant = 0;
		s_cfg.asym = 0;
	} else if (style == MOCHI) {
		s_cfg.eyeW = 44;
		s_cfg.eyeH = 100;
		s_cfg.eyeGap = 42;
		s_cfg.radius = 22;
		s_cfg.slant = 0;
		s_cfg.asym = 0;
	} else if (style == TRAPEZ) {
		s_cfg.eyeW = 68;
		s_cfg.eyeH = 76;
		s_cfg.radius = 0;
		s_cfg.slant = 10;
		s_cfg.asym = 0;
	} else {
		s_cfg.eyeW = 76;
		s_cfg.eyeH = 60;
		s_cfg.radius = 24;
		s_cfg.slant = 0;
		s_cfg.asym = 1;
	}
}

/**
 * @brief Set the mirrored TRAPEZ top slope.
 * @param slant Signed slope offset, bounded by the renderer.
 */
void eyes_set_slant(int8_t slant) {
	s_cfg.slant = slant;
}

/**
 * @brief Set eyelid line offset in pixels.
 * @param lid Lid offset, negative raises lid, positive lowers it.
 */
void eyes_set_lid(int16_t lid) {
	s_cfg.lidTop = lid;
}

/**
 * @brief Enable or disable per-eye asymmetric geometry.
 * @param asym 0 symmetric, 1 per-eye vary.
 */
void eyes_set_asym(uint8_t asym) {
	s_cfg.asym = (uint8_t)(asym ? 1 : 0);
}

/**
 * @brief Override background and foreground colors.
 * @param bg Background color (default BLACK).
 * @param fg Eye body color (default WHITE).
 */
void eyes_set_colors(uint16_t bg, uint16_t fg) {
	s_cfg.bgColor = bg;
	s_cfg.fgColor = fg;
}

/**
 * @brief Override full eye palette in one call.
 * @param bg Background color (RGB565).
 * @param fg Eye body color (RGB565).
 * @param pupil Pupil/overlay color (RGB565).
 * @param highlight Specular highlight color (RGB565).
 */
void eyes_set_eye_colors(uint16_t bg, uint16_t fg, uint16_t pupil, uint16_t highlight) {
	s_cfg.bgColor = bg;
	s_cfg.fgColor = fg;
	s_cfg.pupilColor = pupil;
	s_cfg.highlightColor = highlight;
}

/**
 * @brief Set background clear color only.
 * @param bg Background color (RGB565).
 */
void eyes_set_bg(uint16_t bg) {
	s_cfg.bgColor = bg;
}

/**
 * @brief Set eye body color only.
 * @param fg Eye body color (RGB565).
 */
void eyes_set_fg(uint16_t fg) {
	s_cfg.fgColor = fg;
}

/**
 * @brief Set pupil/overlay color only.
 * @param c Pupil color (RGB565).
 */
void eyes_set_pupil(uint16_t c) {
	s_cfg.pupilColor = c;
}

/**
 * @brief Set specular highlight color only.
 * @param c Highlight color (RGB565).
 */
void eyes_set_highlight(uint16_t c) {
	s_cfg.highlightColor = c;
}

/**
 * @brief Get background clear color.
 * @return Background color (RGB565).
 */
uint16_t eyes_get_bg(void) {
	return s_cfg.bgColor;
}

/**
 * @brief Get eye body color.
 * @return Eye body color (RGB565).
 */
uint16_t eyes_get_fg(void) {
	return s_cfg.fgColor;
}

/**
 * @brief Get pupil/overlay color.
 * @return Pupil color (RGB565).
 */
uint16_t eyes_get_pupil(void) {
	return s_cfg.pupilColor;
}

/**
 * @brief Get specular highlight color.
 * @return Highlight color (RGB565).
 */
uint16_t eyes_get_highlight(void) {
	return s_cfg.highlightColor;
}

/**
 * @brief Get full eye palette in one call.
 * @param bg Output background color, nullable.
 * @param fg Output eye body color, nullable.
 * @param pupil Output pupil color, nullable.
 * @param highlight Output highlight color, nullable.
 */
void eyes_get_colors(uint16_t *bg, uint16_t *fg, uint16_t *pupil, uint16_t *highlight) {
	if (bg != nullptr) {
		*bg = s_cfg.bgColor;
	}
	if (fg != nullptr) {
		*fg = s_cfg.fgColor;
	}
	if (pupil != nullptr) {
		*pupil = s_cfg.pupilColor;
	}
	if (highlight != nullptr) {
		*highlight = s_cfg.highlightColor;
	}
}

/**
 * @brief Access config singleton.
 * @return Reference to global EyeConfig.
 */
EyeConfig &eyes_config() {
	return s_cfg;
}

/**
 * @brief Access state singleton.
 * @return Reference to global EyeState.
 */
EyeState &eyes_state() {
	return s_state;
}

/**
 * @brief Hold the gaze toward a tapped screen position for a bounded time.
 * @param x Tap X in logical screen coordinates.
 * @param y Tap Y in logical screen coordinates.
 * @param nowMs Value of millis() at call time.
 */
void eyes_glance(int16_t x, int16_t y, uint32_t nowMs) {
	int32_t gx = (int32_t(x) - LCD_WIDTH / 2) * 12 / (LCD_WIDTH / 2);
	int32_t gy = (int32_t(y) - LCD_HEIGHT / 2) * 8 / (LCD_HEIGHT / 2);
	s_state.saccadeX = int16_t(gx);
	s_state.saccadeY = int16_t(gy);
	s_state.glanceUntilMs = nowMs + 2500;
}

/**
 * @brief Return bound display.
 * @return Display pointer or nullptr.
 */
Arduino_GFX *eyes_gfx() {
	return s_gfx;
}

/**
 * @brief Advance animation timers and redraw.
 * @param nowMs Value of millis() at call time.
 */
void eyes_update(uint32_t nowMs) {
	uint32_t dt = nowMs - s_state.lastMs;
	if (dt > 100) {
		dt = 100;
	}
	s_state.lastMs = nowMs;
	anim_step_dizzy(nowMs);
	anim_step_float(dt);
	anim_step_saccade(nowMs);
	anim_step_blink(nowMs);
	eyes_imu_poll(nowMs);
	eyes_faces_step(nowMs);
	eyes_life_step(nowMs, dt);
	if (s_state.heartUntilMs != 0 && (int32_t)(nowMs - s_state.heartUntilMs) >= 0) {
		s_state.heartUntilMs = 0;
	}
	eyes_draw();
}
