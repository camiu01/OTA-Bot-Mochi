/**
 * @file eyes_test.cpp
 * @brief Real eye renderer against a native raster: styles, symmetry, blinks, and trails.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eye_geometry.h"
#include "../OTA-bot-Mochi/src/eyes/face_policy.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_faces.h"
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include <assert.h>
#include <algorithm>
#include <iostream>

/** @brief Check solid versus pupilled style rendering, not just preset dimensions. */
static void style_tests() {
	Arduino_GFX gfx;
	eyes_init(&gfx);
	std::vector<uint16_t> mochi;
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_set_style(static_cast<EyeStyle>(style));
		eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
		eyes_set_expression(NEUTRAL);
		eyes_state().dirtyInit = false;
		eyes_draw();
		bool solid = style == MOCHI || style == TRAPEZ;
		assert((std::count(gfx.pixels.begin(), gfx.pixels.end(), 0xF800) == 0) == solid);
		if (solid) { assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), 0x07E0) == 0); }
		if (style == MOCHI) { mochi = gfx.pixels; assert(eyes_config().eyeH > eyes_config().eyeW * 2); }
		if (style == BLOUB) { assert(gfx.pixels != mochi && eyes_config().eyeW > eyes_config().eyeH); }
		if (style == TRAPEZ) {
			for (int y = 0; y < 280; ++y) {
				for (int x = 0; x < 120; ++x) { assert(gfx.pixels[y * 240 + x] == gfx.pixels[y * 240 + 239 - x]); }
			}
		}
	}
}

/** @brief All styles/expressions must erase old geometry and keep their union inside the screen. */
static void dirty_region_tests() {
	Arduino_GFX gfx;
	eyes_init(&gfx);
	eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_set_style(static_cast<EyeStyle>(style));
		for (uint8_t expression = 0; expression < EXPRESSION_COUNT; ++expression) {
			for (bool blinking : {false, true}) {
				eyes_set_expression(static_cast<Expression>(expression));
				EyeState &st = eyes_state();
				st.saccadeX = expression % 2 ? -6 : 6;
				st.tiltY = expression % 2 ? 8 : -8;
				st.blinking = blinking;
				eyes_draw();
				std::vector<uint16_t> dirty = gfx.pixels;
				st.dirtyInit = false;
				eyes_draw();
				assert(gfx.pixels == dirty);
				assert(st.prevX >= 0 && st.prevY >= 0);
				assert(st.prevX + st.prevW <= 240 && st.prevY + st.prevH <= 280);
				if (style == MOCHI || style == TRAPEZ) {
					assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), 0xF800) == 0);
					assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), 0x07E0) == 0);
				}
			}
		}
	}
}

/**
 * @brief Every expression must look different from NEUTRAL on every one of the four styles.
 *
 * The bounds and pupil-free tests only prove a new face is legal, not that it is visible.
 * This is the guard for "must work on all face styles": a face that silently degrades to
 * NEUTRAL on one style fails here instead of shipping unnoticed.
 */
static void distinct_expression_tests() {
	Arduino_GFX gfx;
	eyes_init(&gfx);
	eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_set_style(static_cast<EyeStyle>(style));
		std::vector<uint16_t> baseline;
		for (uint8_t expression = 0; expression < EXPRESSION_COUNT; ++expression) {
			EyeState &st = eyes_state();
			st.saccadeX = st.saccadeY = st.tiltX = st.tiltY = 0;
			st.blinking = false;
			eyes_set_expression(static_cast<Expression>(expression));
			st.dirtyInit = false;
			eyes_draw();
			if (expression == NEUTRAL) { baseline = gfx.pixels; continue; }
			assert(!baseline.empty() && gfx.pixels != baseline);
		}
	}
}

/**
 * @brief A sustained lateral load rolls and shifts the eye pair, which is what reads as cornering.
 *
 * Regression guard for the IMU lean: without it the gaze would only ever translate, and a
 * sign slip in the mount-relative mapping would go unnoticed by the bounds tests.
 */
