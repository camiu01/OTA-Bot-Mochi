/**
 * @file eyes_faces.h
 * @brief Main-task integration for automatic temporary facial expressions.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"
#include "../sound/sound.h"

/** @brief Reset automatic moods after a manual selection. @param base Selected mood. */
void eyes_faces_base(Expression base);
/** @brief Map an expression to the chirp family matching its mood. @param expr Expression. @return Tone family. */
FaceTone eyes_face_tone(Expression expr);
/**
 * @brief React to a short tap without modifying the base/preferences.
 * @param now Timestamp.
 * @param x Tap X in logical screen coordinates.
 * @param y Tap Y in logical screen coordinates.
 */
void eyes_faces_tap(uint32_t now, int16_t x, int16_t y);
/**
 * @brief React to a double tap with a playful face without modifying the base/preferences.
 * @param now Timestamp.
 * @param x Tap X in logical screen coordinates.
 * @param y Tap Y in logical screen coordinates.
 */
void eyes_faces_double(uint32_t now, int16_t x, int16_t y);
/** @brief React to a petting stroke without modifying the base/preferences. @param now Timestamp. @param x Stroke X in logical screen coordinates. @param y Stroke Y in logical screen coordinates. */
void eyes_faces_pet(uint32_t now, int16_t x, int16_t y);
/** @brief Silent startup greeting face that preserves the selected mood. @param now Timestamp. */
void eyes_faces_greet(uint32_t now);
/** @brief Apply automatic mood unless dizzy has priority. @param now Timestamp. */
void eyes_faces_step(uint32_t now);
/** @brief Feed mount-relative IMU motion. @param x G. @param y G. @param z G. @param now Timestamp. */
void eyes_faces_motion(float x, float y, float z, uint32_t now);
