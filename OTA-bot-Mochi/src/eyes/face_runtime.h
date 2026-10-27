#pragma once
#include "eye_shapes.h"
#include "eye_palette.h"
/** @brief Existing eye_mix rendering entry point. */
uint16_t eye_mix(uint16_t ink, uint16_t bg, uint8_t amount);
/** @brief Existing eye_glow_color rendering entry point. */
uint16_t eye_glow_color(uint32_t now, uint16_t base);
/** @brief Existing eye_halo_box rendering entry point. */
EyeBox eye_halo_box(const EyeConfig &cfg, const EyeBox &b, int16_t pad);
/** @brief Existing eye_gauge_box rendering entry point. */
EyeBox eye_gauge_box(const EyeConfig &cfg);
/** @brief Existing eye_effect_bounds rendering entry point. */
EyeLayout eye_effect_bounds(const EyeConfig &cfg, EyeLayout layout);
/** @brief Existing eye_headlight rendering entry point. */
void eye_headlight(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t now);
/** @brief Existing eye_effect_background rendering entry point. */
void eye_effect_background(Arduino_GFX *gfx, EyeConfig &cfg, const EyeState &st, const EyeLayout &layout);
/** @brief Existing eye_gauge_sweep rendering entry point. */
float eye_gauge_sweep(uint32_t elapsed);
/** @brief Existing eye_gauge_draw rendering entry point. */
void eye_gauge_draw(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t now, uint8_t opacity = 100);
/** @brief Existing eye_reaction_amount rendering entry point. */
float eye_reaction_amount(uint32_t elapsed);
/** @brief Existing eye_reaction_ink rendering entry point. */
uint16_t eye_reaction_ink(const EyeConfig &cfg, uint32_t elapsed);
/** @brief Existing eye_reaction_motion rendering entry point. */
void eye_reaction_motion(Expression expr, bool left, uint32_t phase, float a,
		float &sx, float &sy, float &dx, float &dy);
/** @brief Existing eye_reaction_layout rendering entry point. */
void eye_reaction_layout(const EyeConfig &cfg, EyeLayout &layout, uint32_t elapsed);
/** @brief Existing eye_reaction_chevron rendering entry point. */
void eye_reaction_chevron(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b);
/** @brief Existing eye_reaction_rest rendering entry point. */
void eye_reaction_rest(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b);
/** @brief Existing eye_reaction_body rendering entry point. */
bool eye_reaction_body(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral);
/** @brief Existing eye_reference_body rendering entry point. */
bool eye_reference_body(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b);
/** @brief Existing eye_reference_lids rendering entry point. */
void eye_reference_lids(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b);
/** @brief Existing eye_effect_elapsed rendering entry point. */
uint32_t eye_effect_elapsed(EyeState &st, Expression expression);
/** @brief Existing eye_scene_amount rendering entry point. */
float eye_scene_amount(uint32_t elapsed);
/** @brief Existing eye_scene_box rendering entry point. */
EyeBox eye_scene_box(const EyeBox &a, const EyeBox &b, float t);
/** @brief Existing eye_scene_body rendering entry point. */
void eye_scene_body(Arduino_GFX *gfx, EyeConfig cfg, const EyeBox &box, float t);
/** @brief Existing eye_scene_gauge rendering entry point. */
void eye_scene_gauge(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float t);
/** @brief Existing eye_scene_pong rendering entry point. */
void eye_scene_pong(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float t);
/** @brief Existing eye_performance rendering entry point. */
bool eye_performance(Expression expr);
/** @brief Existing eye_performance_amount rendering entry point. */
float eye_performance_amount(uint32_t elapsed);
/** @brief Existing eye_stargazing_look rendering entry point. */
float eye_stargazing_look(uint32_t elapsed);
/** @brief Existing eye_performance_ink rendering entry point. */
uint16_t eye_performance_ink(const EyeConfig &cfg, bool left, uint32_t elapsed);
/** @brief Existing eye_performance_motion rendering entry point. */
void eye_performance_motion(Expression expr, bool left, uint32_t phase, float a,
		float &sx, float &sy, float &dx, float &dy);
/** @brief Existing eye_performance_layout rendering entry point. */
void eye_performance_layout(const EyeConfig &cfg, EyeLayout &layout, uint32_t elapsed);
/** @brief Existing eye_equalizer_bar rendering entry point. */
EyeBox eye_equalizer_bar(const EyeConfig &cfg, const EyeLayout &layout, uint8_t index, uint32_t elapsed);
/** @brief Existing eye_equalizer_ink rendering entry point. */
uint16_t eye_equalizer_ink(uint8_t index);
/** @brief Existing eye_performance_equalizer rendering entry point. */
void eye_performance_equalizer(Arduino_GFX *gfx, const EyeConfig &cfg,
		const EyeLayout &layout, uint32_t elapsed, float a);
