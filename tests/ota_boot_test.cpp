/**
 * @file ota_boot_test.cpp
 * @brief Verify deferred OTA acceptance, continuous health, rollover, and disabled rollback builds.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/update/ota_boot.h"
#include <esp_ota_ops.h>
#include <cassert>

#if CONFIG_APP_ROLLBACK_ENABLE
extern "C" bool verifyRollbackLater();
#endif

/** @brief Require a complete healthy interval after every failure, including timestamp rollover. */
static void health_checks() {
	OtaBootHealth health;
	assert(!health.ready(0, false));
	assert(!health.ready(100, true));
	assert(!health.ready(15099, true));
	assert(health.ready(15100, true));
	assert(!health.ready(15101, false));
	assert(!health.ready(16000, true));
	assert(!health.ready(30999, true));
	assert(health.ready(31000, true));
	OtaBootHealth rollover;
	assert(!rollover.ready(UINT32_MAX - 100, true));
	assert(!rollover.ready(14898, true));
	assert(rollover.ready(14899, true));
}

/** @brief Exercise the production confirmation hook without a bootloader. @return Success. */
int main() {
	health_checks();
#if CONFIG_APP_ROLLBACK_ENABLE
	assert(verifyRollbackLater());
#endif
	ota_boot_poll(100, false);
	ota_boot_poll(200, true);
	ota_boot_poll(15199, true);
	assert(test_confirmations == 0);
	ota_boot_poll(15200, true);
#if CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE
	assert(test_confirmations == 1);
#else
	assert(test_confirmations == 0);
#endif
	ota_boot_poll(60000, true);
	assert(test_confirmations <= 1);
}
