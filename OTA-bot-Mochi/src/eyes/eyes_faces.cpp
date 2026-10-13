/**
 * @file eyes_faces.cpp
 * @brief Temporary auto expressions preserve the selected base and never write flash.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "eyes_faces.h"
#include "face_policy.h"
#include "eyes_api.h"
#include "../settings/settings.h"
#include "../../pin_config.h"
#include <Arduino.h>

static FacePolicy policy;

FaceTone eyes_face_tone(Expression expr) {
	switch (expr) {
		/** Affectionate faces get a warm mid-range warble. */
		case LOVE: case SHY: case KISS: case FLIRT: case DAYDREAM: case HALO: case MERCIFUL:
			return FaceTone::Warm;
		/** Sleepy and calm faces get a slow, low chirp. */
		case SLEEPY: case SLEEPY_EYES: case TIRED: case DOZE: case LAZY: case GROGGY:
		case ZEN: case MELTING: case DEADPAN:
			return FaceTone::Soft;
		/** Excited and surprised faces get a quick high bounce. */
		case SURPRISED: case SHOCKED: case EXCITED: case GLEE: case STAR: case SPARKLE:
		case BOLT: case MANIC: case CHEEKY: case BEAMING: case WINK:
			return FaceTone::Bounce;
		/** Negative and uneasy faces get a short low blip. */
		case SAD: case SAD_DOWN: case SAD_UP: case ANGRY: case ANNOYED: case FRUSTRATED:
		case UNIMPRESSED: case FURIOUS: case SCARED: case TERRIFIED: case DISGUSTED:
		case GLUM: case CRYING: case ZOMBIE: case POUT: case WORRIED: case NERVOUS:
		case GRUMPY: case SKEPTIC: case SQUINT: case SUSPICIOUS:
			return FaceTone::Flat;
		default: return FaceTone::Bright;
	}
}

/** @brief Reset automatic reactions after an explicit mood change. @param base Mood. */
void eyes_faces_base(Expression base) { policy.reset(base, millis()); }

/**
 * @brief Trigger a random temporary face unless dizzy is active, and glance at the tap.
 * @param now Timestamp.
 * @param x Tap X in logical screen coordinates.
 * @param y Tap Y in logical screen coordinates.
 */
void eyes_faces_tap(uint32_t now, int16_t x, int16_t y) {
	if (eyes_state().dizzyUntilMs != 0) { return; }
	policy.tap(now, uint32_t(random(1000000L)));
	eyes_glance(x, y, now);
	eyes_faces_step(now);
	sound_request_face(eyes_face_tone(eyes_config().expression), now);
}

/**
 * @brief Trigger a playful temporary face unless dizzy is active, and glance at the tap.
 * @param now Timestamp.
 * @param x Tap X in logical screen coordinates.
 * @param y Tap Y in logical screen coordinates.
 */
void eyes_faces_double(uint32_t now, int16_t x, int16_t y) {
	if (eyes_state().dizzyUntilMs != 0) { return; }
	policy.play(now, uint32_t(random(1000000L)));
	eyes_glance(x, y, now);
	eyes_faces_step(now);
	sound_request_face(eyes_face_tone(eyes_config().expression), now);
}

/** @brief Trigger an affectionate temporary face and a heart marker unless dizzy is active. @param now Timestamp. @param x Stroke X. @param y Stroke Y. */
void eyes_faces_pet(uint32_t now, int16_t x, int16_t y) {
	if (eyes_state().dizzyUntilMs != 0) { return; }
	policy.pet(now, uint32_t(random(1000000L)));
	eyes_faces_step(now);
	sound_request_face(eyes_face_tone(eyes_config().expression), now);
	EyeState &state = eyes_state();
	state.heartX = int16_t(x < 7 ? 7 : (x > LCD_WIDTH - 8 ? LCD_WIDTH - 8 : x));
	state.heartY = int16_t(y < 20 ? 20 : (y > LCD_HEIGHT - 14 ? LCD_HEIGHT - 14 : y));
	state.heartUntilMs = now + 900;
}

/** @brief Select a temporary expression, leaving DIZZY in the foreground. @param now Timestamp. */
void eyes_faces_step(uint32_t now) {
	const CompanionSettings &p = settings_get();
	Expression face = policy.update(now, p.random_faces, p.imu_faces, uint32_t(random(1000000L)));
	EyeState &state = eyes_state();
	if (state.dizzyUntilMs != 0) { state.savedExpr = face; }
	else {
		if (eyes_config().expression != face && policy.automatic()) {
			sound_request_face(eyes_face_tone(face), now);
		}
		eyes_config().expression = face;
	}
}

/** @brief Pass bounded sensor data to the mood policy. @param x G. @param y G. @param z G. @param now Timestamp. */
void eyes_faces_motion(float x, float y, float z, uint32_t now) {
	policy.sample(x, y, z, now, settings_get().imu_faces);
}

/** @brief Silent startup greeting face that preserves the selected mood. @param now Timestamp. */
void eyes_faces_greet(uint32_t now) {
	if (eyes_state().dizzyUntilMs != 0) { return; }
	policy.tap(now, uint32_t(random(1000000L)));
	eyes_faces_step(now);
}
