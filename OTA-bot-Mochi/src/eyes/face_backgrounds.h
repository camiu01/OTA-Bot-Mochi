/**
 * @file face_backgrounds.h
 * @brief Procedural backdrops with a soft entry drawn behind the themed eye pair.
 *
 * @details A backdrop only paints the rows and columns the pair leaves free, so
 *          eye ink, blink masks and carved interiors stay untouched. Each
 *          painter fades in once over 1.2 seconds and derives its geometry from the
 *          resting eye layout of the panel, never from the animated layout, so
 *          a themed mood must return to the same frame after its performance,
 *          which the native cycle tests enforce. Colors are always blended from
 *          cfg.bgColor so the selected palette and the night theme keep their
 *          meaning. Painters reuse the legacy FacePreset::background hook, so
 *          they use the mood-relative elapsed time in the effect state.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "face_support.h"

namespace face_backgrounds {

/** @brief Fade backdrop ink in once, with a gentle start and stop and restrained contrast, never per cycle. */
inline uint16_t entry_mix(const EyeState &st, uint16_t ink, uint16_t bg, uint8_t amount) {
	float t = st.lastMs < 1200 ? st.lastMs / 1200.0f : 1.0f;
	t = t * t * (3 - 2 * t);
	return eye_mix(ink, bg, uint8_t(amount * 0.55f * t * st.backdropOpacity));
}

/** @brief Full-screen dirty bounds for a backdrop painting outside the pair. */
inline EyeLayout full_bounds(const EyeConfig &cfg, EyeLayout layout) {
	int16_t w = int16_t(cfg.screenW), h = int16_t(cfg.screenH);
	layout.left = {0, 0, w, h, 0, true};
	layout.right = {0, 0, w, h, 0, false};
	return layout;
}

/** @brief First row above the pair including a margin, clamped to the panel. */
inline int16_t pair_top(const EyeConfig &cfg, const EyeLayout &layout, int16_t pad) {
	int16_t first = layout.left.y < layout.right.y ? layout.left.y : layout.right.y;
	return eye_limit(int16_t(first - pad), 0, int16_t(cfg.screenH));
}

/** @brief First row below the pair including a margin, clamped to the panel. */
inline int16_t pair_bottom(const EyeConfig &cfg, const EyeLayout &layout, int16_t pad) {
	int16_t last = layout.left.y + layout.left.h > layout.right.y + layout.right.h ?
		layout.left.y + layout.left.h : layout.right.y + layout.right.h;
	return eye_limit(int16_t(last + pad), 0, int16_t(cfg.screenH));
}

/** @brief Columns the pair spans including a margin, clamped to the panel. */
inline void pair_columns(const EyeConfig &cfg, const EyeLayout &layout, int16_t pad, int16_t &left, int16_t &right) {
	int16_t first = layout.left.x < layout.right.x ? layout.left.x : layout.right.x;
	int16_t last = layout.left.x + layout.left.w > layout.right.x + layout.right.w ?
		layout.left.x + layout.left.w : layout.right.x + layout.right.w;
	left = eye_limit(int16_t(first - pad), 0, int16_t(cfg.screenW));
	right = eye_limit(int16_t(last + pad), left, int16_t(cfg.screenW));
}

