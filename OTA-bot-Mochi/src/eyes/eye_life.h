/**
 * @file eye_life.h
 * @brief Soft blink-covered face transitions, damped gaze, and six bounded idle gestures.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"
#include <math.h>

struct EyeLifeFrame {
	bool active = false;
	Expression expression = NEUTRAL;
	float openL = 1, openR = 1, scaleX = 1, scaleY = 1;
	int16_t x = 0, y = 0;
};

class EyeLife {
	EyeLifeFrame frame;
	Expression target = NEUTRAL;
	EyeStyle style = NORMAL;
	bool changing = false, gesturing = false;
	uint32_t change_at = 0, gesture_at = 0, idle_at = 0, idle_delay = 18000;
	uint8_t gesture = 0;
	float fromL = 1, fromR = 1, gazeX = 0, gazeY = 0;
	int8_t sign = 1;
	/** @brief Smoothstep in a bounded interval. @param p Progress. @return Eased. */
	static float ease(float p) { p = p < 0 ? 0 : (p > 1 ? 1 : p); return p * p * (3 - 2 * p); }
	/** @brief Close, hold briefly, then reopen over a face handoff. @param now Time. @param amount Intensity. */
	void transition(uint32_t now, float amount) {
		float p = float(uint32_t(now - change_at)) / 320;
		if (p >= 1) { changing = false; frame.expression = target; return; }
		if (p < 0.35f) {
			float close = ease(p / 0.35f);
			frame.openL = fromL * (1 - close); frame.openR = fromR * (1 - close);
		} else {
			frame.expression = target;
			frame.openL = frame.openR = ease((p - 0.45f) / 0.55f);
		}
		float spring = sinf(3.14159265f * p);
		frame.scaleX += spring * 0.06f * amount;
		frame.scaleY -= spring * 0.06f * amount;
	}
	/** @brief Apply one short decorative gesture, never change base/style. @param now Time. @param amount Intensity. */
	void pose(uint32_t now, float amount) {
		float p = float(uint32_t(now - gesture_at)) / 900;
		if (p >= 1) { gesturing = false; return; }
		float wave = sinf(3.14159265f * p);
		switch (gesture) {
			case 0: {
				float blink = p < 0.3f ? sinf(3.14159265f * p / 0.3f) :
					(p > 0.45f && p < 0.75f ? sinf(3.14159265f * (p - 0.45f) / 0.3f) : 0);
				frame.openL *= 1 - blink; frame.openR *= 1 - blink; break;
			}
			case 1: if (sign > 0) { frame.openL *= 1 - wave; } else { frame.openR *= 1 - wave; } break;
			case 2: frame.x += int16_t(wave * 6 * sign * amount); break;
			case 3: frame.openL *= 1 - wave * 0.25f; frame.y -= int16_t(wave * 3 * amount); break;
			case 4: frame.scaleX -= wave * 0.07f * amount; frame.scaleY += wave * 0.12f * amount; frame.y -= int16_t(wave * 4 * amount); break;
			case 5: frame.y += int16_t(sinf(p * 6.2831853f) * 4 * amount); break;
		}
	}
public:
	/** @brief Reset on initialization. */
	void reset() { *this = EyeLife{}; }
	/** @brief Access a read-only visual pose. @return Frame. */
	const EyeLifeFrame &get() const { return frame; }
	/** @brief Advance without drawing or I/O. @param cfg Target. @param st Raw gaze/blink. @param now Time. @param dt Bounded delta. @param mode Calm/Dasai/playful. @param amount Intensity. @param choice Random input. */
	void step(const EyeConfig &cfg, const EyeState &st, uint32_t now, uint32_t dt,
		uint8_t mode, float amount, uint32_t choice) {
		if (!frame.active || style != cfg.style) {
			reset(); frame.active = true; style = cfg.style; frame.expression = target = cfg.expression; idle_at = now;
		}
		if (cfg.expression != target) {
			fromL = frame.openL; fromR = frame.openR; change_at = now;
			target = cfg.expression; changing = true; gesturing = false; idle_at = now;
		}
		frame.openL = frame.openR = frame.scaleX = frame.scaleY = 1;
		float alpha = float(dt > 100 ? 100 : dt) / (80 + float(dt > 100 ? 100 : dt));
		gazeX += (st.saccadeX + st.tiltX - gazeX) * alpha;
		gazeY += (st.saccadeY + st.tiltY + st.floatY - gazeY) * alpha;
		frame.x = int16_t(gazeX); frame.y = int16_t(gazeY);
		if (changing) { transition(now, mode ? amount : 0); }
		if (st.blinking) {
			float p = float(uint32_t(now - st.blinkStartMs)) / (st.blinkDurMs ? st.blinkDurMs : 1);
			float open = p < 1 ? 1 - sinf(3.14159265f * p) : 1;
			frame.openL *= open; frame.openR *= open;
		}
		if (!mode) { gesturing = false; idle_at = now; }
		if (mode && !changing && !gesturing && uint32_t(now - idle_at) >= idle_delay) {
			gesture = (gesture + 1 + choice % 5) % 6;
			sign = choice & 1 ? 1 : -1; gesture_at = idle_at = now; gesturing = true;
			idle_delay = (mode == 2 ? 12000 : 18000) + choice % 12001 + (amount < 0.8f ? 8000 : 0);
		}
		if (gesturing && !changing) { pose(now, amount); }
	}
};

/** @brief Reset the renderer's session pose. */
void eyes_life_reset();
/** @brief Advance the current live pose after face/IMU updates. @param now Time. @param dt Delta. */
void eyes_life_step(uint32_t now, uint32_t dt);
/** @brief Read current visual pose. @return Frame. */
const EyeLifeFrame &eyes_life_frame();
