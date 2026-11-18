/**
 * @file obd_ble.h
 * @brief Optional BLE ELM327 client; scans only while the drive page is visible.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

/** @brief Start the low-priority worker after NimBLE is initialized. @return Worker running. */
bool obd_begin();
