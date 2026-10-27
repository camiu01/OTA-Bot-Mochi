/**
 * @file cat.h
 * @brief cat definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace cat_face {
using namespace face_primitives;

/** @brief Immutable cat face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "cat";
	p.left.widthDelta = -4;
	p.right.widthDelta = -4;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid cat face parameters");
} }
