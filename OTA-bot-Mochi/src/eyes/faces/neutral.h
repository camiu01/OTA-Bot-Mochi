/**
 * @file neutral.h
 * @brief neutral definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace neutral_face {
using namespace face_primitives;

/** @brief Immutable neutral face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "neutral";
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid neutral face parameters");
} }
