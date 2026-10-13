/**
 * @file eyes_faces.cpp
 * @brief Temporary auto expressions preserve the selected base and never write flash.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "eyes_faces.h"
#include "face_policy.h"
#include "eyes_api.h"
#include "../settings/settings.h"
#include "../sound/sound.h"
#include <Arduino.h>

static FacePolicy policy;

/** @brief Reset automatic reactions after an explicit mood change. @param base Mood. */
void eyes_faces_base(Expression base) { policy.reset(base, millis()); }
/** @brief Trigger a random temporary face unless dizzy is active. @param now Timestamp. */
void eyes_faces_tap(uint32_t now) {
	if (eyes_state().dizzyUntilMs != 0) { return; }
	policy.tap(now, uint32_t(random(1000000L)));
	sound_request(SoundCue::Tap, now);
	eyes_faces_step(now);
}

/** @brief Select a temporary expression, leaving DIZZY in the foreground. @param now Timestamp. */
void eyes_faces_step(uint32_t now) {
	const CompanionSettings &p = settings_get();
	Expression face = policy.update(now, p.random_faces, p.imu_faces, uint32_t(random(1000000L)));
	EyeState &state = eyes_state();
	if (state.dizzyUntilMs != 0) { state.savedExpr = face; }
	else {
		if (eyes_config().expression != face && policy.automatic()) { sound_request(SoundCue::Mood, now); }
		eyes_config().expression = face;
	}
}

/** @brief Pass bounded sensor data to the mood policy. @param x G. @param y G. @param z G. @param now Timestamp. */
void eyes_faces_motion(float x, float y, float z, uint32_t now) {
	policy.sample(x, y, z, now, settings_get().imu_faces);
}
