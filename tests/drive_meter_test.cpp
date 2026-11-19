/**
 * @file drive_meter_test.cpp
 * @brief Mount-relative tare, gyro-fused lean, G signs, trip peaks and graph ring regressions.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/play/drive_meter.h"
#include <assert.h>
#include <iostream>

static bool near(float a, float b, float eps = 0.02f) { return fabsf(a - b) < eps; }
static const float still[3] = {0, 0, 0};
static uint32_t clock_ms = 0;

/** @brief Feed one 50 ms poll in the flip-corrected screen frame. */
static bool feed(DriveMeter &m, float x, float y, float z, const float *w = still) {
	const float a[3] = {x, y, z};
	clock_ms += 50;
	return m.sample(a, w, clock_ms);
}

/** @brief Static tilt, G axes and peaks without gyro motion. */
static void accelerometer_checks() {
	DriveMeter flat;
	assert(!feed(flat, 0, 0, 2) && !flat.tared);	// Free fall/impact never becomes the reference.
	assert(!feed(flat, NAN, 0, 1) && !flat.tared);
	const float spinning[3] = {0.3f, 0, 0};
	assert(!feed(flat, 0, 0, 1, spinning) && !flat.tared);	// Moving gyro never becomes the bias.
	assert(feed(flat, 0, 0, 1) && flat.tared);
	for (int i = 0; i < 400; ++i) { feed(flat, 0.5f, 0, 1); }
	assert(near(flat.lateral, 0.5f) && near(flat.longitudinal, 0));
	assert(near(flat.roll, -26.565f, 0.2f) && flat.max_left > 26 && flat.max_right == 0);
	assert(near(flat.peak_g, 0.5f));
	for (int i = 0; i < 60; ++i) { feed(flat, 0, 0.3f, 1); }	// Screen top is "ahead" when lying flat.
	assert(near(flat.longitudinal, 0.3f) && near(flat.lateral, 0) && near(flat.max_accel, 0.3f));
	assert(near(flat.pitch, 16.7f, 0.1f));
	for (int i = 0; i < 60; ++i) { feed(flat, 0, -0.6f, 1); }
	assert(near(flat.max_brake, 0.6f));
	assert(flat.trip_seconds(clock_ms) == (520 * 50 + 50) / 1000 - 0);
	flat.active = true;
	uint32_t revision = flat.revision;
	flat.zero();
	assert(flat.active && !flat.tared && flat.filled == 0 && flat.peak_g == 0 && flat.revision > revision);
	assert(flat.trip_seconds(clock_ms) == 0);

	DriveMeter upright;	// Screen faces the rider: forward is the panel's -Z.
	feed(upright, 0, 1, 0);
	for (int i = 0; i < 60; ++i) { feed(upright, 0, 1, -0.3f); }
	assert(near(upright.longitudinal, 0.3f) && near(upright.lateral, 0));

	DriveMeter side;	// Panel on its side: screen X is vertical, so Y becomes "right".
	feed(side, 1, 0, 0);
	feed(side, 1, 0.4f, 0);
	assert(isfinite(side.lateral) && side.lateral > 0 && isfinite(side.roll));
}

/** @brief A coordinated turn reads true lean even though gravity looks vertical. */
static void gyro_checks() {
	for (float sign : {1.0f, -1.0f}) {
		DriveMeter bike;	// Upright handlebar mount: up +Y, right +X, ahead -Z.
		feed(bike, 0, 1, 0);
		float lean = sign * 0.5236f, yaw = 0.5f;	// 30 degrees, steady turn toward the lean.
		// Body components of a world yaw rate toward the lean: up gets -sign*cos, right gets sign*sin.
		const float w[3] = {sign * yaw * sinf(lean), -sign * yaw * cosf(lean), 0};
		for (int i = 0; i < 400; ++i) { feed(bike, 0, 1 / cosf(lean), 0, w); }
		assert(near(bike.roll, sign * 30, 0.5f) && near(bike.lateral, 0, 0.05f));
		assert(sign > 0 ? bike.max_right > 29 : bike.max_left > 29);
	}
	DriveMeter flick;
	feed(flick, 0, 1, 0);
	const float roll_rate[3] = {0, 0, -1};	// +1 rad/s about "ahead" (-Z) tips the right side down.
	for (int i = 0; i < 10; ++i) { feed(flick, 0, 1, 0, roll_rate); }
	assert(flick.roll > 20 && flick.roll < 29);
	const float biased[3] = {0.05f, 0.05f, 0.05f};
	DriveMeter drift;
	feed(drift, 0, 1, 0, biased);
	for (int i = 0; i < 400; ++i) { feed(drift, 0, 1, 0, biased); }
	assert(near(drift.roll, 0, 0.1f));	// Tare captures bias.
}

/** @brief Graph keeps one point per two samples and wraps oldest-first. */
static void graph_checks() {
	DriveMeter fresh;
	feed(fresh, 0, 0, 1);
	for (int i = 0; i < 60; ++i) { feed(fresh, 0, 0, 1); }
	assert(fresh.filled == 30);
	for (int i = 0; i < 250; ++i) { feed(fresh, i < 200 ? 0.0f : 0.5f, 0, 1); }
	assert(fresh.filled == DriveMeter::HISTORY && fresh.trace(0, true) == 0 &&
		fresh.trace(DriveMeter::HISTORY - 1, true) >= 24 && fresh.trace(DriveMeter::HISTORY - 1, false) < 0);
}

int main() {
	accelerometer_checks();
	gyro_checks();
	graph_checks();
	std::cout << "drive meter tests passed\n";
	return 0;
}
