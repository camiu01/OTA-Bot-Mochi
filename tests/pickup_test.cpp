/**
 * @file pickup_test.cpp
 * @brief Pickup heuristic: stable arming, motion, changed pose, cooldown and rollover.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/eyes/pickup_policy.h"
#include <cassert>
#include <limits>
#include <cstdio>
#include <initializer_list>

/** @brief Replay one stable rest, impulse and quiet changed pose at 50ms cadence. */
static unsigned lift(PickupPolicy &p, uint32_t start, bool enabled = true, bool changed = true) {
	unsigned events = 0;
	for (uint32_t t = 0; t <= 2000; t += 50) {
		float x = 0, z = 1;
		if (t == 1400 || t == 1450) { z = 1.3f; }
		if (changed && t >= 1500) { x = 0.4f; z = sqrtf(0.84f); }
		events += p.update(x, 0, z, start + t, enabled);
	}
	return events;
}

int main() {
	for (uint32_t start : {0u, UINT32_MAX - 1600}) {
		PickupPolicy p;
		assert(lift(p, start) == 1);
		assert(lift(p, start + 2050) == 0);
		assert(lift(p, start + 8050) == 1);
	}
	PickupPolicy p;
	assert(lift(p, 0, false) == 0);
	p.reset(); assert(lift(p, 0, true, false) == 0);
	p.reset();
	for (uint32_t t = 0; t < 2000; t += 50) { assert(!p.update(0, 0, 1, t, true)); }
	assert(!p.update(0, 0, std::numeric_limits<float>::quiet_NaN(), 2000, true));
	assert(!p.update(0.4f, 0, sqrtf(0.84f), 2050, true));
	p.reset();
	for (uint32_t t = 0; t < 1400; t += 50) { assert(!p.update(0, 0, 1, t, true)); }
	assert(!p.update(0, 0, 1.3f, 1400, true));
	for (uint32_t t = 1700; t < 2300; t += 50) { assert(!p.update(0.4f, 0, sqrtf(0.84f), t, true)); }
	p.reset();
	for (uint32_t t = 0; t < 2000; t += 50) {
		float a = float(t) / 4000;
		assert(!p.update(sinf(a), 0, cosf(a), t, true));
	}
	p.reset();
	for (uint32_t t = 0; t < 1400; t += 50) { assert(!p.update(0, 0, 1, t, true)); }
	assert(!p.update(0, 0, 3, 1400, true));
	for (uint32_t t = 1450; t < 2300; t += 50) { assert(!p.update(0.4f, 0, sqrtf(0.84f), t, true)); }
	std::puts("Pickup rest/impulse/pose, cooldown, invalid input, gap, bump, tilt and rollover passed");
}
