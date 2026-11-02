/**
 * @file mecha_break.h
 * @brief Mecha armor assembles, then breaks away to reveal the configured eyes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "mecha.h"
namespace face_defs { namespace mecha_break_face {
/** @brief Share the helmet artwork while reversing the release order and panel trajectories. */
inline constexpr FacePreset preset = [] {
	FacePreset p = mecha_face::preset;
	p.name = "mecha_break";
	return p;
}();
static_assert(face_preset_valid(preset), "Invalid breakaway mecha face");
} }
