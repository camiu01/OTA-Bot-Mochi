/**
 * @file eye_play.h
 * @brief Short visual-only touch reactions and rare gags; never consumes a gesture or changes mood.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"
#include <math.h>

struct EyePlayPose {
	bool busy = false, fly = false;
	float openL = 1, openR = 1;
	int16_t x = 0, y = 0, leftY = 0, rightY = 0, flyX = 0, flyY = 0;
};

class EyePlay {
	bool initialized = false, touching = false, touch_active = false, gag_active = false;
	uint8_t taps = 0, reaction = 0, gag = 0;
	int8_t sign = 1;
	uint32_t last_touch = 0, touch_at = 0, gag_at = 0, quiet_at = 0, quiet_delay = 45000;
	/** @brief Consume contact edges even when suppressed; do not queue feedback behind petting/scenes. */
	void contact(const EyeState &st, uint32_t now, bool enabled) {
		bool edge = st.tracking && !touching; touching = st.tracking;
		if (!enabled) {
			if (st.tracking || st.dizzyUntilMs || st.petUntilMs || st.wakeUntilMs) { taps = 0; quiet_at = now; }
			touch_active = false; return;
		}
		if (!edge) { return; }
		taps = uint32_t(now - last_touch) <= 2500 ? uint8_t(taps + 1) : 1;
		last_touch = touch_at = quiet_at = now; touch_active = true; gag_active = false;
		reaction = taps >= 3 ? 2 : (abs(st.saccadeX) + abs(st.saccadeY) > 6 ? 1 : 0);
		sign = st.saccadeX < 0 ? 1 : -1;
		if (reaction == 2) { taps = 0; }
	}
	/** @brief Center tickle, off-center dodge, then a brief sulk after three close contacts. */
	void touch_pose(EyePlayPose &pose, uint32_t now, float amount) {
		float p = float(uint32_t(now - touch_at)) / (reaction == 2 ? 900 : 420);
		if (p >= 1) { touch_active = false; return; }
		float wave = sinf(3.14159265f * p) * amount;
		pose.busy = true;
		if (reaction == 0) {
			pose.x = int16_t(sinf(p * 18.849556f) * wave * 3);
			pose.openL = 1 - wave * 0.2f; pose.openR = 1 - wave * 0.3f;
		} else if (reaction == 1) {
			pose.x = int16_t(wave * 8 * sign); pose.y = int16_t(wave * 2);
		} else {
			pose.x = int16_t(wave * 4 * sign);
			pose.openL = pose.openR = 1 - wave * 0.45f;
		}
	}
	/** @brief Peek sideways, catch a slipping eye, or follow a tiny fly; all finish within 1100ms. */
	void gag_pose(EyePlayPose &pose, uint32_t now, float amount) {
		float p = float(uint32_t(now - gag_at)) / 1100;
		if (p >= 1) { gag_active = false; return; }
		float wave = sinf(3.14159265f * p) * amount;
		pose.busy = true;
		if (gag == 0) {
			pose.x = int16_t(wave * 8 * sign);
			pose.openL = 1 - wave * 0.65f; pose.openR = 1 - wave * 0.15f;
		} else if (gag == 1) {
			float slip = wave * wave * 24;
			pose.rightY = int16_t(slip); pose.leftY = -int16_t(wave * 3);
			pose.openL = 1 - wave * 0.25f;
		} else {
			pose.fly = p > 0.08f && p < 0.92f;
			pose.flyX = int16_t((p * 2 - 1) * 72 * sign);
			pose.flyY = int16_t(-55 + sinf(p * 12.566371f) * 8);
			pose.x = int16_t((p * 2 - 1) * wave * 8 * sign); pose.y = -int16_t(wave * 5);
		}
	}
public:
	/** @brief Advance in the main task; caller reserves the next 1250ms for automatic handoffs. */
	EyePlayPose step(const EyeState &st, uint32_t now, float amount, bool enabled, bool allow_gag, uint32_t choice) {
		EyePlayPose pose;
		if (!initialized) { initialized = true; quiet_at = now; quiet_delay += choice % 45001; }
		contact(st, now, enabled);
		if (!enabled) { gag_active = false; return pose; }
		if (touch_active) { touch_pose(pose, now, amount); return pose; }
		if (st.tracking) { quiet_at = now; gag_active = false; return pose; }
		if (!allow_gag) { gag_active = false; }
		if (!gag_active && allow_gag && uint32_t(now - quiet_at) >= quiet_delay) {
			gag_active = true; gag_at = quiet_at = now; gag = uint8_t(choice % 3);
			sign = choice & 1 ? 1 : -1; quiet_delay = 45000 + choice % 45001;
		}
		if (gag_active) { gag_pose(pose, now, amount); }
		return pose;
	}
};
