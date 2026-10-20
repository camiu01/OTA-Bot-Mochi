/**
 * @file eye_reference.h
 * @brief Eye-only video-inspired silhouettes; no mouths, cheeks, or external decorations.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "eye_shapes.h"
#include "../play/mini_widgets.h"

/** @brief Draw closed chevrons or animated clock eyes. @param gfx Canvas. @param cfg Config. @param st State. @param b Bounds. @return True when this eye is complete. */
inline bool eye_reference_body(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b) {
	if (cfg.expression == CLOCK_EYES) {
		eye_body(gfx, cfg, b);
		int16_t r = (b.w < b.h ? b.w : b.h) / 2 - 3;
		mini_clock_draw(gfx, b.x + b.w / 2, b.y + b.h / 2, r,
			st.lastMs / 1000, cfg.bgColor, cfg.bgColor);
		return true;
	}
	if (cfg.expression != SQUEEZED) { return false; }
	int16_t edge = b.left ? b.x + 3 : b.x + b.w - 4;
	int16_t tip = b.left ? b.x + b.w - 4 : b.x + 3;
	int16_t thickness = b.h / 14 + 1;
	for (int16_t i = 0; i < thickness; ++i) {
		gfx->drawLine(edge, b.y + b.h / 5 + i, tip, b.y + b.h / 2 + i, cfg.fgColor);
		gfx->drawLine(tip, b.y + b.h / 2 + i, edge, b.y + b.h * 4 / 5 + i, cfg.fgColor);
	}
	return true;
}

/** @brief Clip existing style bodies into curved or tilted lids. @param gfx Canvas. @param cfg Config. @param b Bounds. */
inline void eye_reference_lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	if (cfg.expression != ARC_HAPPY && cfg.expression != ARC_SAD && cfg.expression != TILTED) { return; }
	for (int16_t i = 0; i < b.w; ++i) {
		int16_t top = 0, bottom = 0;
		if (cfg.expression == TILTED) {
			int16_t along = b.left ? i : b.w - 1 - i;
			top = along * b.h / (b.w * 3);
			bottom = (b.w - 1 - along) * b.h / (b.w * 3);
		} else {
			int32_t t = 2 * i - b.w + 1;
			int16_t curve = int16_t(t * t * b.h / (int32_t(b.w) * b.w * 3));
			top = cfg.expression == ARC_HAPPY ? b.h / 5 + curve : b.h * 3 / 5 - curve;
			bottom = b.h - top - (b.h / 9 + 2);
		}
		if (top > 0) { gfx->fillRect(b.x + i, b.y, 1, top, cfg.bgColor); }
		if (bottom > 0) { gfx->fillRect(b.x + i, b.y + b.h - bottom, 1, bottom, cfg.bgColor); }
	}
}