static void bank_tests() {
	eyes_init(nullptr);
	EyeConfig cfg = eyes_config();
	cfg.expression = NEUTRAL;
	cfg.asym = 0;
	EyeState rest{};
	EyeLayout centre = eye_layout(cfg, rest);
	EyeState leaning = rest;
	leaning.tiltX = 12;
	EyeLayout right_turn = eye_layout(cfg, leaning);
	assert(right_turn.left.y < centre.left.y && right_turn.right.y > centre.right.y);
	assert(right_turn.left.x > centre.left.x);
	leaning.tiltX = -12;
	EyeLayout left_turn = eye_layout(cfg, leaning);
	assert(left_turn.left.y > left_turn.right.y && left_turn.left.x < centre.left.x);
	assert(centre.left.y == centre.right.y);
}

/** @brief Random reactions are temporary, avoid the base, respect flags, and survive rollover. */
static void random_tests() {
	FacePolicy policy;
	policy.reset(NEUTRAL, 0);
	assert(policy.update(11999, true, false, 0) == NEUTRAL);
	assert(policy.update(12000, true, false, 0) == HAPPY);
	assert(policy.update(14999, true, false, 0) == HAPPY);
	assert(policy.update(15000, true, false, 0) == NEUTRAL);
	assert(policy.update(24000, true, false, 0) != HAPPY);
	assert(policy.update(24001, false, false, 0) == NEUTRAL);
	policy.reset(HAPPY, UINT32_MAX - 6000);
	assert(policy.update(6000, true, false, 0) != HAPPY);
	assert(policy.update(9000, false, false, 0) == HAPPY);
}

/** @brief IMU faces require sustained motion, not noise/one bump; random cannot interrupt them. */
static void imu_face_tests() {
	FacePolicy policy;
	policy.reset(HAPPY, 0);
	policy.sample(0, 0, 2, 50, true);
	policy.sample(0, 0, 0, 100, true);
	assert(policy.update(100, false, true, 0) == HAPPY);
	for (uint32_t now = 150; now <= 500; now += 50) { policy.sample(0.4f, 0, 0, now, true); }
	assert(policy.update(500, true, true, 0) == SKEPTIC);
	assert(policy.update(2700, true, true, 0) == HAPPY);
	for (uint32_t now = 5500; now <= 5850; now += 50) { policy.sample(0, 0.4f, 0, now, true); }
	assert(policy.update(5850, true, true, 0) == FOCUSED);
	assert(policy.update(5851, true, false, 0) == HAPPY);
	policy.reset(NEUTRAL, 0);
	policy.sample(0.4f, 0, 0, 0, true);
	policy.sample(0.4f, 0, 0, 1000, true);
	assert(policy.update(1000, false, true, 0) == NEUTRAL);
	for (uint32_t now = 1050; now < 2000; now += 50) {
		policy.sample(now % 100 ? -0.4f : 0.4f, 0, 0, now, true);
	}
	assert(policy.update(2000, false, true, 0) == NEUTRAL);
	policy.reset(NEUTRAL, 0);
	policy.tap(0, 0);
	Expression tapped = policy.update(0, false, true, 0);
	for (uint32_t now = 50; now <= 500; now += 50) { policy.sample(0.4f, 0, 0, now, true); }
	assert(policy.update(500, false, true, 0) == tapped);
	assert(policy.update(3000, false, true, 0) == NEUTRAL);
}

