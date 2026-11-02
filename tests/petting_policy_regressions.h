/**
 * @file petting_policy_regressions.h
 * @brief Petting continuity, release timeout, priority and unsigned timestamp rollover.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

/** @brief Continued stroking keeps one face beyond six seconds and releases after the last stroke. */
inline void continued_pet_tests() {
	FacePolicy p;
	p.reset(NEUTRAL, 0);
	assert(p.pet(0, 0));
	Expression face = p.update(0, false, false, 0);
	for (uint32_t now : {1500u, 3000u, 4500u, 7000u}) {
		assert(!p.pet(now, 1));
		assert(p.update(now, false, false, 0) == face);
	}
	assert(p.update(9999, false, false, 0) == face);
	assert(p.update(10000, false, false, 0) == NEUTRAL);
	p.reset(NEUTRAL, UINT32_MAX - 1000);
	assert(p.pet(UINT32_MAX - 1000, 0));
	assert(!p.pet(500, 1));
	assert(p.update(3499, false, false, 0) != NEUTRAL);
	assert(p.update(3500, false, false, 0) == NEUTRAL);
	// Without an intervening update, an expired pet is still a fresh reaction.
	assert(p.pet(3500, 1));
	p.tap(4000, 0);
	assert(p.pet(4200, 0));
	assert(p.update(7199, false, false, 0) != NEUTRAL);
	assert(p.update(7200, false, false, 0) == NEUTRAL);
}

/** @brief Petting requests an affectionate face, avoids the base and holds priority over IMU moods. */
inline void pet_tests() {
	auto affectionate = [](Expression face) {
		return face == HAPPY || face == LOVE || face == GLEE ||
			face == BEAMING || face == MILD_HAPPY || face == EXCITED;
	};
	FacePolicy p;
	p.reset(NEUTRAL, 0);
	assert(p.pet(0, 0));
	Expression face = p.update(0, false, false, 0);
	assert(affectionate(face));
	assert(p.update(2999, false, false, 0) == face);
	assert(p.update(3000, false, false, 0) == NEUTRAL);
	p.reset(HAPPY, 0);
	assert(p.pet(0, 1));
	assert(p.update(0, false, false, 0) != HAPPY);
	p.reset(NEUTRAL, 0);
	assert(p.pet(0, 0));
	Expression held = p.update(0, false, true, 0);
	for (uint32_t now = 50; now <= 500; now += 50) { p.sample(0.4f, 0, 0, now, true); }
	assert(p.update(500, false, true, 0) == held);
	assert(p.update(3000, false, true, 0) == NEUTRAL);
	continued_pet_tests();
}
