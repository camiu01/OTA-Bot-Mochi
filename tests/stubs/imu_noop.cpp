/**
 * @file imu_noop.cpp
 * @brief UI/settings tests isolate I2C; imu_test exercises the real sensor driver.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <stdint.h>
/** @brief Skip I2C during settings/UI tests. @param now Timestamp. */
void eyes_imu_poll(uint32_t now) { (void)now; }
/** @brief Report an intentionally absent sensor in UI-only tests. @return False. */
bool eyes_imu_ready() { return false; }