/** @brief Petting requests a temporary affectionate face and never the base. */
static void pet_tests() {
	auto affectionate = [](Expression face) {
		return face == HAPPY || face == LOVE || face == GLEE ||
			face == BEAMING || face == MILD_HAPPY || face == EXCITED;
	};
	FacePolicy policy;
	policy.reset(NEUTRAL, 0);
	policy.pet(0, 0);
	Expression petted = policy.update(0, false, false, 0);
	assert(affectionate(petted));
	assert(policy.update(2999, false, false, 0) == petted);
	assert(policy.update(3000, false, false, 0) == NEUTRAL);
	policy.reset(HAPPY, 0);
	policy.pet(0, 1);
	assert(policy.update(0, false, false, 0) != HAPPY);
	policy.reset(NEUTRAL, 0);
	policy.pet(0, 0);
	Expression held = policy.update(0, false, true, 0);
	for (uint32_t now = 50; now <= 500; now += 50) { policy.sample(0.4f, 0, 0, now, true); }
	assert(policy.update(500, false, true, 0) == held);
	assert(policy.update(3000, false, true, 0) == NEUTRAL);
	// Continued stroking keeps the same face and extends it up to six seconds.
	FacePolicy stroked;
	stroked.reset(NEUTRAL, 0);
	stroked.pet(0, 0);
	Expression stroke_face = stroked.update(0, false, false, 0);
	stroked.pet(1500, 0);
	assert(stroked.update(4499, false, false, 0) == stroke_face);
	stroked.pet(3000, 0);
	assert(stroked.update(5999, false, false, 0) == stroke_face);
	stroked.pet(4500, 0);
	assert(stroked.update(6000, false, false, 0) == NEUTRAL);
	// A tap ends the affection run: the next pet is a fresh three-second reaction.
	FacePolicy mixed;
	mixed.reset(NEUTRAL, 0);
	mixed.pet(0, 0);
	mixed.tap(1000, 0);
	mixed.pet(1200, 0);
	assert(mixed.update(4199, false, false, 0) != NEUTRAL);
	assert(mixed.update(4200, false, false, 0) == NEUTRAL);
}

/** @brief A double tap requests a playful face from its own bounded pool. */
static void play_tests() {
	auto playful = [](Expression face) {
		return face == AWE || face == EXCITED || face == STAR ||
			face == SPARKLE || face == GLEE || face == BEAMING || face == MANIC;
	};
	FacePolicy policy;
	policy.reset(NEUTRAL, 0);
	policy.play(0, 0);
	Expression played = policy.update(0, false, false, 0);
	assert(playful(played));
	assert(policy.update(3499, false, false, 0) == played);
	assert(policy.update(3500, false, false, 0) == NEUTRAL);
}

/** @brief Taps direct the gaze toward the touch point for a bounded hold. */
static void glance_tests() {
	eyes_init(nullptr);
	EyeState &st = eyes_state();
	eyes_glance(239, 279, 1000);
	assert(st.saccadeX > 0 && st.saccadeX <= 12);
	assert(st.saccadeY > 0 && st.saccadeY <= 8);
	assert(st.glanceUntilMs == 3500);
	int16_t held_x = st.saccadeX;
	eyes_update(2000);
	assert(st.saccadeX == held_x && st.glanceUntilMs == 3500);
	eyes_update(3500);
	assert(st.glanceUntilMs == 0);
	eyes_glance(0, 0, 4000);
	assert(st.saccadeX < 0 && st.saccadeY < 0);
	// Tap reactions carry the same gaze; dizzy keeps priority and ignores double taps.
	CompanionSettings p = settings_get();
	p.random_faces = p.imu_faces = false;
	settings_update(p);
	eyes_faces_tap(5000, 0, 279);
	assert(st.saccadeX < 0 && st.saccadeY > 0 && st.glanceUntilMs == 7500);
	eyes_state().dizzyUntilMs = 8000;
	eyes_config().expression = DIZZY;
	eyes_faces_double(6000, 239, 0);
	assert(eyes_config().expression == DIZZY && st.glanceUntilMs == 7500);
	eyes_state().dizzyUntilMs = 0;
	st.glanceUntilMs = 0;
}

