/**
 * @file ota_boot.cpp
 * @brief Override Arduino's early OTA confirmation when bootloader rollback is available.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ota_boot.h"
#include <sdkconfig.h>
#include <esp_ota_ops.h>
#include <esp_log.h>

#if defined(CONFIG_APP_ROLLBACK_ENABLE) && CONFIG_APP_ROLLBACK_ENABLE
/** @brief Preserve PENDING_VERIFY through Arduino initialization. @return Deferred verification. */
extern "C" bool verifyRollbackLater() { return true; }
#endif

void ota_boot_poll(uint32_t now, bool healthy) {
#if defined(CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE) && CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE
	static OtaBootHealth health;
	static bool checked = false;
	if (checked || !health.ready(now, healthy)) { return; }
	esp_ota_img_states_t state;
	const esp_partition_t *running = esp_ota_get_running_partition();
	esp_err_t result = running ? esp_ota_get_state_partition(running, &state) : ESP_FAIL;
	checked = true;
	if (result != ESP_OK) { ESP_LOGE("ota-boot", "Cannot read running image state: %d", int(result)); return; }
	if (state != ESP_OTA_IMG_PENDING_VERIFY) { return; }
	result = esp_ota_mark_app_valid_cancel_rollback();
	if (result != ESP_OK) { ESP_LOGE("ota-boot", "Cannot confirm healthy image: %d", int(result)); }
#else
	(void)now;
	(void)healthy;
#endif
}
