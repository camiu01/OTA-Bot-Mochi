/**
 * @file eyes_types.h
 * @brief Shared types for bloub/Mochi robot eyes.
 *
 * @purpose Define EyeStyle, Expression, EyeConfig and EyeState singletons.
 */

#ifndef EYES_TYPES_H
#define EYES_TYPES_H

#include <stdint.h>

/**
 * @brief Rendering style of the eyes.
 */
enum EyeStyle : uint8_t {
	NORMAL = 0,	/**< Flat cartoon style with small pupils. */
	MOCHI = 1,	/**< Rounded Mochi style with highlights. */
	BLOUB = 2,	/**< Default Bloub style with large rounded body. */
	TRAPEZ = 3	/**< Corner-lifted trapezoid, high-left pulled up by slant px. */
};

/**
 * @brief Emotional expression shown by the eyes.
 */
enum Expression : uint8_t {
	NEUTRAL = 0,	/**< Relaxed default gaze. */
	HAPPY = 1,	/**< Lidded cheerful eyes. */
	SAD = 2,	/**< Drooping eyelids. */
	ANGRY = 3,	/**< Angled eyelids. */
	SURPRISED = 4,	/**< Enlarged eyes. */
	SLEEPY = 5,	/**< Narrowed eyes. */
	LOVE = 6,	/**< Heart-shaped pupils. */
	DIZZY = 7,	/**< Cross-shaped wobbling pupils. */
	GLEE = 8,	/**< Joyful wide eyes with double arcs. */
	WORRIED = 9,	/**< Raised inner lids, uneasy stare. */
	FOCUSED = 10,	/**< Low bars, intense stare. */
	ANNOYED = 11,	/**< Half lids, asymmetric flat look. */
	SKEPTIC = 12,	/**< One big eye, one small eye. */
	FRUSTRATED = 13,	/**< Angled asymmetric lids. */
	UNIMPRESSED = 14,	/**< Flat half-mast lids. */
	SUSPICIOUS = 15,	/**< One high eye, one low eye. */
	SQUINT = 16,	/**< Narrowed suspicious slits. */
	FURIOUS = 17,	/**< Thick angled lids, intense glare. */
	SCARED = 18,	/**< Small narrow tall eyes. */
	AWE = 19,	/**< Huge round wonder eyes. */
	SLEEPY_EYES = 20,	/**< Heavy asymmetric drooping lids. */
	SAD_DOWN = 21,	/**< Sad gaze looking downward. */
	SAD_UP = 22	/**< Sad gaze looking upward. */
};

/**
 * @brief Persistent configuration singleton.
 */
struct EyeConfig {
	EyeStyle style;	/**< Active rendering style. */
	Expression expression;	/**< Active emotional expression. */
	uint16_t bgColor;	/**< Background clear color (RGB565). */
	uint16_t fgColor;	/**< Eye body color (RGB565). */
	uint16_t pupilColor;	/**< Pupil/overlay color (RGB565). */
	uint16_t highlightColor;	/**< Specular highlight color (RGB565). */
	int16_t eyeW;	/**< Single eye width in pixels. */
	int16_t eyeH;	/**< Single eye height in pixels. */
	int16_t eyeGap;	/**< Horizontal gap between eyes in pixels. */
	int16_t radius;	/**< Corner radius used by fillRoundRect. */
	int8_t slant;	/**< TRAPEZ corner-lift in px, pulls high-left corner up. */
	uint8_t asym;	/**< 0 symmetric, 1 per-eye geometry may vary. */
	int16_t lidTop;	/**< Eyelid line offset in px, negative raises lid. */
};

/**
 * @brief Mutable animation state singleton.
 */
struct EyeState {
	int16_t saccadeX;	/**< Random gaze offset on X axis. */
	int16_t saccadeY;	/**< Random gaze offset on Y axis. */
	int16_t tiltX;	/**< IMU-driven pupil offset on X axis. */
	int16_t tiltY;	/**< IMU-driven pupil offset on Y axis. */
	int16_t floatY;	/**< Sinusoidal idle bobbing offset. */
	float floatPhase;	/**< Phase accumulator for idle bobbing. */
	uint32_t lastMs;	/**< Timestamp of last update call. */
	uint32_t nextSaccadeMs;	/**< Scheduled time for next saccade. */
	uint32_t nextBlinkMs;	/**< Scheduled time for next blink. */
	uint32_t blinkStartMs;	/**< Timestamp when current blink started. */
	uint32_t blinkDurMs;	/**< Duration of current blink. */
	uint32_t dizzyUntilMs;	/**< Expiry time of DIZZY override, 0 when inactive. */
	bool blinking;	/**< True while eyelids are closed. */
	Expression savedExpr;	/**< Expression to restore after DIZZY. */
	int16_t prevX;	/**< X of dirty region union from last frame. */
	int16_t prevY;	/**< Y of dirty region union from last frame. */
	int16_t prevW;	/**< Width of dirty region union from last frame. */
	int16_t prevH;	/**< Height of dirty region union from last frame. */
	bool dirtyInit;	/**< False until first full-screen clear is done. */
};

#endif