/** @brief A startup greeting is a silent temporary face that preserves the mood. */
static void greet_tests() {
	eyes_init(nullptr);
	settings_defaults();
	eyes_faces_base(static_cast<Expression>(settings_get().mood));
	eyes_faces_greet(80000);
	Expression greeted = eyes_config().expression;
	assert(greeted != settings_get().mood);
	eyes_update(82999);
	assert(eyes_config().expression == greeted);
	eyes_update(83000);
	assert(eyes_config().expression == settings_get().mood);
}

/** @brief Expression families map to distinct chirp tone bands. */
static void tone_tests() {
	assert(eyes_face_tone(Expression(HAPPY)) == FaceTone::Bright);
	assert(eyes_face_tone(Expression(NEUTRAL)) == FaceTone::Bright);
	assert(eyes_face_tone(Expression(LOVE)) == FaceTone::Warm);
	assert(eyes_face_tone(Expression(KISS)) == FaceTone::Warm);
	assert(eyes_face_tone(Expression(SLEEPY)) == FaceTone::Soft);
	assert(eyes_face_tone(Expression(ZEN)) == FaceTone::Soft);
	assert(eyes_face_tone(Expression(SHOCKED)) == FaceTone::Bounce);
	assert(eyes_face_tone(Expression(BEAMING)) == FaceTone::Bounce);
	assert(eyes_face_tone(Expression(SAD)) == FaceTone::Flat);
	assert(eyes_face_tone(Expression(SKEPTIC)) == FaceTone::Flat);
	// The newest faces keep their families: waiting/power stay soft, glitch stays flat.
	assert(eyes_face_tone(Expression(MUSIC)) == FaceTone::Warm);
	assert(eyes_face_tone(Expression(LOW_BATT)) == FaceTone::Soft);
	assert(eyes_face_tone(Expression(LOADING)) == FaceTone::Soft);
	assert(eyes_face_tone(Expression(HYPER)) == FaceTone::Bounce);
	assert(eyes_face_tone(Expression(SQUISH)) == FaceTone::Bounce);
	assert(eyes_face_tone(Expression(GLITCH)) == FaceTone::Flat);
	assert(eyes_face_tone(Expression(CALCULATING)) == FaceTone::Flat);
	assert(eyes_face_tone(Expression(CYBORG)) == FaceTone::Flat);
	// Out-of-range expressions fall back to the bright family instead of reading past tables.
	assert(eyes_face_tone(Expression(EXPRESSION_COUNT)) == FaceTone::Bright);
}

/** @brief Test real automatic-face integration without changing style or saved mood. */
static void integration_tests() {
	eyes_init(nullptr);
	CompanionSettings p = settings_get();
	p.random_faces = true; p.imu_faces = false; p.mood = 0; p.style = MOCHI;
	test_now = 0;
	settings_update(p);
	test_now = 12000;
	eyes_update(test_now);
	assert(eyes_config().expression != NEUTRAL && eyes_config().style == MOCHI);
	assert(settings_get().mood == 0);
	test_now = 15000;
	eyes_update(test_now);
	assert(eyes_config().expression == NEUTRAL);
	p.random_faces = false;
	settings_update(p);
	test_now = 16000;
	eyes_faces_tap(test_now, 120, 140);
	Expression tapped = eyes_config().expression;
	assert(tapped != NEUTRAL && settings_get().mood == 0 && !settings_get().random_faces);
	eyes_update(18999);
	assert(eyes_config().expression == tapped);
	eyes_update(19000);
	assert(eyes_config().expression == NEUTRAL);
	eyes_faces_tap(19001, 120, 140);
	assert(eyes_config().expression != tapped);
	eyes_faces_pet(19002, 120, 180);
	Expression petted = eyes_config().expression;
	assert(petted == HAPPY || petted == LOVE || petted == GLEE ||
		petted == BEAMING || petted == MILD_HAPPY || petted == EXCITED);
	assert(settings_get().mood == 0);
	assert(eyes_state().heartUntilMs == 19902 && eyes_state().heartX == 120 && eyes_state().heartY == 180);
	// Continued stroking extends the affection face, and out-of-range points clamp on screen.
	eyes_faces_pet(19002, 2, 2);
	assert(eyes_config().expression == petted);
	assert(eyes_state().heartUntilMs == 19902 && eyes_state().heartX == 7 && eyes_state().heartY == 20);
	eyes_state().dizzyUntilMs = 21001;
	eyes_config().expression = DIZZY;
	eyes_faces_tap(19002, 120, 140);
	assert(eyes_config().expression == DIZZY);
	eyes_state().dizzyUntilMs = 0;
	// The heart marker survives until its expiry, then the dirty union drops it.
	eyes_update(19901);
	assert(eyes_state().heartUntilMs == 19902);
	eyes_update(19902);
	assert(eyes_state().heartUntilMs == 0);
}