/** @brief Gundam-style armour: pale cheek plates, red trim, dark crown and vent slats. */
inline void armor(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &) {
	const EyeLayout layout = eye_layout(cfg, EyeState{});
	int16_t top = pair_top(cfg, layout, cfg.screenH / 40);
	int16_t bottom = pair_bottom(cfg, layout, cfg.screenH / 40);
	int16_t left = 0, right = 0;
	pair_columns(cfg, layout, cfg.screenW / 30, left, right);
	const uint16_t plate = entry_mix(st, eye_rgb(224, 228, 235), cfg.bgColor, 74);
	const uint16_t shade = entry_mix(st, eye_rgb(148, 156, 168), cfg.bgColor, 74);
	const uint16_t trim = entry_mix(st, eye_rgb(214, 48, 54), cfg.bgColor, 88);
	const uint16_t crown = entry_mix(st, eye_rgb(40, 44, 52), cfg.bgColor, 82);
	// Cheek plates flank the pair, so the eye ink keeps the centre panel.
	for (int16_t side : {0, 1}) {
		int16_t x = side ? right : 0;
		int16_t w = side ? int16_t(cfg.screenW - right) : left;
		if (w <= 1) { continue; }
		gfx->fillRect(x, 0, w, int16_t(cfg.screenH), plate);
		int16_t stripe = eye_limit(int16_t(w / 5), 2, w);
		gfx->fillRect(side ? int16_t(x + w - stripe) : x, top, stripe, int16_t(bottom - top), trim);
		gfx->fillRect(x, int16_t(bottom - 1), w, 1, shade);
	}
	// The dark crown carries a white V-fin and red brow marks above the pair.
	if (top >= 10) {
		gfx->fillRect(0, 0, int16_t(cfg.screenW), top, crown);
		int16_t s = eye_limit(int16_t(top / 2), 3, int16_t(cfg.screenW / 6));
		int16_t cx = int16_t(cfg.screenW / 2), cy = int16_t(top / 2);
		int16_t brow = eye_limit(int16_t(top / 3), 3, top);
		int16_t bw = eye_limit(int16_t(s * 2), 6, int16_t(cfg.screenW / 5));
		for (int16_t side : {0, 1}) {
			int16_t dx = side ? s : int16_t(-s);
			gfx->drawLine(cx, cy, int16_t(cx + dx), int16_t(cy - s), plate);
			gfx->drawLine(cx, int16_t(cy + 1), int16_t(cx + dx), int16_t(cy - s + 1), plate);
			int16_t bx = side ? int16_t(cfg.screenW - bw) : 0;
			gfx->fillRect(bx, int16_t(top - brow), bw, brow, trim);
		}
	}
	// Three dark vent slats close the white mouth plate below the pair.
	int16_t vents = int16_t(cfg.screenH - bottom);
	if (vents >= 10) {
		gfx->fillRect(0, bottom, int16_t(cfg.screenW), vents, plate);
		int16_t slat = eye_limit(int16_t(vents / 8), 1, int16_t(vents / 5));
		int16_t gap = eye_limit(int16_t(vents / 10), 1, int16_t(vents / 6));
		int16_t y = int16_t(bottom + (vents - (slat * 3 + gap * 2)) / 2);
		int16_t bw = eye_limit(int16_t(cfg.screenW / 4), 8, int16_t(cfg.screenW / 2));
		for (uint8_t i = 0; i < 3; ++i) {
			gfx->fillRect(int16_t(cfg.screenW / 2 - bw / 2), int16_t(y + i * (slat + gap)), bw, slat, crown);
		}
	}
}

/** @brief Hinomaru disc behind the pair with a dark rim and a low horizon band. */
inline void sun(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &) {
	const EyeLayout layout = eye_layout(cfg, EyeState{});
	int16_t bottom = pair_bottom(cfg, layout, cfg.screenH / 30);
	int16_t span = cfg.screenW < cfg.screenH ? cfg.screenW : cfg.screenH;
	int16_t r = eye_limit(int16_t(span * 44 / 100), 8, int16_t(cfg.screenW));
	int16_t cx = int16_t(cfg.screenW / 2), cy = int16_t(cfg.screenH / 2);
	gfx->fillCircle(cx, cy, r, entry_mix(st, eye_rgb(212, 64, 62), cfg.bgColor, 44));
	gfx->drawCircle(cx, cy, r, entry_mix(st, eye_rgb(126, 38, 42), cfg.bgColor, 62));
	int16_t h = int16_t(cfg.screenH - bottom);
	if (h >= 6) {
		gfx->fillRect(0, bottom, int16_t(cfg.screenW), h, entry_mix(st, eye_rgb(58, 60, 72), cfg.bgColor, 70));
		gfx->fillRect(0, bottom, int16_t(cfg.screenW), 1, entry_mix(st, eye_rgb(196, 70, 66), cfg.bgColor, 70));
	}
}

