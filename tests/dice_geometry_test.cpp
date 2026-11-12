/**
 * @file dice_geometry_test.cpp
 * @brief Independent solid invariants: face counts/types, planarity, convexity and closed Euler topology.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/play/dice_geometry.h"
#include <assert.h>
#include <cmath>
#include <iostream>

/** @brief Test physical polygon faces, including the ten kite faces of a D10. */
static void solid_checks(uint8_t sides, uint8_t vertices, uint8_t edges, uint8_t face_size) {
	auto mesh = dice_mesh(sides);
	assert(mesh.vertex_count == vertices && mesh.face_count == sides);
	uint8_t incidence[20][20] = {};
	for (uint8_t i = 0; i < mesh.face_count; ++i) {
		const auto &face = mesh.faces[i];
		assert(face.count == face_size);
		auto a = mesh.vertices[face.vertex[0]], b = mesh.vertices[face.vertex[1]], c = mesh.vertices[face.vertex[2]];
		DiceVertex u{b.x-a.x,b.y-a.y,b.z-a.z}, v{c.x-a.x,c.y-a.y,c.z-a.z};
		DiceVertex n{u.y*v.z-u.z*v.y,u.z*v.x-u.x*v.z,u.x*v.y-u.y*v.x};
		assert(n.x*n.x+n.y*n.y+n.z*n.z > .01f);
		assert(n.x*a.x+n.y*a.y+n.z*a.z > 0); // Outward winding.
		for (uint8_t j = 0; j < mesh.vertex_count; ++j) {
			auto p = mesh.vertices[j];
			float distance = n.x*(p.x-a.x)+n.y*(p.y-a.y)+n.z*(p.z-a.z);
			assert(distance < .0001f); // Every vertex inside every outward face plane.
		}
		for (uint8_t j = 0; j < face.count; ++j) {
			uint8_t from = face.vertex[j], to = face.vertex[(j+1)%face.count];
			assert(from < vertices && to < vertices && from != to);
			auto p = mesh.vertices[from];
			assert(std::abs(n.x*(p.x-a.x)+n.y*(p.y-a.y)+n.z*(p.z-a.z)) < .0001f);
			++incidence[from][to];
		}
	}
	uint8_t actual_edges = 0;
	for (uint8_t a = 0; a < vertices; ++a) {
		for (uint8_t b = a+1; b < vertices; ++b) {
			if (!incidence[a][b] && !incidence[b][a]) { continue; }
			assert(incidence[a][b] == 1 && incidence[b][a] == 1); ++actual_edges;
		}
	}
	assert(actual_edges == edges && vertices - edges + sides == 2);
}
int main() {
	solid_checks(4,4,6,3); solid_checks(6,8,12,4); solid_checks(8,6,12,3);
	solid_checks(10,12,20,4); solid_checks(12,20,30,5); solid_checks(20,12,30,3);
	assert(dice_mesh(100).vertices == dice_mesh(10).vertices);
	assert(!dice_mesh(0).vertex_count && !dice_mesh(255).face_count);
	std::cout << "Dice solids: faces, edges, planarity, convexity and closed topology passed\n";
}
