/**
 * @file eye_life.h
 * @brief Emotional face handoffs, damped gaze and bounded interactive gestures.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"
#include "face_random.h"
#include "face_presets.h"
#include "eye_play.h"
#include <math.h>

struct EyeLifeFrame {
	bool active = false;
	bool busy = false;
	uint32_t variationSeed = 0;
	float variationAmount = 0;
	Expression expression = NEUTRAL;
	float openL = 1, openR = 1, scaleX = 1, scaleY = 1;
	int16_t x = 0, y = 0;
	EyePlayPose play;
};

class EyeLife {
	EyeLifeFrame frame;
	EyePlay play;
	Expression target = NEUTRAL;
	EyeStyle style = NORMAL;
	bool changing = false, gesturing = false;
	uint32_t change_at = 0, gesture_at = 0, idle_at = 0, idle_delay = 18000;
	uint32_t pending_seed = 0, variation_at = 0, gesture_duration = 900;
	uint8_t gesture = 0;
	float fromL = 1, fromR = 1, gazeX = 0, gazeY = 0;
	int8_t sign = 1;
	float comfort = 0;
	bool touching = false, tilted = false, reacting = false, touch_reaction = false;
	uint32_t reaction_at = 0;
	uint32_t covered_blink_at = 0;
	bool covered_blink = false;
	int8_t lean = 1;
	/** @brief Scenes and symbols own their entry, opening and pose. */
	static bool ordinary(Expression expression) {
		const auto *p = face_preset(expression);
		return expression != DIZZY && p && !p->fullScene && !p->normalCycle && !p->keepOpen && !p->wholeEye;
	}
	/** @brief React once per contact or tilt excursion; never queue behind higher-priority feedback. */
	bool interact(const EyeState &st, uint32_t now, float amount, bool enabled, bool motion) {
		bool contact = st.tracking && !touching;
		int magnitude = abs(st.tiltX) + abs(st.tiltY);
		bool excursion = motion && magnitude >= 7 && !tilted;
		if (magnitude <= 3 || !motion) { tilted = false; }
		else if (excursion) { tilted = true; }
		touching = st.tracking;
		if (!enabled || (reacting && !touch_reaction && !motion)) { reacting = false; return false; }
		if (contact || (excursion && !st.tracking && !reacting)) {
			reacting = true; touch_reaction = contact; reaction_at = now;
			lean = st.tiltX < 0 ? -1 : 1;
		}
		if (!reacting) { return false; }
		float p = float(uint32_t(now - reaction_at)) / (touch_reaction ? 420 : 700);
		if (p >= 1 || (!touch_reaction && st.tracking)) { reacting = false; return false; }
		float wave = sinf(3.14159265f * p) * amount;
		frame.scaleY += wave * (touch_reaction ? 0.09f : 0.045f);
		frame.scaleX -= wave * 0.025f;
		if (!touch_reaction) {
			frame.y -= int16_t(wave * 3);
			if (lean > 0) { frame.openL *= 1 - wave * 0.18f; }
			else { frame.openR *= 1 - wave * 0.18f; }
		}
		return true;
	}
	/** @brief Blend explicit affection and pickup feedback without replacing a face or palette. */
	bool feedback(const EyeState &st, uint32_t now, uint32_t dt) {
		if (st.dizzyUntilMs) { comfort = 0; return false; }
		bool pet = st.petUntilMs && int32_t(st.petUntilMs - now) > 0;
		float alpha = float(dt > 100 ? 100 : dt) / (180 + float(dt > 100 ? 100 : dt));
		comfort += ((pet ? 1.0f : 0.0f) - comfort) * alpha;
		if (!pet && comfort < 0.005f) { comfort = 0; }
		frame.openL *= 1 - comfort * 0.38f; frame.openR *= 1 - comfort * 0.38f;
		frame.scaleX += comfort * 0.025f; frame.scaleY -= comfort * 0.025f;
		bool waking = !st.tracking && !pet && !comfort && st.wakeUntilMs && int32_t(st.wakeUntilMs - now) > 0;
		if (waking) {
			float p = 1 - float(uint32_t(st.wakeUntilMs - now)) / 1400;
			float open = 0.55f + 0.45f * ease(p / 0.6f);
			frame.openL *= open; frame.openR *= open;
			frame.scaleY += sinf(3.14159265f * p) * 0.08f;
		}
		return pet || comfort > 0 || waking;
	}
	/** @brief Give every entry a fresh idle wait instead of a fixed first gesture time. */
	void arm_idle(uint32_t now, uint32_t choice, uint8_t mode, float amount) {
		idle_at = now;
		idle_delay = (mode == 2 ? 3000 : 5000) + choice % 7001 + (amount < 0.8f ? 5000 : 0);
	}
	/** @brief Smoothstep in a bounded interval. @param p Progress. @return Eased. */
	static float ease(float p) { p = p < 0 ? 0 : (p > 1 ? 1 : p); return p * p * (3 - 2 * p); }
	/** @brief Change the event seed only when the new face becomes visible behind the blink. */
	void reveal(uint32_t now) {
		if (frame.expression != target) { frame.variationSeed = pending_seed; variation_at = now; }
		frame.expression = target;
	}
	/** @brief Close, hold briefly, then reopen over a face handoff. @param now Time. @param amount Intensity. */
	void transition(uint32_t now, float amount) {
		float p = float(uint32_t(now - change_at)) / 320;
		if (p >= 1) { changing = false; reveal(now); return; }
		if (p < 0.35f) {
			float close = ease(p / 0.35f);
			frame.openL = fromL * (1 - close); frame.openR = fromR * (1 - close);
		} else {
			reveal(now);
			float opening = ease((p - 0.45f) / 0.55f);
			if (ordinary(target) && amount) {
				if (target == SAD || target == SLEEPY || target == TIRED || target == SAD_DOWN || target == SLEEPY_EYES) {
					opening *= opening; frame.y += int16_t(sinf(3.14159265f * opening) * 3 * amount);
				} else if (target == SURPRISED || target == SHOCKED || target == AWE) {
					opening = 1 - (1 - opening) * (1 - opening);
				} else if (target == HAPPY || target == GLEE || target == EXCITED || target == LOVE) {
					frame.scaleY += sinf(6.2831853f * opening) * 0.045f * amount;
				}
			}
			frame.openL = frame.openR = opening;
		}
		float spring = sinf(3.14159265f * p);
		frame.scaleX += spring * 0.06f * amount;
		frame.scaleY -= spring * 0.06f * amount;
	}
	/** @brief Apply one short decorative gesture, never change base/style. @param now Time. @param amount Intensity. */
	void pose(uint32_t now, float amount) {
		float p = float(uint32_t(now - gesture_at)) / gesture_duration;
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
			case 6:
				frame.y += int16_t(sinf(p * 12.5663706f) * wave * 4 * amount);
				frame.scaleY -= wave * 0.035f * amount; break;
			case 7:
				frame.x += int16_t(sinf(p * 6.2831853f) * wave * 8 * sign * amount);
				frame.y -= int16_t(wave * 3 * amount); break;
			case 8:
				frame.x += int16_t(wave * 4 * sign * amount);
				frame.y -= int16_t(wave * 2 * amount);
				if (sign > 0) { frame.openR *= 1 - wave * 0.32f; }
				else { frame.openL *= 1 - wave * 0.32f; } break;
		}
	}
	/** @brief Smooth per-entry micro-movements; preserve Calm and the explicit symmetry setting. */
	void vary(uint32_t now, bool asymmetric, bool tracking) {
		float a = frame.variationAmount;
		if (!a) { return; }
		uint32_t elapsed = now - variation_at, seed = frame.variationSeed;
		auto sample = [&](uint32_t salt) {
			salt += uint32_t(frame.expression) * 1597u;
			uint32_t period = 1400 + uint32_t(face_random(0, salt, seed) * 2800);
			return face_smooth_random(elapsed, period, salt, seed);
		};
		// Alternate quiet holds and livelier beats; never draw new random values on repaint.
		float activity = 0.3f + 0.7f * ease((sample(823) - 0.25f) / 0.5f);
		a = (a > 1.2f ? 1.2f : a) * activity;
		if (!tracking) {
			frame.x += int16_t((sample(173) - 0.5f) * 8 * a);
			frame.y += int16_t((sample(947) - 0.5f) * 6 * a);
		}
		frame.scaleX += (sample(511) - 0.5f) * 0.06f * a;
		frame.scaleY += (sample(733) - 0.5f) * 0.08f * a;
		if (asymmetric) {
			frame.openL *= 1 - sample(157) * 0.045f * a;
			frame.openR *= 1 - sample(439) * 0.045f * a;
		}
	}