/** @brief Cracked rock strata below the pair with drifting steam above it. */
inline void wall(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &) {
	const EyeLayout layout = eye_layout(cfg, EyeState{});
	int16_t top = pair_top(cfg, layout, cfg.screenH / 30);
	int16_t bottom = pair_bottom(cfg, layout, cfg.screenH / 30);
	int16_t h = int16_t(cfg.screenH - bottom);
	if (h >= 8) {
		for (uint8_t i = 0; i < 4; ++i) {
			int16_t y = int16_t(bottom + int32_t(h) * i / 4);
			uint16_t rock = entry_mix(st, eye_rgb(uint8_t(94 - i * 16), uint8_t(66 - i * 12),
				uint8_t(60 - i * 10)), cfg.bgColor, 66);
			gfx->fillRect(0, y, int16_t(cfg.screenW), int16_t(h / 4 + 1), rock);
		}
		for (uint8_t i = 0; i < 3; ++i) {
			int16_t x = int16_t(cfg.screenW * (i * 2 + 1) / 6);
			gfx->drawLine(x, bottom, x, int16_t(cfg.screenH - 1), entry_mix(st, eye_rgb(30, 24, 26), cfg.bgColor, 72));
		}
	}
	if (top >= 8) {
		uint16_t steam = entry_mix(st, eye_rgb(168, 162, 156), cfg.bgColor, 38);
		for (uint8_t i = 0; i < 3; ++i) {
			int16_t w = eye_limit(int16_t(cfg.screenW / (5 + i)), 6, int16_t(cfg.screenW - 2));
			int16_t x = eye_limit(int16_t(cfg.screenW - w - i * cfg.screenW / 10), 0, int16_t(cfg.screenW - w));
			gfx->fillRect(x, int16_t(top * (i + 1) / 4), w, 1, steam);
		}
	}
}

/** @brief Pale moon above the pair with static embers resting in the free columns. */
inline void night(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &) {
	const EyeLayout layout = eye_layout(cfg, EyeState{});
	int16_t top = pair_top(cfg, layout, cfg.screenH / 30);
	int16_t left = 0, right = 0;
	pair_columns(cfg, layout, cfg.screenW / 30, left, right);
	if (top >= 14) {
		int16_t r = eye_limit(int16_t(top / 3), 4, int16_t(top / 2));
		gfx->fillCircle(int16_t(cfg.screenW - r - cfg.screenW / 16), int16_t(top / 2), r,
			entry_mix(st, eye_rgb(236, 232, 214), cfg.bgColor, 58));
		uint16_t star = entry_mix(st, eye_rgb(214, 226, 250), cfg.bgColor, 52);
		for (uint8_t i = 0; i < 3; ++i) {
			gfx->fillRect(int16_t(cfg.screenW / 14 + i * cfg.screenW / 9), int16_t(top / 5 + i * 2), 1, 1, star);
		}
	}
	if (left < 5 && cfg.screenW - right < 5) { return; }
	uint16_t ember = entry_mix(st, eye_rgb(242, 150, 78), cfg.bgColor, 74);
	for (uint8_t i = 0; i < 8; ++i) {
		int16_t y = int16_t(cfg.screenH * (i * 4 + 3) / 32);
		int16_t step = int16_t(1 + i % 3);
		if (i & 1) {
			if (cfg.screenW - right >= 5) {
				gfx->fillRect(int16_t(right + (cfg.screenW - right) * step / 4), y, 2, 2, ember);
			}
		} else if (left >= 5) {
			gfx->fillRect(int16_t(left * step / 4), y, 2, 2, ember);
		}
	}
}

