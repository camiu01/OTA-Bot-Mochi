/**
 * @file dice_shape.h
 * @brief Orthographic solid dice with visible polygon edges and a result on the largest front face.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "../play/dice_geometry.h"
#include "ui_theme.h"
#include <math.h>

/** @brief Fixed three-quarter pose, shared by projection and face visibility. */
inline DiceVertex dice_rotate(DiceVertex v, uint8_t sides) {
	if (sides == 10) { v = {v.x, v.z, -v.y}; } // Keep the D10's two poles upright.
	if (sides == 4) {
		float x = .980067f * v.x + .198669f * v.z;
		float z = -.198669f * v.x + .980067f * v.z;
		return {x, .980067f * v.y - .198669f * z, .198669f * v.y + .980067f * z};
	}
	float x = .852525f * v.x + .522687f * v.z;
	float z = -.522687f * v.x + .852525f * v.z;
	return {x, .921061f * v.y - .389418f * z, .389418f * v.y + .921061f * z};
}

/** @brief Draw actual front-facing polygon boundaries; radius and center are design coordinates. */
inline void dice_shape_draw(Arduino_GFX *gfx, uint8_t sides, int16_t cx, int16_t cy,
	int16_t radius, const String &result) {
	const auto mesh = dice_mesh(sides);
	if (!mesh.vertex_count || radius <= 0) { return; }
	DiceVertex vertices[20];
	float extent = 0;
	for (uint8_t i = 0; i < mesh.vertex_count; ++i) {
		vertices[i] = dice_rotate(mesh.vertices[i], sides);
		float length = sqrtf(vertices[i].x * vertices[i].x + vertices[i].y * vertices[i].y);
		if (length > extent) { extent = length; }
	}
	int16_t x[20], y[20];
	for (uint8_t i = 0; i < mesh.vertex_count; ++i) {
		x[i] = ui_s(cx) + int16_t(vertices[i].x * ui_s(radius) / extent);
		y[i] = ui_s(cy) - int16_t(vertices[i].y * ui_s(radius) / extent);
	}
	int32_t best_area = 0; uint8_t front = 0;
	for (uint8_t i = 0; i < mesh.face_count; ++i) {
		const auto &f = mesh.faces[i]; int32_t area = 0;
		for (uint8_t j = 0; j < f.count; ++j) {
			uint8_t a = f.vertex[j], b = f.vertex[(j + 1) % f.count];
			area += int32_t(x[a]) * y[b] - int32_t(x[b]) * y[a];
		}
		if (area >= 0) { continue; }
		if (-area > best_area) { best_area = -area; front = i; }
		for (uint8_t j = 1; j + 1 < f.count; ++j) {
			gfx->fillTriangle(x[f.vertex[0]], y[f.vertex[0]], x[f.vertex[j]], y[f.vertex[j]],
				x[f.vertex[j + 1]], y[f.vertex[j + 1]], UI_CARD);
		}
		for (uint8_t j = 0; j < f.count; ++j) {
			uint8_t a = f.vertex[j], b = f.vertex[(j + 1) % f.count];
			gfx->drawLine(x[a], y[a], x[b], y[b], UI_ACCENT);
		}
	}
	const auto &face = mesh.faces[front]; int32_t tx = 0, ty = 0;
	for (uint8_t j = 0; j < face.count; ++j) { tx += x[face.vertex[j]]; ty += y[face.vertex[j]]; }
	constexpr uint8_t size = 2;
	int16_t width = int16_t(result.length()) * 6 * ui_text_size(size);
	ui_label(gfx, result, int16_t(tx / face.count - width / 2),
		int16_t(ty / face.count - 4 * ui_text_size(size)), width + 4, size, UI_TEXT);
}
