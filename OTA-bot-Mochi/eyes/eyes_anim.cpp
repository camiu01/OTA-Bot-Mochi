/**
 * @file eyes_anim.cpp
 * @brief Eye animation timers and public state API.
 */
#include "eyes_api.h"
#include <Arduino.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

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
	const float speed = 0.004f;
	s_state.floatPhase += (float)dtMs * speed;
	if (s_state.floatPhase > 6.2831853f) {
		s_state.floatPhase -= 6.2831853f;
	}
	s_state.floatY = (int16_t)(sinf(s_state.floatPhase) * 4.0f);
}

/**
 * @brief Trigger a random gaze jump when due.
 * @param now Current timestamp in milliseconds.
 */
static void anim_step_saccade(uint32_t now) {
	if ((int32_t)(now - s_state.nextSaccadeMs) < 0) {
		return;
	}
	s_state.saccadeX = (int16_t)(random(-12, 13));
	s_state.saccadeY = (int16_t)(random(-8, 9));
	anim_arm_saccade(now, 1500, 2500);
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
			s_state.blinkDurMs = 120 + (uint32_t)(random(80));
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
	s_cfg.style = BLOUB;
	s_cfg.expression = NEUTRAL;
	s_cfg.bgColor = BLACK;
	s_cfg.fgColor = WHITE;
	s_cfg.pupilColor = BLACK;
	s_cfg.highlightColor = WHITE;
	s_cfg.eyeW = 64;
	s_cfg.eyeH = 96;
	s_cfg.eyeGap = 28;
	s_cfg.radius = 28;
	s_cfg.slant = 0;
	s_cfg.asym = 1;
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
	if (style == NORMAL) {
		s_cfg.eyeW = 56;
		s_cfg.eyeH = 84;
		s_cfg.radius = 10;
		s_cfg.slant = 0;
		s_cfg.asym = 0;
	} else if (style == MOCHI) {
		s_cfg.eyeW = 62;
		s_cfg.eyeH = 92;
		s_cfg.radius = 46;
		s_cfg.slant = 0;
		s_cfg.asym = 0;
	} else if (style == TRAPEZ) {
		s_cfg.eyeW = 66;
		s_cfg.eyeH = 88;
		s_cfg.radius = 14;
		s_cfg.slant = 14;
		s_cfg.asym = 0;
	} else {
		s_cfg.eyeW = 64;
		s_cfg.eyeH = 96;
		s_cfg.radius = 28;
		s_cfg.slant = 0;
		s_cfg.asym = 1;
	}
}

/**
 * @brief Change emotional expression.
 * @param expr One of NEUTRAL, HAPPY, SAD, ANGRY, SURPRISED, SLEEPY, LOVE, DIZZY, GLEE, WORRIED, FOCUSED, ANNOYED, SKEPTIC, FRUSTRATED, UNIMPRESSED, SUSPICIOUS, SQUINT, FURIOUS, SCARED, AWE, SLEEPY_EYES, SAD_DOWN, SAD_UP.
 */
void eyes_set_expression(Expression expr) {
	if (s_state.dizzyUntilMs != 0) {
		s_state.savedExpr = expr;
		return;
	}
	s_cfg.expression = expr;
}

/**
 * @brief Compare two names case-insensitively.
 * @param input User supplied string, nullable.
 * @param cand Lowercase candidate literal.
 * @return True when strings match ignoring case.
 */
static bool match_name(const char *input, const char *cand) {
	if (input == nullptr || cand == nullptr) {
		return false;
	}
	while (*cand != '\0') {
		if (tolower((unsigned char)*input) != *cand) {
			return false;
		}
		input++;
		cand++;
	}
	return *input == '\0';
}

/**
 * @brief Map a normalized name to its Expression value.
 * @param name Lowercase-compared input string.
 * @param out Output expression on match.
 * @return True when name matched, false otherwise.
 */
static bool lookup_expression(const char *name, Expression *out) {
	if (match_name(name, "neutral")) { *out = NEUTRAL; return true; }
	if (match_name(name, "happy")) { *out = HAPPY; return true; }
	if (match_name(name, "glee")) { *out = GLEE; return true; }
	if (match_name(name, "worried")) { *out = WORRIED; return true; }
	if (match_name(name, "focused")) { *out = FOCUSED; return true; }
	if (match_name(name, "annoyed")) { *out = ANNOYED; return true; }
	if (match_name(name, "surprised")) { *out = SURPRISED; return true; }
	if (match_name(name, "skeptic")) { *out = SKEPTIC; return true; }
	if (match_name(name, "frustrated")) { *out = FRUSTRATED; return true; }
	if (match_name(name, "unimpressed")) { *out = UNIMPRESSED; return true; }
	if (match_name(name, "sleepy")) { *out = SLEEPY; return true; }
	if (match_name(name, "sleepy_eyes")) { *out = SLEEPY_EYES; return true; }
	if (match_name(name, "suspicious")) { *out = SUSPICIOUS; return true; }
	if (match_name(name, "squint")) { *out = SQUINT; return true; }
	if (match_name(name, "angry")) { *out = ANGRY; return true; }
	if (match_name(name, "furious")) { *out = FURIOUS; return true; }
	if (match_name(name, "scared")) { *out = SCARED; return true; }
	if (match_name(name, "awe")) { *out = AWE; return true; }
	if (match_name(name, "love")) { *out = LOVE; return true; }
	if (match_name(name, "dizzy")) { *out = DIZZY; return true; }
	if (match_name(name, "sad")) { *out = SAD; return true; }
	if (match_name(name, "sad_down") || match_name(name, "saddown")) { *out = SAD_DOWN; return true; }
	if (match_name(name, "sad_up") || match_name(name, "sadup")) { *out = SAD_UP; return true; }
	return false;
}

/**
 * @brief Set expression by case-insensitive name string.
 * @param name One of neutral, happy, glee, worried, focused, annoyed, surprised, skeptic, frustrated, unimpressed, sleepy, suspicious, squint, angry, furious, scared, awe, love, dizzy, sad, sad_down, sad_up, sleepy_eyes.
 * @return True when name matched, false otherwise.
 */
bool eyes_set_expression_by_name(const char *name) {
	Expression e = NEUTRAL;
	if (name == nullptr || !lookup_expression(name, &e)) {
		return false;
	}
	eyes_set_expression(e);
	return true;
}

/**
 * @brief Set TRAPEZ corner-lift slant in pixels.
 * @param slant Corner-lift offset, positive pulls high-left corner up.
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
	eyes_draw();
}
