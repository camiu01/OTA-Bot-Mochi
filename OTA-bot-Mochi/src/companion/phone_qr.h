/** @file phone_qr.h @brief Main-task QR rendering of bounded phone content using the bundled encoder. */
#pragma once
#include <Arduino_GFX_Library.h>
/** @brief Draw an exact payload of at most 96 bytes with a four-module quiet zone. @param gfx Canvas. @param value Payload. @return Rendered. */
bool phone_qr_draw(Arduino_GFX *gfx, const String &value);
