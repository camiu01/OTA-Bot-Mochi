#pragma once
#include "face_runtime.h"
#include "../play/mini_widgets.h"
#include <math.h>
#include <string.h>

namespace face_primitives {
/** @brief Shared strip drawing primitive. */
void strip(Arduino_GFX *gfx, const EyeBox &b, int16_t height, bool bottom, uint16_t color);
/** @brief Shared slope drawing primitive. */
void slope(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, bool angry, int16_t depth);
/** @brief Shared smile drawing primitive. */
void smile(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t depth, int16_t rise);
/** @brief Shared heart drawing primitive. */
void heart(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared cross drawing primitive. */
void cross(Arduino_GFX *gfx, const EyeBox &b, uint16_t color);
/** @brief Shared star drawing primitive. */
void star(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared spiral drawing primitive. */
void spiral(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared tear drawing primitive. */
void tear(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared tear_y drawing primitive. */
int16_t tear_y(const EyeBox &b);
/** @brief Shared annulus drawing primitive. */
void annulus(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared halo drawing primitive. */
void halo(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared horns drawing primitive. */
void horns(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared flame drawing primitive. */
void flame(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared crescent drawing primitive. */
void crescent(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut);
/** @brief Shared sparkle drawing primitive. */
void sparkle(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared bolt drawing primitive. */
void bolt(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared rhombus drawing primitive. */
void rhombus(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared crosshair drawing primitive. */
void crosshair(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared money drawing primitive. */
void money(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared glitch_glyph drawing primitive. */
void glitch_glyph(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared hourglass drawing primitive. */
void hourglass(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared low_batt drawing primitive. */
void low_batt(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared music_note drawing primitive. */
void music_note(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared power_sym drawing primitive. */
void power_sym(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared exclaim drawing primitive. */
void exclaim(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
/** @brief Shared question drawing primitive. */
void question(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color);
}
