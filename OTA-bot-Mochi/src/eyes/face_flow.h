/**
 * @file face_flow.h
 * @brief Stable automatic-expression playback modes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

enum FaceFlow : uint8_t {
	FACE_RETURN = 0, FACE_GROUP5, FACE_SEQUENCE, FACE_CONTINUOUS, FACE_FLOW_COUNT
};
