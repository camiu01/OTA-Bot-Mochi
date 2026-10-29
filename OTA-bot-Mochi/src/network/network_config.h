/**
 * @file network_config.h
 * @brief Independent main-task network preferences; secrets never enter public state.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

struct NetworkConfig {
	bool enabled = false, weather = false;
	uint8_t zone = 1;
	char ssid[33] = {}, password[65] = {}, server[64] = "pool.ntp.org", city[65] = {};
	int32_t latitude = 0, longitude = 0;
};
/** @brief Load an independent versioned record, retaining defaults on failure. */
bool network_config_init();
/** @brief Main-task access only; never serialize password into a response. */
const NetworkConfig &network_config();
/** @brief Validate the complete candidate without radio or flash activity. */
bool network_config_valid(const NetworkConfig &value);
/** @brief Persist before applying; failures leave the previous configuration intact. */
bool network_config_save(const NetworkConfig &value);
/** @brief Has a valid record been loaded or saved, including an explicit disabled record? */
bool network_config_exists();
/** @brief Revision for optimistic concurrency and invalidating old worker replies. */
uint32_t network_config_revision();
