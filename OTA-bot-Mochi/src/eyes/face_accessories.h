/**
 * @file face_accessories.h
 * @brief Small external face accessories with blink-stable anchors and bounded dirty regions.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "face_support.h"

namespace face_accessories {
/** @brief Match the scenery's smooth entry, retaining the accessory's full final palette. */
inline uint16_t ink(const EyeConfig &cfg, const EyeState &st, uint16_t color) {
	float t = st.lastMs < 1200 ? st.lastMs / 1200.0f : 1.0f;
	return eye_mix(color, cfg.bgColor, uint8_t(100 * t * t * (3 - 2 * t)));
}
/** @brief Anchor above the unclosed eye, retaining live pair translation. */
inline int16_t top(const EyeConfig &cfg, const EyeBox &box) {
	EyeLayout resting = eye_layout(cfg, EyeState{});
	int16_t height = box.left ? resting.left.h : resting.right.h;
	return int16_t(box.y + box.h / 2 - height / 2);
}
/** @brief Include the full accessory envelope without clearing the entire panel. */
inline EyeLayout bounds(const EyeConfig &cfg, EyeLayout layout) {
	int16_t pad = int16_t(36 * eye_scale(cfg) / 100 + 4);
	for (EyeBox *box : {&layout.left, &layout.right}) {
		int16_t right = eye_limit(box->x + box->w + pad, 1, cfg.screenW);
		int16_t bottom = eye_limit(box->y + box->h + pad, 1, cfg.screenH);
		box->x = eye_limit(box->x - pad, 0, cfg.screenW - 1);
		box->y = eye_limit(top(cfg, *box) - pad, 0, cfg.screenH - 1);
		box->w = right - box->x; box->h = bottom - box->y;
	}
	return layout;
}
/** @brief Draw one open golden aureole above both eyes, not a pupil mark. */
inline void aureole(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeLayout &layout) {
	int16_t scale = eye_scale(cfg);
	int16_t rx = eye_limit(23 * scale / 100, 10, cfg.screenW / 5);
	int16_t ry = eye_limit(7 * scale / 100, 4, 16);
	int16_t cx = (layout.left.x + layout.left.w / 2 + layout.right.x + layout.right.w / 2) / 2;
	int16_t leftTop = top(cfg, layout.left), rightTop = top(cfg, layout.right);
	int16_t cy = eye_limit((leftTop < rightTop ? leftTop : rightTop) - ry - 7 * scale / 100,
		ry + 2, cfg.screenH - ry - 2);
	int16_t stroke = eye_limit(2 * scale / 100, 1, 4);
	gfx->fillEllipse(cx, cy, rx, ry, ink(cfg, st, eye_rgb(255, 210, 48)));
	gfx->fillEllipse(cx, cy, rx - stroke, ry - stroke, cfg.bgColor);
}
}