public:
	/** @brief Reset on initialization. */
	void reset() { *this = EyeLife{}; }
	/** @brief Access a read-only visual pose. @return Frame. */
	const EyeLifeFrame &get() const { return frame; }
	/** @brief Advance without drawing or I/O. @param cfg Target. @param st Raw gaze/blink. @param now Time. @param dt Bounded delta. @param mode Calm/Dasai/playful. @param amount Intensity. @param choice Random input. @param allow_idle Start idle gestures unless a handoff is due. @param motion Enable tilt feedback. */
	void step(const EyeConfig &cfg, const EyeState &st, uint32_t now, uint32_t dt,
		uint8_t mode, float amount, uint32_t choice, bool allow_idle = true, bool motion = false) {
		if (!frame.active || style != cfg.style) {
			reset(); frame.active = true; style = cfg.style; frame.expression = target = cfg.expression; idle_at = now;
			frame.variationSeed = choice ? choice : 1; variation_at = now;
			arm_idle(now, choice, mode, amount);
		}
		if (cfg.expression != target) {
			fromL = frame.openL; fromR = frame.openR; change_at = now;
			target = cfg.expression; changing = true; gesturing = false; idle_at = now;
			pending_seed = choice ? choice : 1;
			arm_idle(now, choice, mode, amount);
		}
		frame.variationAmount = mode ? amount : 0;
		frame.openL = frame.openR = frame.scaleX = frame.scaleY = 1;
		float alpha = float(dt > 100 ? 100 : dt) / (80 + float(dt > 100 ? 100 : dt));
		gazeX += (st.saccadeX + (st.tracking ? 0 : st.tiltX) - gazeX) * alpha;
		gazeY += (st.saccadeY + (st.tracking ? 0 : st.tiltY + st.floatY) - gazeY) * alpha;
		frame.x = int16_t(gazeX); frame.y = int16_t(gazeY);
		if (changing) { transition(now, mode ? amount : 0); }
		if (!st.blinking || st.blinkStartMs != covered_blink_at) { covered_blink = false; }
		if (changing && st.blinking) { covered_blink = true; covered_blink_at = st.blinkStartMs; }
		if (st.blinking && !changing && !covered_blink) {
			float p = float(uint32_t(now - st.blinkStartMs)) / (st.blinkDurMs ? st.blinkDurMs : 1);
			float open = p < 1 ? 1 - sinf(3.14159265f * p) : 1;
			frame.openL *= open; frame.openR *= open;
		}
		bool responding = feedback(st, now, dt);
		bool playful = mode && !responding && !st.dizzyUntilMs && ordinary(frame.expression) && ordinary(cfg.expression);
		bool expressive = playful && !changing;
		responding = interact(st, now, amount, expressive, motion) || responding;
		frame.play = play.step(st, now, amount, playful, allow_idle && !changing && !gesturing && !responding, choice);
		responding = responding || frame.play.busy;
		if (!expressive || st.tracking || responding) { gesturing = false; idle_at = now; }
		if (expressive && allow_idle && !st.tracking && !responding && !gesturing && uint32_t(now - idle_at) >= idle_delay) {
			gesture = (gesture + 1 + choice % 8) % 9;
			sign = choice & 1 ? 1 : -1; gesture_at = idle_at = now; gesturing = true;
			gesture_duration = 700 + choice % 501;
			idle_delay = (mode == 2 ? 4000 : 7000) + choice % 10001 + (amount < 0.8f ? 6000 : 0);
		}
		if (gesturing && !changing) { pose(now, amount); }
		vary(now, cfg.asym != 0, st.tracking);
		frame.x += frame.play.x; frame.y += frame.play.y;
		frame.openL *= frame.play.openL; frame.openR *= frame.play.openR;
		frame.busy = changing || gesturing || responding;
	}
};

/** @brief Reset the renderer's session pose. */
void eyes_life_reset();
/** @brief Advance the current live pose after face/IMU updates. @param now Time. @param dt Delta. */
void eyes_life_step(uint32_t now, uint32_t dt);
/** @brief Read current visual pose. @return Frame. */
const EyeLifeFrame &eyes_life_frame();
