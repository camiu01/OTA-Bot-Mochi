/**
 * @file eyes_imu.cpp
 * @brief QMI8658C accelerometer polling, tilt mapping and shake detection.
 */
#include "eyes_api.h"
#include "../../pin_config.h"
#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#define QMI_ADDR 0x6B	/**< QMI8658C I2C address. */
#define QMI_REG_WHOAMI 0x00	/**< Chip ID register. */
#define QMI_REG_CTRL1 0x02	/**< Accelerometer control register. */
#define QMI_REG_CTRL7 0x08	/**< Sensor enable register. */
#define QMI_REG_AX_L 0x35	/**< First accelerometer data register. */
#define QMI_POLL_MS 50	/**< Minimum interval between IMU reads. */
#define QMI_TILT_MAX 10	/**< Maximum pupil tilt offset in pixels. */
#define QMI_SHAKE_G 2.2f	/**< Acceleration magnitude triggering DIZZY. */

static bool s_imuOk = false;
static uint32_t s_lastPoll = 0;
static float s_smoothX = 0.0f;
static float s_smoothY = 0.0f;

/**
 * @brief Write one byte to a QMI8658C register.
 * @param reg Target register address.
 * @param val Value to write.
 */
static void imu_write(uint8_t reg, uint8_t val) {
	Wire.beginTransmission((uint8_t)QMI_ADDR);
	Wire.write(reg);
	Wire.write(val);
	Wire.endTransmission();
}

/**
 * @brief Read raw accelerometer axes from the sensor.
 * @param ax Output raw X axis value.
 * @param ay Output raw Y axis value.
 * @param az Output raw Z axis value.
 * @return True on successful 6-byte read, false on I2C error.
 */
static bool imu_read_acc(int16_t *ax, int16_t *ay, int16_t *az) {
	Wire.beginTransmission((uint8_t)QMI_ADDR);
	Wire.write((uint8_t)QMI_REG_AX_L);
	if (Wire.endTransmission(false) != 0) {
		return false;
	}
	if (Wire.requestFrom((uint8_t)QMI_ADDR, (uint8_t)6) != 6) {
		return false;
	}
	uint8_t xl = Wire.read();
	uint8_t xh = Wire.read();
	uint8_t yl = Wire.read();
	uint8_t yh = Wire.read();
	uint8_t zl = Wire.read();
	uint8_t zh = Wire.read();
	*ax = (int16_t)((xh << 8) | xl);
	*ay = (int16_t)((yh << 8) | yl);
	*az = (int16_t)((zh << 8) | zl);
	return true;
}

/**
 * @brief Clamp a smoothed tilt value to the pixel range.
 * @param v Smoothed tilt value in pixels.
 * @return Clamped offset within [-QMI_TILT_MAX, QMI_TILT_MAX].
 */
static int16_t clamp_tilt(float v) {
	if (v > QMI_TILT_MAX) {
		return QMI_TILT_MAX;
	}
	if (v < -QMI_TILT_MAX) {
		return -QMI_TILT_MAX;
	}
	return (int16_t)v;
}

/**
 * @brief Init QMI8658C on QMI_SDA/QMI_SCL.
 */
void eyes_imu_init() {
	Wire.begin(QMI_SDA, QMI_SCL);
	imu_write(QMI_REG_CTRL1, 0x60);
	imu_write(QMI_REG_CTRL7, 0x03);
	s_imuOk = true;
	s_lastPoll = millis();
}

/**
 * @brief Set pupil offset from tilt in pixels.
 * @param tx Horizontal offset.
 * @param ty Vertical offset.
 */
void eyes_imu_set_tilt(int16_t tx, int16_t ty) {
	EyeState &st = eyes_state();
	st.tiltX = tx;
	st.tiltY = ty;
}

/**
 * @brief Force DIZZY expression for 2s.
 * @param nowMs Value of millis() at call time.
 */
void eyes_imu_trigger_shake(uint32_t nowMs) {
	EyeState &st = eyes_state();
	EyeConfig &cfg = eyes_config();
	if (st.dizzyUntilMs != 0) {
		st.dizzyUntilMs = nowMs + 2000;
		return;
	}
	st.savedExpr = cfg.expression;
	cfg.expression = DIZZY;
	st.dizzyUntilMs = nowMs + 2000;
}

/**
 * @brief Poll accelerometer, drive tilt and shake-to-dizzy.
 * @param nowMs Value of millis() at call time.
 */
void eyes_imu_poll(uint32_t nowMs) {
	if (!s_imuOk) {
		return;
	}
	if ((uint32_t)(nowMs - s_lastPoll) < QMI_POLL_MS) {
		return;
	}
	s_lastPoll = nowMs;
	int16_t ax = 0;
	int16_t ay = 0;
	int16_t az = 0;
	if (!imu_read_acc(&ax, &ay, &az)) {
		return;
	}
	float fx = (float)ax / 2048.0f;
	float fy = (float)ay / 2048.0f;
	float fz = (float)az / 2048.0f;
	float mag = sqrtf(fx * fx + fy * fy + fz * fz);
	if (mag > QMI_SHAKE_G) {
		eyes_imu_trigger_shake(nowMs);
		return;
	}
	s_smoothX = s_smoothX * 0.8f + fx * 0.2f;
	s_smoothY = s_smoothY * 0.8f + fy * 0.2f;
	eyes_imu_set_tilt(clamp_tilt(s_smoothX * 8.0f), clamp_tilt(-s_smoothY * 6.0f));
}
