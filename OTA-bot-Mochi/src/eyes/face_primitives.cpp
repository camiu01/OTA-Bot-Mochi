#include "face_support.h"

/** @brief Shared eye_mix animation primitive. */
uint16_t eye_mix(uint16_t ink, uint16_t bg, uint8_t amount) {
	uint16_t r = (((ink >> 11) * amount + (bg >> 11) * (100 - amount)) / 100);
	uint16_t g = ((((ink >> 5) & 63) * amount + ((bg >> 5) & 63) * (100 - amount)) / 100);
	uint16_t b = (((ink & 31) * amount + (bg & 31) * (100 - amount)) / 100);
	return uint16_t((r << 11) | (g << 5) | b);
}

/** @brief Shared eye_halo_box animation primitive. */
EyeBox eye_halo_box(const EyeConfig &cfg, const EyeBox &b, int16_t pad) {
	int16_t x = eye_limit(b.x - pad, 0, cfg.screenW - 1);
	int16_t y = eye_limit(b.y - pad, 0, cfg.screenH - 1);
	return {x, y, int16_t(eye_limit(b.x + b.w + pad, x + 1, cfg.screenW) - x),
		int16_t(eye_limit(b.y + b.h + pad, y + 1, cfg.screenH) - y), int16_t(b.radius + pad), b.left};
}

/** @brief Shared eye_reaction_amount animation primitive. */
float eye_reaction_amount(uint32_t elapsed) {
	uint32_t p = elapsed % 3000;
	float t = p < 300 ? 0 : (p < 800 ? (p - 300) / 500.0f :
		(p < 2200 ? 1 : (p < 2700 ? (2700 - p) / 500.0f : 0)));
	return t * t * (3 - 2 * t);
}

/** @brief Shared eye_reaction_chevron animation primitive. */
void eye_reaction_chevron(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	int16_t edge = b.left ? b.x + 2 : b.x + b.w - 3;
	int16_t tip = b.left ? b.x + b.w - 3 : b.x + 2;
	int16_t thick = eye_limit(b.w / 7, 2, b.h / 5);
	for (int16_t i = 0; i < thick; ++i) {
		gfx->drawLine(edge, b.y + b.h / 5 + i, tip, b.y + b.h / 2 + i, cfg.fgColor);
		gfx->drawLine(tip, b.y + b.h / 2 + i, edge, b.y + b.h * 4 / 5 + i, cfg.fgColor);
	}
}

/** @brief Shared eye_effect_elapsed animation primitive. */
uint32_t eye_effect_elapsed(EyeState &st, Expression expression) {
	if (!st.effectClockInit || st.effectExpression != expression) {
		st.effectExpression = expression;
		st.effectStartMs = st.lastMs;
		st.effectClockInit = true;
	}
	return st.lastMs - st.effectStartMs;
}

/** @brief Shared eye_scene_amount animation primitive. */
float eye_scene_amount(uint32_t elapsed) {
	uint32_t phase = elapsed % 12000;
	float t = phase < 800 ? 0 : (phase < 2000 ? (phase - 800) / 1200.0f :
		(phase < 10000 ? 1 : (phase < 11200 ? (11200 - phase) / 1200.0f : 0)));
	return t * t * (3 - 2 * t);
}

/** @brief Shared eye_scene_box animation primitive. */
EyeBox eye_scene_box(const EyeBox &a, const EyeBox &b, float t) {
	return {int16_t(a.x + (b.x - a.x) * t), int16_t(a.y + (b.y - a.y) * t),
		int16_t(a.w + (b.w - a.w) * t), int16_t(a.h + (b.h - a.h) * t),
		int16_t(a.radius + (b.radius - a.radius) * t), a.left};
}

/** @brief Shared eye_scene_body animation primitive. */
void eye_scene_body(Arduino_GFX *gfx, EyeConfig cfg, const EyeBox &box, float t) {
	cfg.slant = int8_t(cfg.slant * (1 - t));
	if (cfg.style == TRAPEZ && t < 0.5f) { eye_body(gfx, cfg, box); }
	else { gfx->fillRoundRect(box.x, box.y, box.w, box.h, box.radius, cfg.fgColor); }
	if (t < 0.3f) {
		cfg.pupilColor = eye_mix(cfg.fgColor, cfg.pupilColor, uint8_t(t * 333));
		cfg.highlightColor = eye_mix(cfg.fgColor, cfg.highlightColor, uint8_t(t * 333));
		eye_pupils(gfx, cfg, EyeState{}, box);
	}
}

/** @brief Shared eye_performance_amount animation primitive. */
float eye_performance_amount(uint32_t elapsed) {
	uint32_t p = elapsed % 6000;
	float t = p < 600 ? 0 : (p < 1500 ? (p - 600) / 900.0f :
		(p < 4500 ? 1 : (p < 5400 ? (5400 - p) / 900.0f : 0)));
	return t * t * (3 - 2 * t);
}

/** @brief Shared eye_extra_text animation primitive. */
void eye_extra_text(Arduino_GFX *gfx, const EyeConfig &cfg, const char *text,
		int16_t x, int16_t y, uint16_t color) {
	uint8_t length = uint8_t(strlen(text));
	int16_t size = cfg.screenW / (6 * length + 6);
	if (size < 1) { size = 1; }
	int16_t left = eye_limit(int16_t(x - length * 3 * size), 0, cfg.screenW - length * 6 * size);
	gfx->setFont(nullptr);
	gfx->setTextWrap(false);
	gfx->setTextSize(uint8_t(size));
	gfx->setTextColor(color);
	gfx->setCursor(left, eye_limit(y, 0, cfg.screenH - 8 * size));
	gfx->print(String(text));
}