/** @brief Dim neon tubes along the panel edges the pair leaves free. */
inline void tubes(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &) {
	const EyeLayout layout = eye_layout(cfg, EyeState{});
	int16_t top = pair_top(cfg, layout, cfg.screenW / 30);
	int16_t bottom = pair_bottom(cfg, layout, cfg.screenW / 30);
	int16_t left = 0, right = 0;
	pair_columns(cfg, layout, cfg.screenW / 30, left, right);
	const uint16_t tube = entry_mix(st, eye_rgb(88, 206, 252), cfg.bgColor, 52);
	const uint16_t core = entry_mix(st, eye_rgb(222, 246, 255), cfg.bgColor, 74);
	for (int16_t side : {0, 1}) {
		int16_t w = side ? int16_t(cfg.screenW - right) : left;
		if (w < 6) { continue; }
		int16_t x = side ? right : 0;
		int16_t thick = eye_limit(int16_t(w / 4), 2, int16_t(w - 2));
		int16_t tube_x = int16_t(x + (w - thick) / 2);
		gfx->fillRect(tube_x, 0, thick, int16_t(cfg.screenH), tube);
		gfx->fillRect(int16_t(tube_x + thick / 4), 0, eye_limit(int16_t(thick / 2), 1, thick),
			int16_t(cfg.screenH), core);
	}
	if (top >= 6) { gfx->fillRect(0, int16_t(top - 2), int16_t(cfg.screenW), 2, tube); }
	if (cfg.screenH - bottom >= 6) { gfx->fillRect(0, bottom, int16_t(cfg.screenW), 2, tube); }
}

/** @brief Static asphalt below the pair with lane dashes and short speed streaks. */
inline void road(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &) {
	const EyeLayout layout = eye_layout(cfg, EyeState{});
	int16_t top = pair_top(cfg, layout, cfg.screenH / 30);
	int16_t bottom = pair_bottom(cfg, layout, cfg.screenH / 30);
	int16_t h = int16_t(cfg.screenH - bottom);
	if (h >= 6) {
		gfx->fillRect(0, bottom, int16_t(cfg.screenW), h, entry_mix(st, eye_rgb(54, 57, 64), cfg.bgColor, 72));
		gfx->fillRect(0, bottom, int16_t(cfg.screenW), 1, entry_mix(st, eye_rgb(146, 154, 166), cfg.bgColor, 66));
		int16_t dash = eye_limit(int16_t(cfg.screenW / 10), 4, int16_t(cfg.screenW / 3));
		int16_t mark = eye_limit(int16_t(h / 10), 1, 3);
		int16_t y = eye_limit(int16_t(bottom + h * 2 / 3), bottom, int16_t(cfg.screenH - mark));
		uint16_t paint = entry_mix(st, eye_rgb(238, 234, 222), cfg.bgColor, 80);
		for (int16_t x = int16_t(cfg.screenW / 20); x < cfg.screenW; x += dash * 2) {
			gfx->fillRect(x, y, dash, mark, paint);
		}
	}
	if (top >= 6) {
		gfx->fillRect(0, int16_t(top - 2), int16_t(cfg.screenW), 1, entry_mix(st, eye_rgb(126, 140, 158), cfg.bgColor, 58));
		uint16_t streak = entry_mix(st, eye_rgb(150, 168, 190), cfg.bgColor, 46);
		for (uint8_t i = 0; i < 3; ++i) {
			int16_t w = eye_limit(int16_t(cfg.screenW / (6 + i)), 4, int16_t(cfg.screenW / 2));
			int16_t x = eye_limit(int16_t(cfg.screenW / 12 + i * cfg.screenW / 4), 0, int16_t(cfg.screenW - w));
			gfx->fillRect(x, int16_t(top * (i + 1) / 4), w, 1, streak);
		}
	}
}

/** @brief Layered pale halo behind each cup, the soft bloom of the reference look. */
inline void soft(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &) {
	const EyeLayout layout = eye_layout(cfg, EyeState{});
	for (const EyeBox &b : {layout.left, layout.right}) {
		for (int16_t layer = 3; layer >= 1; --layer) {
			int16_t pad = int16_t(layer * cfg.screenW / 40);
			if (pad < 2) { continue; }
			EyeConfig glow = cfg;
			glow.fgColor = entry_mix(st, eye_rgb(146, 190, 246), cfg.bgColor, uint8_t(16 * (4 - layer)));
			eye_body(gfx, glow, eye_halo_box(cfg, b, pad));
		}
	}
}

}  // namespace face_backgrounds
