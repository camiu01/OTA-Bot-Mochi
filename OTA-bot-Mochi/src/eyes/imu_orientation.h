/**
 * @file imu_orientation.h
 * @brief Parked calibration and debounced 180-degree rotation.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include <math.h>

enum class ImuCalibration : uint8_t { Idle, Sampling, Ready, Failed };

class ImuOrientation {
	float reference[3] = {}, sum[3] = {}, first[3] = {};
	uint8_t count = 0;
	uint32_t started = 0, previous = 0, pending_since = 0;
	bool reference_flip = false, wanted = false, pending = false, tracking = false;

public:
	bool enabled = true, applied = false;
	bool persisted = false, save_failed = false;
	ImuCalibration status = ImuCalibration::Idle;

	/** @brief Start a one-second stable average. @param now Time. @param flip Current displayed orientation. */
	void calibrate(uint32_t now, bool flip) {
		status = ImuCalibration::Sampling;
		count = 0; started = previous = now;
		reference_flip = wanted = flip; tracking = false;
		for (float &v : sum) { v = 0; }
	}

	/** @brief Use manual mounting until calibrated; manual edits invalidate the old reference. @param manual Manual flip. @return Desired flip. */
	bool target(bool manual) {
		if (status == ImuCalibration::Ready && manual != manual_at_calibration) { status = ImuCalibration::Idle; }
		return enabled && status == ImuCalibration::Ready ? wanted : manual;
	}
	bool manual_at_calibration = false;
	/** @brief Read a calibrated raw gravity component. @param axis 0 X, 1 Y, 2 Z. @return Acceleration in g. */
	float gravity(uint8_t axis) const { return axis < 3 ? reference[axis] : 0; }
	/** @brief Read the orientation associated with the stored gravity vector. @return Reference flip. */
	bool reference_flipped() const { return reference_flip; }
	/** @brief Install a validated saved reference without changing the displayed orientation. @param axes Gravity in g. @param flip Reference orientation. @param manual Current manual setting. */
	void restore(const float axes[3], bool flip, bool manual) {
		for (uint8_t i = 0; i < 3; ++i) { reference[i] = axes[i]; }
		reference_flip = wanted = flip; manual_at_calibration = manual;
		status = ImuCalibration::Ready; persisted = true; save_failed = tracking = false;
	}
	/** @brief Expire calibration even if sensor reads stop arriving. @param now Time. */
	void tick(uint32_t now) {
		if (status == ImuCalibration::Sampling && uint32_t(now - started) >= 5000) { status = ImuCalibration::Failed; }
	}

	/** @brief Accumulate stable gravity or detect a sustained turnover, rejecting flat/side poses. @param x X g. @param y Y g. @param z Z g. @param now Time. @return Calibration just completed. */
	bool sample(float x, float y, float z, uint32_t now) {
		tick(now);
		float axes[] = {x, y, z};
		float energy = x * x + y * y + z * z;
		if (status == ImuCalibration::Sampling) {
			bool stable = isfinite(energy) && energy >= 0.81f && energy <= 1.21f;
			for (uint8_t i = 0; i < 3 && count; ++i) { stable &= fabsf(axes[i] - first[i]) < 0.06f; }
			if (!stable || uint32_t(now - previous) > 150) {
				count = 0;
				for (float &v : sum) { v = 0; }
			}
			previous = now;
			if (!stable) { return false; }
			if (!count) { for (uint8_t i = 0; i < 3; ++i) { first[i] = axes[i]; } }
			for (uint8_t i = 0; i < 3; ++i) { sum[i] += axes[i]; }
			if (++count < 20) { return false; }
			for (uint8_t i = 0; i < 3; ++i) { reference[i] = sum[i] / count; }
			status = ImuCalibration::Ready;
			persisted = false; save_failed = false;
			return true;
		}
		if (!enabled || status != ImuCalibration::Ready) { tracking = false; return false; }
		float xy = x * x + y * y, ref_xy = reference[0] * reference[0] + reference[1] * reference[1];
		float dot = (x * reference[0] + y * reference[1]) / sqrtf(xy * ref_xy);
		if (!isfinite(dot) || energy < 0.81f || energy > 1.21f || xy < 0.25f || ref_xy < 0.25f || fabsf(dot) < 0.85f) {
			tracking = false; return false;
		}
		bool candidate = reference_flip != (dot < 0);
		if (!tracking || pending != candidate || uint32_t(now - previous) > 150) {
			pending = candidate; pending_since = now; tracking = true;
		}
		previous = now;
		if (uint32_t(now - pending_since) >= 600) { wanted = candidate; }
		return false;
	}
};

/** @brief Shared main-task calibration/orientation policy. @return Session state. */
inline ImuOrientation &imu_orientation() {
	static ImuOrientation policy;
	return policy;
}
