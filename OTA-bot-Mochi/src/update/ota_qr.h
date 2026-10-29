/**
 * @file ota_qr.h
 * @brief Main-task-only cached Wi-Fi QR, encoded by the bundled ESP32 component.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <Arduino_GFX_Library.h>
/** @brief Cache a new session's Wi-Fi QR without console output. @param ssid Generated SSID. @param password Generated password. @return Encoding succeeded. */
bool ota_qr_prepare(const char *ssid, const char *password);
/** @brief Cache an IPv4 HTTP settings URL with its generated authorization fragment. */
bool ota_qr_prepare_url(const char *url);
/** @brief Draw black modules and a four-module white quiet zone at integer scale. @param gfx Canvas. @param x Box left. @param y Box top. @param side Available square pixels. @return QR fits and was drawn. */
bool ota_qr_draw(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t side);
