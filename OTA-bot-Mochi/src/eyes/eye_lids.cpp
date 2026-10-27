/** @brief Face-owned eyelid procedures followed by the configured manual lid. */
#include "face_support.h"

void eye_lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b) {
	const FacePreset *p = face_preset(cfg.expression);
	if (p && p->lids) { p->lids(gfx, cfg, b); }
	face_primitives::strip(gfx, b, cfg.lidTop * eye_scale(cfg) / 100, false, cfg.bgColor);
}
