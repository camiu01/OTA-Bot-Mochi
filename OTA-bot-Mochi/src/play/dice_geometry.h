/**
 * @file dice_geometry.h
 * @brief Closed convex dice meshes; polygon boundaries are physical edges, not triangulation diagonals.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>

struct DiceVertex { float x, y, z; };
struct DiceFace { uint8_t count, vertex[5]; };
struct DiceMesh {
	const DiceVertex *vertices;
	const DiceFace *faces;
	uint8_t vertex_count, face_count;
};

/** @brief Return the actual solid for a die; percentile dice use the D10 mesh twice. */
inline DiceMesh dice_mesh(uint8_t sides) {
	static constexpr DiceVertex tetra[] = {{1,1,1}, {1,-1,-1}, {-1,1,-1}, {-1,-1,1}};
	static constexpr DiceFace tetra_faces[] = {
		{3,{2,0,1}}, {3,{1,0,3}}, {3,{3,0,2}}, {3,{2,1,3}}
	};
	static constexpr DiceVertex cube[] = {
		{-1,-1,-1}, {-1,-1,1}, {-1,1,-1}, {-1,1,1},
		{1,-1,-1}, {1,-1,1}, {1,1,-1}, {1,1,1}
	};
	static constexpr DiceFace cube_faces[] = {
		{4,{2,0,1,3}}, {4,{1,0,4,5}}, {4,{4,0,2,6}},
		{4,{3,1,5,7}}, {4,{6,2,3,7}}, {4,{5,4,6,7}}
	};
	static constexpr DiceVertex octa[] = {{-1,0,0}, {1,0,0}, {0,-1,0}, {0,1,0}, {0,0,-1}, {0,0,1}};
	static constexpr DiceFace octa_faces[] = {
		{3,{2,0,4}}, {3,{5,0,2}}, {3,{4,0,3}}, {3,{3,0,5}},
		{3,{4,1,2}}, {3,{2,1,5}}, {3,{3,1,4}}, {3,{5,1,3}}
	};
	static constexpr DiceVertex deca[] = {
		{0,0,1}, {0,0,-1}, {1,0,.105573f}, {.809017f,.587785f,-.105573f},
		{.309017f,.951057f,.105573f}, {-.309017f,.951057f,-.105573f},
		{-.809017f,.587785f,.105573f}, {-1,0,-.105573f},
		{-.809017f,-.587785f,.105573f}, {-.309017f,-.951057f,-.105573f},
		{.309017f,-.951057f,.105573f}, {.809017f,-.587785f,-.105573f}
	};
	static constexpr DiceFace deca_faces[] = {
		{4,{3,4,0,2}}, {4,{2,0,10,11}}, {4,{5,6,0,4}}, {4,{7,8,0,6}}, {4,{10,0,8,9}},
		{4,{11,1,3,2}}, {4,{3,1,5,4}}, {4,{6,5,1,7}}, {4,{8,7,1,9}}, {4,{9,1,11,10}}
	};
	static constexpr DiceVertex dodeca[] = {
		{-1,-1,-1}, {-1,-1,1}, {-1,1,-1}, {-1,1,1},
		{1,-1,-1}, {1,-1,1}, {1,1,-1}, {1,1,1},
		{0,-.618034f,-1.618034f}, {0,-.618034f,1.618034f},
		{0,.618034f,-1.618034f}, {0,.618034f,1.618034f},
		{-.618034f,-1.618034f,0}, {-.618034f,1.618034f,0},
		{.618034f,-1.618034f,0}, {.618034f,1.618034f,0},
		{-1.618034f,0,-.618034f}, {-1.618034f,0,.618034f},
		{1.618034f,0,-.618034f}, {1.618034f,0,.618034f}
	};
	static constexpr DiceFace dodeca_faces[] = {
		{5,{17,16,0,12,1}}, {5,{10,8,0,16,2}}, {5,{14,12,0,8,4}},
		{5,{3,17,1,9,11}}, {5,{5,9,1,12,14}}, {5,{3,13,2,16,17}},
		{5,{6,10,2,13,15}}, {5,{15,13,3,11,7}}, {5,{5,14,4,18,19}},
		{5,{6,18,4,8,10}}, {5,{11,9,5,19,7}}, {5,{19,18,6,15,7}}
	};
	static constexpr DiceVertex icosa[] = {
		{0,-1,-1.618034f}, {0,-1,1.618034f}, {0,1,-1.618034f}, {0,1,1.618034f},
		{-1,-1.618034f,0}, {-1,1.618034f,0}, {1,-1.618034f,0}, {1,1.618034f,0},
		{-1.618034f,0,-1}, {-1.618034f,0,1}, {1.618034f,0,-1}, {1.618034f,0,1}
	};
	static constexpr DiceFace icosa_faces[] = {
		{3,{2,0,8}}, {3,{10,0,2}}, {3,{4,0,6}}, {3,{8,0,4}}, {3,{6,0,10}},
		{3,{9,1,3}}, {3,{3,1,11}}, {3,{6,1,4}}, {3,{4,1,9}}, {3,{11,1,6}},
		{3,{7,2,5}}, {3,{5,2,8}}, {3,{10,2,7}}, {3,{5,3,7}}, {3,{9,3,5}},
		{3,{7,3,11}}, {3,{8,4,9}}, {3,{9,5,8}}, {3,{11,6,10}}, {3,{10,7,11}}
	};
	switch (sides) {
		case 4: return {tetra, tetra_faces, 4, 4};
		case 6: return {cube, cube_faces, 8, 6};
		case 8: return {octa, octa_faces, 6, 8};
		case 10: case 100: return {deca, deca_faces, 12, 10};
		case 12: return {dodeca, dodeca_faces, 20, 12};
		case 20: return {icosa, icosa_faces, 12, 20};
		default: return {nullptr, nullptr, 0, 0};
	}
}
