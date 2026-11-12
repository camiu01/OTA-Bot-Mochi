/**
 * @file favorites_checks.h
 * @brief Empty stars, strict records, catalog growth and independent explicit persistence.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
/** @brief Old stars never implicitly include appended IDs; corruption leaves the caller untouched. */
static void favorites_formats() {
	FaceFavorites stars, decoded;
	assert(stars.count() == 0 && !stars.contains(Expression(255)) && !stars.set(Expression(255), true));
	for (uint16_t id = 0; id < EXPRESSION_COUNT; ++id) { assert(stars.set(Expression(id), true)); }
	assert(stars.count() == EXPRESSION_COUNT);
	uint8_t record[FaceFavorites::RECORD_BYTES]; stars.encode(record);
	assert(decoded.decode(record, sizeof(record)) && decoded == stars);
	assert(!decoded.decode(nullptr, sizeof(record)) && !decoded.decode(record, sizeof(record) - 1));
	record[sizeof(record) - 2] |= 16; record[sizeof(record) - 1] ^= 16;
	assert(!decoded.decode(record, sizeof(record)) && decoded == stars);
	stars.encode(record); record[2] ^= 1;
	assert(!decoded.decode(record, sizeof(record)) && decoded == stars);
	uint8_t old[] = {1, 8, uint8_t(1U << HAPPY), uint8_t(0xA5 ^ 1 ^ 8 ^ (1U << HAPPY))};
	assert(decoded.decode(old, sizeof(old)) && decoded.count() == 1 && decoded.contains(HAPPY));
	assert(!decoded.contains(BALLOON_EYES) && decoded.set(HAPPY, false) && decoded.count() == 0);
	decoded.encode(record); assert(stars.decode(record, sizeof(record)) && stars.count() == 0);
}
/** @brief Star-only saves preserve dirty profiles/exclusions; failed writes remain retryable. */
static void favorites_persistence() {
	settings_defaults(); assert(settings_save());
	auto profile = test_profile; auto faces = test_nvs["mochi/faces"];
	auto p = settings_get(); p.brightness = 70; assert(settings_update(p));
	FaceFilter excluded; excluded.set(HAPPY, false); assert(settings_faces_update(excluded));
	FaceFavorites stars; stars.set(HAPPY, true); stars.set(BALLOON_EYES, true);
	unsigned writes = test_flash_writes; uint32_t revision = settings_revision();
	settings_favorites_update(stars); assert(settings_revision() == revision + 1);
	settings_favorites_update(stars); assert(settings_revision() == revision + 1);
	assert(settings_dirty() && settings_favorites_dirty() && test_flash_writes == writes);
	test_save_fails = true; assert(!settings_favorites_save() && settings_favorites_dirty());
	test_save_fails = false; assert(settings_favorites_save() && !settings_favorites_dirty() && settings_dirty());
	assert(settings_favorites_save() && test_flash_writes == writes + 1);
	assert(test_profile == profile && test_nvs["mochi/faces"] == faces);
	assert(settings_init() && settings_get().brightness == 30 && face_filter().allows(HAPPY));
	assert(settings_favorites() == stars && !settings_favorites_dirty());
	settings_defaults(); assert(settings_favorites_dirty() && settings_save() && settings_init());
	assert(settings_favorites().count() == 0);
	test_nvs["mochi/favorites"][2] ^= 1;
	assert(settings_init() && settings_favorites().count() == 0);
	test_storage_ready = false; assert(!settings_init() && !settings_favorites_save());
	test_storage_ready = true; assert(settings_init());
}
