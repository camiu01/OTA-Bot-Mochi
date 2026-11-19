/**
 * @file drive_meter.h
 * @brief Gyro-fused lean, G-meter and trip peaks for the vehicle dashboard.
 *
 * The first stable sample after entry (or ZERO) defines "up" and the gyro bias;
 * screen X projected onto the horizontal plane is "right" and up x right is
 * "ahead", so the frame follows any dash or handlebar mount. Lean integrates the
 * gyro roll rate and is pulled toward a reference: during a turn the ratio of
 * the body-right to body-up yaw components (tan(lean) for a coordinated turn,
 * independent of speed); on straights the accelerometer tilt. Pitch stays
 * accelerometer-only.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include <math.h>

struct DriveMeter {
	static constexpr uint8_t HISTORY = 96;	/**< Graph samples, two IMU polls each. */
	static constexpr float DEG = 57.29578f;
	bool active = false, tared = false;
	float up[3] = {}, right[3] = {}, ahead[3] = {}, bias[3] = {};
	float lateral = 0, longitudinal = 0, roll = 0, pitch = 0;	/**< g, g, deg (+right down), deg (+nose up). */
	float peak_g = 0, max_left = 0, max_right = 0, max_accel = 0, max_brake = 0;
	int8_t roll_trace[HISTORY] = {}, g_trace[HISTORY] = {};	/**< Degrees and lateral centi-g / 2. */
	uint8_t head = 0, filled = 0, phase = 0;
	uint32_t revision = 0, started = 0, last = 0;

	/** @brief Forget the reference, peaks, trip time and graph; the next stable sample re-zeroes. */
	void zero() { bool visible = active; uint32_t next = revision + 1; *this = DriveMeter{}; active = visible; revision = next; }

	/**
	 * @brief Feed one flip-corrected sample.
	 * @param a Acceleration in g. @param w Angular rate in rad/s (zeros without a gyro). @param now Time in ms.
	 * @return Visible values changed.
	 */
	bool sample(const float a[3], const float w[3], uint32_t now) {
		float energy = dot(a, a);
		if (!isfinite(energy) || !isfinite(dot(w, w))) { return false; }
		if (!tared) { return tare(a, w, energy, now); }
		float dt = float(uint32_t(now - last)) / 1000.0f;
		last = now;
		if (dt > 0.2f) { dt = 0.2f; }
		const float d[3] = {a[0] - up[0], a[1] - up[1], a[2] - up[2]};
		const float r[3] = {w[0] - bias[0], w[1] - bias[1], w[2] - bias[2]};
		smooth(lateral, dot(d, right), 0.35f);
		smooth(longitudinal, dot(d, ahead), 0.35f);
		float vertical = dot(a, up), yaw = dot(r, up);
		float reference = fabsf(yaw) > 0.15f ? atanf(-dot(r, right) / yaw) * DEG :
			-atan2f(dot(a, right), vertical) * DEG;
		roll = 0.98f * (roll + dot(r, ahead) * DEG * dt) + 0.02f * reference;
		smooth(pitch, atan2f(dot(a, ahead), vertical) * DEG, 0.25f);
		peaks();
		if (++phase >= 2) { phase = 0; record(); }
		++revision;
		return true;
	}

	/** @brief Read a graph point oldest-first. @param i Index below filled. @param g Lateral trace instead of roll. @return Stored value. */
	int8_t trace(uint8_t i, bool g) const {
		uint8_t at = uint8_t((head + HISTORY - filled + i) % HISTORY);
		return g ? g_trace[at] : roll_trace[at];
	}
	/** @brief Elapsed trip time since the reference was taken. @param now Time. @return Seconds, zero before tare. */
	uint32_t trip_seconds(uint32_t now) const { return tared ? uint32_t(now - started) / 1000 : 0; }

private:
	static float dot(const float *a, const float *b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
	static void smooth(float &value, float target, float alpha) { value += alpha * (target - value); }
	static int8_t clamp8(float v) { return int8_t(v > 100 ? 100 : (v < -100 ? -100 : v)); }

	/** @brief Update session extremes. */
	void peaks() {
		float g = sqrtf(lateral * lateral + longitudinal * longitudinal);
		if (g > peak_g) { peak_g = g; }
		if (roll > max_right) { max_right = roll; }
		if (-roll > max_left) { max_left = -roll; }
		if (longitudinal > max_accel) { max_accel = longitudinal; }
		if (-longitudinal > max_brake) { max_brake = -longitudinal; }
	}
	/** @brief Accept a resting reference, gyro bias and the right/ahead axes. @param a Accel. @param w Gyro. @param energy Squared norm. @param now Time. @return Reference set. */
	bool tare(const float *a, const float *w, float energy, uint32_t now) {
		if (energy < 0.64f || energy > 1.44f || dot(w, w) > 0.01f) { return false; }
		float n = sqrtf(energy);
		for (uint8_t i = 0; i < 3; ++i) { up[i] = a[i] / n; bias[i] = w[i]; }
		// Screen X is "right" unless the panel is on its side; then fall back to screen Y.
		float axis[3] = {1, 0, 0};
		if (fabsf(up[0]) > 0.9f) { axis[0] = 0; axis[1] = 1; }
		float k = dot(axis, up);
		for (uint8_t i = 0; i < 3; ++i) { right[i] = axis[i] - k * up[i]; }
		float length = sqrtf(dot(right, right));
		for (float &v : right) { v /= length; }
		ahead[0] = up[1] * right[2] - up[2] * right[1];
		ahead[1] = up[2] * right[0] - up[0] * right[2];
		ahead[2] = up[0] * right[1] - up[1] * right[0];
		tared = true; started = last = now; ++revision;
		return true;
	}
	/** @brief Append the current roll and lateral G to the ring. */
	void record() {
		roll_trace[head] = clamp8(roll);
		g_trace[head] = clamp8(lateral * 50);
		head = uint8_t((head + 1) % HISTORY);
		if (filled < HISTORY) { ++filled; }
	}
};

/** @brief Main-task state shared by IMU polling and the dashboard view. @return Session meter. */
inline DriveMeter &drive_meter() { static DriveMeter meter; return meter; }