/** @brief Existing eye_performance_rotor rendering entry point. */
void eye_performance_rotor(Arduino_GFX *gfx, EyeConfig cfg, const EyeBox &b,
		uint32_t elapsed, float a);
/** @brief Existing eye_performance_scene rendering entry point. */
bool eye_performance_scene(Arduino_GFX *gfx, const EyeConfig &cfg,
		const EyeLayout &layout, uint32_t elapsed);
/** @brief Existing eye_performance_rain rendering entry point. */
void eye_performance_rain(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a);
/** @brief Existing eye_performance_water rendering entry point. */
void eye_performance_water(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b,
		uint32_t elapsed, float a);
/** @brief Existing eye_performance_air rendering entry point. */
void eye_performance_air(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b,
		uint32_t elapsed, float a);
/** @brief Existing eye_performance_background rendering entry point. */
void eye_performance_background(Arduino_GFX *gfx, const EyeConfig &cfg,
		const EyeLayout &layout, uint32_t elapsed);
/** @brief Existing eye_sushi_target rendering entry point. */
EyeBox eye_sushi_target(const EyeConfig &cfg);
/** @brief Existing eye_sushi_topping rendering entry point. */
void eye_sushi_topping(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &rice, float a);
/** @brief Existing eye_performance_sushi rendering entry point. */
void eye_performance_sushi(Arduino_GFX *gfx, EyeConfig cfg, const EyeLayout &layout,
		uint32_t elapsed, float a);
/** @brief Existing eye_turbo_impeller rendering entry point. */
void eye_turbo_impeller(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cx,
		int16_t cy, int16_t rx, int16_t ry, float angle, float a);
/** @brief Existing eye_turbo_target rendering entry point. */
EyeBox eye_turbo_target(const EyeConfig &cfg, bool left);
/** @brief Existing eye_turbo_smoke rendering entry point. */
void eye_turbo_smoke(Arduino_GFX *gfx, const EyeConfig &cfg, int16_t cx, int16_t cy,
		int16_t r, bool left, uint32_t elapsed, float a);
/** @brief Existing eye_performance_turbo rendering entry point. */
void eye_performance_turbo(Arduino_GFX *gfx, EyeConfig cfg, const EyeBox &b,
		uint32_t elapsed, float a);
/** @brief Existing eye_performance_stars rendering entry point. */
void eye_performance_stars(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a);
/** @brief Existing eye_extra_mood rendering entry point. */
bool eye_extra_mood(Expression expr);
/** @brief Existing eye_extra_text rendering entry point. */
void eye_extra_text(Arduino_GFX *gfx, const EyeConfig &cfg, const char *text,
		int16_t x, int16_t y, uint16_t color);
/** @brief Existing eye_extra_banner rendering entry point. */
void eye_extra_banner(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a);
/** @brief Existing eye_extra_hearts rendering entry point. */
void eye_extra_hearts(Arduino_GFX *gfx, const EyeConfig &cfg, uint32_t elapsed, float a);
/** @brief Existing eye_extra_marks rendering entry point. */
void eye_extra_marks(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float a);
/** @brief Existing eye_extra_background rendering entry point. */
void eye_extra_background(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeLayout &layout,
		uint32_t elapsed, float a);
/** @brief Existing eye_crying_ease rendering entry point. */
float eye_crying_ease(float t);
/** @brief Existing eye_crying_closure rendering entry point. */
float eye_crying_closure(uint32_t elapsed);
/** @brief Existing eye_crying_lid rendering entry point. */
void eye_crying_lid(Arduino_GFX *gfx, EyeConfig cfg, const EyeState &st, const EyeBox &b, float closure);
/** @brief Existing eye_crying_water_rows rendering entry point. */
void eye_crying_water_rows(const EyeBox &b, uint32_t elapsed, int16_t &top, int16_t &begin, int16_t &end);
/** @brief Existing eye_crying_water rendering entry point. */
void eye_crying_water(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, uint32_t elapsed, uint16_t neutral);
/** @brief Existing eye_reaction_tears rendering entry point. */
void eye_reaction_tears(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeState &st, const EyeBox &b, uint32_t elapsed, uint16_t neutral);
