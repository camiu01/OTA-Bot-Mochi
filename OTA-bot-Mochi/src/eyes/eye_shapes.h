/**
 * @file eye_shapes.h
 * @brief Style-specific bodies, pupil policy, and expression silhouettes.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eyes_api.h"
#include "eye_geometry.h"

/** @brief Draw a style body or solid expression symbol. @param gfx Canvas. @param cfg Config. @param box Eye. */
void eye_body(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &box);
/** @brief Draw pupils only for NORMAL/BLOUB. @param gfx Canvas. @param cfg Config. @param st State. @param box Eye. */
void eye_pupils(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &box);
/** @brief Shape eyelids inside one eye's bounds. @param gfx Canvas. @param cfg Config. @param box Eye. */
void eye_lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &box);