/** @brief Face handoffs occur while closed; damped gaze and soft blinking remain bounded. */
static void motion_policy_tests() {
	EyeLife life;
	EyeConfig cfg{}; cfg.style = MOCHI; cfg.expression = NEUTRAL;
	EyeState st{};
	life.step(cfg, st, 0, 0, 1, 0.55f, 0);
	cfg.expression = LOVE;
	life.step(cfg, st, 32, 32, 1, 0.55f, 0);
	assert(life.get().expression == NEUTRAL);
	life.step(cfg, st, 144, 100, 1, 0.55f, 0);
	assert(life.get().expression == LOVE && life.get().openL < 0.01f);
	life.step(cfg, st, 352, 100, 1, 0.55f, 0);
	assert(life.get().expression == LOVE && life.get().openL == 1);
	st.saccadeX = 20;
	life.step(cfg, st, 384, 32, 0, 0.55f, 0);
	assert(life.get().x > 0 && life.get().x < 20);
	st.blinking = true; st.blinkStartMs = 384; st.blinkDurMs = 200;
	life.step(cfg, st, 484, 100, 0, 0.55f, 0);
	assert(life.get().openL < 0.01f);
}

/** @brief Animated expression/gesture frames keep solid styles pupil-free and erase actual previous bounds. */
static void animated_raster_tests() {
	Arduino_GFX gfx;
	auto p = settings_get(); p.animation = 2; p.random_faces = p.imu_faces = false; settings_update(p);
	for (uint8_t style = 0; style < 4; ++style) {
		eyes_init(&gfx); eyes_set_style(static_cast<EyeStyle>(style));
		eyes_set_eye_colors(0, 0xFFFF, 0xF800, 0x07E0);
		// Keep a heart marker active so the wipe/redraw invariant covers it every frame.
		eyes_state().heartUntilMs = 40001;
		eyes_state().heartX = 120; eyes_state().heartY = 60;
		for (uint32_t now = 0; now <= 40000; now += 16) {
			if (now % 4000 == 0) { eyes_set_expression(now % 8000 ? LOVE : HAPPY); }
			eyes_life_step(now, 16);
			eyes_draw();
			auto dirty = gfx.pixels;
			eyes_state().dirtyInit = false; eyes_draw();
			assert(dirty == gfx.pixels);
			const auto &pose = eyes_life_frame();
			assert(pose.openL >= 0 && pose.openL <= 1 && pose.openR >= 0 && pose.openR <= 1);
			if (style == MOCHI || style == TRAPEZ) {
				assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), 0xF800) == 0);
				assert(std::count(gfx.pixels.begin(), gfx.pixels.end(), 0x07E0) == 0);
			}
		}
	}
}

/** @brief Run real-renderer and automatic-expression regressions. @return Success. */
int main() {
	assert(settings_init());
	style_tests();
	dirty_region_tests();
	distinct_expression_tests();
	bank_tests();
	random_tests();
	imu_face_tests();
	pet_tests();
	play_tests();
	glance_tests();
	greet_tests();
	tone_tests();
	integration_tests();
	motion_policy_tests();
	animated_raster_tests();
	std::cout << "Real eye rendering, solid styles, mirrored trapezoids, dirty regions, and auto faces passed\n";
}
