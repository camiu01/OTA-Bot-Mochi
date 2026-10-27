/**
 * @file deadpan.h
 * @brief deadpan definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace deadpan_face {
using namespace face_primitives;

/** @brief Immutable deadpan face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "deadpan";
	p.left.height = 12;
	p.right.height = 12;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid deadpan face parameters");
} }
