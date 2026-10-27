/**
 * @file dizzy.h
 * @brief dizzy definition and expression-specific effects.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../face_support.h"

namespace face_defs { namespace dizzy_face {
using namespace face_primitives;

/** @brief Immutable dizzy face; C++17 named-field initialization. */
inline constexpr FacePreset preset = [] {
	FacePreset p;
	p.name = "dizzy";
	p.replacesSolid = true;
	p.cross = true;
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid dizzy face parameters");
} }
