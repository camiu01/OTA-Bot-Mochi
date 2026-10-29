/**
 * @file face_filter_test.cpp
 * @brief Global exclusions, one-face pools, independent NVS, migration and rendering.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/face_policy.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_faces.h"
#include "../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/settings/usb_commands.h"
#include <Preferences.h>
#include <cassert>
#include <iostream>
#include <string>

/** @brief Restrict a valid mask to one ID without transiently creating an empty list. */
static FaceFilter only(Expression keep) {
	FaceFilter f;
	for (uint8_t i = 0; i < EXPRESSION_COUNT; ++i) { if (i != keep) { assert(f.set(Expression(i), false)); } }
	return f;
}
/** @brief Strict encodings, invalid IDs, empty masks, corruption and appended-ID migration. */
static void formats() {
	FaceFilter f, decoded;
	assert(f.count() == EXPRESSION_COUNT && !f.allows(Expression(255)));
	assert(!f.set(Expression(255), true));
	f = only(TIC_TAC_TOE);
	assert(!f.set(TIC_TAC_TOE, false) && f.count() == 1);
	char hex[FaceFilter::BYTES * 2 + 1]; f.hex(hex);
	assert(decoded.parse_hex(hex) && decoded == f);
	for (std::string bad : {std::string(FaceFilter::BYTES * 2, '0'), std::string(FaceFilter::BYTES * 2, 'f'), std::string("xyz")}) {
		assert(!decoded.parse_hex(bad.c_str()) && decoded == f);
	}
	uint8_t record[FaceFilter::RECORD_BYTES]; f.encode(record);
	assert(decoded.decode(record, sizeof(record)) && decoded == f);
	assert(!decoded.decode(record, sizeof(record) - 1));
	record[2] ^= 1; assert(!decoded.decode(record, sizeof(record)) && decoded == f);
	uint8_t old[] = {1, 8, 1, uint8_t(0xA5 ^ 1 ^ 8 ^ 1)};
	assert(decoded.decode(old, sizeof(old)));
	assert(decoded.allows(NEUTRAL) && !decoded.allows(HAPPY) && decoded.allows(TIC_TAC_TOE));
	old[2] = 0; old[3] = 0xA5 ^ 1 ^ 8;
	assert(!decoded.decode(old, sizeof(old)));
}
/** @brief Every one-face library survives all modes, touches and sustained IMU samples. */
static void pools() {
	for (uint8_t id = 0; id < EXPRESSION_COUNT; ++id) {
		face_filter() = only(Expression(id));
		for (uint8_t mode = 0; mode < FACE_FLOW_COUNT; ++mode) {
			FacePolicy p; uint32_t start = UINT32_MAX - 1000; p.reset(NEUTRAL, start);
			for (uint32_t t = 0; t < 60000; t += 100) {
				uint32_t now = start + t;
				p.sample(0.5f, 0.5f, 1.0f, now, true);
				assert(p.update(now, true, true, 0, true, mode) == Expression(id));
				if (t == 1000) { p.tap(now, 0); }
				if (t == 2000) { p.play(now, 0); }
				if (t == 3000) { p.pet(now, 0); }
			}
		}
	}
	face_filter() = FaceFilter{};
}
/** @brief Face-only saves never commit unrelated preferences; failures remain retryable. */
static void persistence() {
	assert(settings_init()); eyes_init(nullptr); settings_apply();
	assert(settings_save()); auto original = test_profile;
	auto p = settings_get(); p.brightness = 70; assert(settings_update(p));
	unsigned writes = test_flash_writes;
	assert(settings_faces_update(only(TIC_TAC_TOE)));
	assert(settings_get().mood == TIC_TAC_TOE && eyes_config().expression == TIC_TAC_TOE);
	assert(settings_faces_dirty() && test_flash_writes == writes);
	test_save_fails = true; assert(!settings_faces_save() && settings_faces_dirty());
	test_save_fails = false;
	assert(settings_faces_save() && !settings_faces_dirty() && settings_dirty());
	assert(test_profile == original && test_flash_writes == writes + 1);
	assert(settings_faces_save() && test_flash_writes == writes + 1);
	assert(settings_init() && settings_get().brightness == 30 && face_filter().count() == 1);
	assert(settings_get().mood == TIC_TAC_TOE);
	assert(settings_parse("set mood happy", settings_get()).action == UsbAction::Invalid);
	assert(!eyes_set_expression_by_name("happy"));
	eyes_set_expression(HAPPY); assert(eyes_config().expression == TIC_TAC_TOE);
	eyes_imu_trigger_shake(100); assert(eyes_state().dizzyUntilMs == 0);
	eyes_faces_tap(200, 120, 140); eyes_faces_double(300, 120, 140); eyes_faces_pet(400, 120, 140);
	assert(eyes_config().expression == TIC_TAC_TOE);
	eyes_faces_greet(500); assert(eyes_config().expression == TIC_TAC_TOE);
	settings_defaults(); assert(face_filter().count() == EXPRESSION_COUNT && settings_faces_dirty());
	assert(settings_save() && settings_init() && face_filter().count() == EXPRESSION_COUNT);
	test_nvs["mochi/faces"][2] ^= 1;
	assert(settings_init() && face_filter().count() == EXPRESSION_COUNT);
	test_storage_ready = false; assert(!settings_init() && !settings_faces_save());
	test_storage_ready = true;
}
/** @brief Exclusion immediately clears an outgoing pose/backdrop and cancels excluded dizzy. */
static void rendering() {
	assert(settings_init()); Arduino_GFX gfx(240, 280); eyes_init(&gfx);
	settings_defaults(); settings_apply();
	eyes_set_expression(MECHA); eyes_update(1000); eyes_update(2000);
	eyes_imu_trigger_shake(2100); assert(eyes_state().dizzyUntilMs != 0);
	assert(settings_faces_update(only(HAPPY)));
	assert(eyes_state().dizzyUntilMs == 0 && !eyes_state().backdrop.initialized);
	for (uint32_t t = 2200; t < 13000; t += 33) {
		test_now = t; eyes_update(t);
		assert(eyes_config().expression == HAPPY && eyes_life_frame().expression == HAPPY);
	}
	settings_defaults();
}
int main() {
	formats(); pools(); persistence(); rendering();
	std::cout << "Face exclusions, one-face pools, NVS failure/migration, touch/IMU and raster guards passed\n";
}
