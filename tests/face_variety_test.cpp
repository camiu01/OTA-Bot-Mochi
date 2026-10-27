/**
 * @file face_variety_test.cpp
 * @brief Continuous playback separates recent IDs and similar silhouettes without changing sequence order.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/face_policy.h"
#include <cassert>
#include <iostream>
#include <vector>

/** @brief Exercise adversarial repeated randomness, every base, and timestamp rollover. */
static void continuous(uint32_t choice) {
	for (uint8_t base = 0; base < EXPRESSION_COUNT; ++base) {
		FacePolicy p;
		uint32_t now = UINT32_MAX - 1000;
		p.reset(Expression(base), now - 12000);
		std::vector<Expression> seen;
		for (unsigned i = 0; i < 150; ++i) {
			Expression next = p.update(now, true, false, choice, true, FACE_CONTINUOUS);
			assert(next != NEUTRAL && next != DIZZY && next != Expression(base));
			assert(face_family(next) != face_family(Expression(base)));
			for (size_t j = 0; j < seen.size() && j < 8; ++j) {
				Expression prior = seen[seen.size() - 1 - j];
				assert(next != prior);
				if (j < 2) { assert(face_family(next) != face_family(prior)); }
			}
			uint32_t length = FacePolicy::cycle_duration(next);
			assert(p.update(now + length - 1, true, false, choice, false, FACE_CONTINUOUS) == next);
			seen.push_back(next); now += length;
		}
	}
}
/** @brief Ordered playback remains complete and uses the original enum order. */
static void sequence() {
	FacePolicy p; p.reset(NEUTRAL, 0);
	uint32_t now = 12000;
	for (uint8_t id = 1; id < EXPRESSION_COUNT; ++id) {
		if (id == DIZZY) { continue; }
		assert(p.update(now, true, false, 0, true, FACE_SEQUENCE) == Expression(id));
		now += FacePolicy::cycle_duration(Expression(id));
	}
}
/** @brief Repeated sensor reactions must not indefinitely starve continuous automatic playback. */
static void sensor_handoff(uint32_t start) {
	FacePolicy p; p.reset(NEUTRAL, start);
	Expression previous = NEUTRAL;
	unsigned automatic_changes = 0;
	for (uint32_t elapsed = 0; elapsed <= 120000; elapsed += 50) {
		uint32_t now = start + elapsed;
		p.sample(0.4f, 0, 0, now, true);
		Expression face = p.update(now, true, true, 0, true, FACE_CONTINUOUS);
		// This fixed X deflection can only produce SKEPTIC; it is not random progress.
		if (face != previous && face != SKEPTIC && p.automatic()) { ++automatic_changes; }
		previous = face;
	}
	assert(automatic_changes > 10);
	// Turning random playback off must still stop chaining, even after a sensor event.
	assert(p.update(start + 130000, false, false, 0, true, FACE_CONTINUOUS) == NEUTRAL);
}
int main() {
	for (uint32_t choice : {0u, 1u, 6u, 73u, UINT32_MAX}) { continuous(choice); }
	sequence();
	sensor_handoff(0); sensor_handoff(UINT32_MAX - 1000);
	assert(face_family(HAPPY) == face_family(MILD_HAPPY));
	assert(face_family(SLEEPY) == face_family(DOZE));
	assert(face_family(LOVE) == face_family(ADORE));
	std::cout << "Continuous variety, all bases, full cycles, sequence and sustained-IMU recovery passed\n";
}
