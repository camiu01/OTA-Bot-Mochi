/**
 * @file eyes_faces.h
 * @brief Main-task integration for automatic temporary facial expressions.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_types.h"

/** @brief Reset automatic moods after a manual selection. @param base Selected mood. */
void eyes_faces_base(Expression base);
/** @brief React to a short tap without modifying the base/preferences. @param now Timestamp. */
void eyes_faces_tap(uint32_t now);
/** @brief Apply automatic mood unless dizzy has priority. @param now Timestamp. */
void eyes_faces_step(uint32_t now);
/** @brief Feed mount-relative IMU motion. @param x G. @param y G. @param z G. @param now Timestamp. */
void eyes_faces_motion(float x, float y, float z, uint32_t now);
