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
#include "favorites_checks.h"

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
	for (std::string bad : {std::string(FaceFilter::BYTES * 2, '0'), std::string(FaceFilter::BYTES * 2, 'g'), std::string("xyz")}) {
		assert(!decoded.parse_hex(bad.c_str()) && decoded == f);
	}
	uint8_t record[FaceFilter::RECORD_BYTES]; f.encode(record);
	assert(decoded.decode(record, sizeof(record)) && decoded == f);
	assert(!decoded.decode(record, sizeof(record) - 1));
	uint8_t previous[20] = {1, 136};
	previous[2 + TIC_TAC_TOE / 8] = uint8_t(1U << (TIC_TAC_TOE % 8));
	previous[sizeof(previous) - 1] = 0xA5;
	for (size_t i = 0; i < sizeof(previous) - 1; ++i) { previous[sizeof(previous) - 1] ^= previous[i]; }
	assert(decoded.decode(previous, sizeof(previous)) && decoded.allows(MECHA_BREAK));
	assert(decoded.count() == EXPRESSION_COUNT - 136 + 1 && decoded.allows(TIC_TAC_TOE));
	uint8_t prior[21] = {1, 137};
	prior[2 + TIC_TAC_TOE / 8] = uint8_t(1U << (TIC_TAC_TOE % 8));
	prior[sizeof(prior) - 1] = 0xA5;
	for (size_t i = 0; i < sizeof(prior) - 1; ++i) { prior[sizeof(prior) - 1] ^= prior[i]; }
	assert(decoded.decode(prior, sizeof(prior)) && decoded.count() == EXPRESSION_COUNT - 137 + 1);
	assert(decoded.allows(TIC_TAC_TOE) && !decoded.allows(MECHA_BREAK));
	for (uint8_t id = HOLLOW_ANGER; id < EXPRESSION_COUNT; ++id) { assert(decoded.allows(Expression(id))); }
	// ID 144 crosses a byte boundary; keep old exclusions and enable appended IDs.
	uint8_t last[21] = {1, 144};
	last[2 + TIC_TAC_TOE / 8] = uint8_t(1U << (TIC_TAC_TOE % 8));
	last[sizeof(last) - 1] = 0xA5;
	for (size_t i = 0; i < sizeof(last) - 1; ++i) { last[sizeof(last) - 1] ^= last[i]; }
	assert(decoded.decode(last, sizeof(last)) && decoded.count() == EXPRESSION_COUNT - 144 + 1);
	assert(decoded.allows(TIC_TAC_TOE) && decoded.allows(CHINESE_DRAGON) && !decoded.allows(YAKURA));
	assert(decoded.decode(record, sizeof(record)) && decoded == f);
	record[2] ^= 1; assert(!decoded.decode(record, sizeof(record)) && decoded == f);
	uint8_t old[] = {1, 8, 1, uint8_t(0xA5 ^ 1 ^ 8 ^ 1)};
	assert(decoded.decode(old, sizeof(old)));
	assert(decoded.allows(NEUTRAL) && !decoded.allows(HAPPY) && decoded.allows(TIC_TAC_TOE));
	old[2] = 0; old[3] = 0xA5 ^ 1 ^ 8;
	assert(!decoded.decode(old, sizeof(old)));
}
/** @brief Reserved high bits are rejected before either hexadecimal or NVS state is assigned. */
static void reserved_bits() {
	static_assert(EXPRESSION_COUNT == 172 && FaceFilter::BYTES == 22 && FaceFilter::RECORD_BYTES == 25,
		"The four final comedy faces own only the low four bits of the final mask byte");
	FaceFilter f, decoded;
	char hex[FaceFilter::BYTES * 2 + 1]; f.hex(hex);
	assert(hex[FaceFilter::BYTES * 2 - 2] == '0');
	hex[FaceFilter::BYTES * 2 - 2] = '1';
	assert(!decoded.parse_hex(hex) && decoded == f);
	uint8_t record[FaceFilter::RECORD_BYTES]; f.encode(record);
	record[FaceFilter::RECORD_BYTES - 2] |= 16;
	record[FaceFilter::RECORD_BYTES - 1] ^= 16;
	assert(!decoded.decode(record, sizeof(record)) && decoded == f);
}
/** @brief A 145-face record keeps exclusions and enables Angel and all subsequently appended IDs. */
static void angel_migration() {
	uint8_t previous[22] = {1, 145};
	previous[2 + HALO / 8] = uint8_t(1U << (HALO % 8));
	previous[sizeof(previous) - 1] = 0xA5;
	for (size_t i = 0; i < sizeof(previous) - 1; ++i) { previous[sizeof(previous) - 1] ^= previous[i]; }
	FaceFilter decoded;
	assert(decoded.decode(previous, sizeof(previous)) && decoded.count() == EXPRESSION_COUNT - 145 + 1);
	assert(decoded.allows(HALO) && decoded.allows(ANGEL) && !decoded.allows(DEVIL));
	assert(!decoded.allows(CHINESE_DRAGON));
}
/** @brief Crossing the 152-ID byte boundary preserves an actual previous 146-face record. */
static void cinematic_migration() {
	uint8_t previous[22] = {1, 146};
	previous[2 + HAPPY / 8] = uint8_t(1U << (HAPPY % 8));
	previous[21] = 0xA5;
	for (size_t i = 0; i < 21; ++i) { previous[21] ^= previous[i]; }
	FaceFilter decoded;
	assert(decoded.decode(previous, sizeof(previous)) && decoded.count() == EXPRESSION_COUNT - 146 + 1);
	for (uint8_t id = 0; id < 146; ++id) { assert(decoded.allows(Expression(id)) == (id == HAPPY)); }
	for (uint8_t id = HACKER; id < EXPRESSION_COUNT; ++id) { assert(decoded.allows(Expression(id))); }
	uint8_t current[FaceFilter::RECORD_BYTES]; decoded.encode(current);
	assert(current[0] == 1 && current[1] == EXPRESSION_COUNT);
	FaceFilter restored; assert(restored.decode(current, sizeof(current)) && restored == decoded);
}
/** @brief The installed 154-face record retains exclusions and enables only appended additions. */
static void anime_migration() {
	uint8_t previous[23] = {1, 154};
	previous[2 + PIXEL_CRASH / 8] = uint8_t(1U << (PIXEL_CRASH % 8));
	previous[22] = 0xA5;
	for (size_t i = 0; i < 22; ++i) { previous[22] ^= previous[i]; }
	FaceFilter decoded;
	assert(decoded.decode(previous, sizeof(previous)) && decoded.count() == EXPRESSION_COUNT - 154 + 1);
	for (uint8_t id = 0; id < 154; ++id) { assert(decoded.allows(Expression(id)) == (id == PIXEL_CRASH)); }
	for (uint8_t id = POWER_UP; id < EXPRESSION_COUNT; ++id) { assert(decoded.allows(Expression(id))); }
	uint8_t record[FaceFilter::RECORD_BYTES]; decoded.encode(record);
	FaceFilter restored; assert(restored.decode(record, sizeof(record)) && restored == decoded);
}
/** @brief Growing beyond 168 IDs preserves the old anime catalog's exclusions. */
static void comedy_migration() {
	uint8_t previous[24] = {1, 166};
	previous[2 + CHIBI_JOY / 8] = uint8_t(1U << (CHIBI_JOY % 8));
	previous[23] = 0xA5;
	for (size_t i = 0; i < 23; ++i) { previous[23] ^= previous[i]; }
	FaceFilter decoded;
	assert(decoded.decode(previous, sizeof(previous)) && decoded.count() == 7);
	for (uint8_t id = 0; id < 166; ++id) { assert(decoded.allows(Expression(id)) == (id == CHIBI_JOY)); }
	for (uint8_t id = RUBBER_BAND; id < EXPRESSION_COUNT; ++id) { assert(decoded.allows(Expression(id))); }
	uint8_t record[FaceFilter::RECORD_BYTES]; decoded.encode(record);
	FaceFilter restored; assert(restored.decode(record, sizeof(record)) && restored == decoded);
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
	formats(); reserved_bits(); angel_migration(); cinematic_migration(); anime_migration(); comedy_migration();
	pools(); persistence(); rendering(); favorites_formats(); favorites_persistence();
	std::cout << "Face exclusions, one-face pools, NVS failure/migration, touch/IMU and raster guards passed\n";
}
