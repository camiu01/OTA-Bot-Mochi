/**
 * @file eyes_expression.cpp
 * @brief Expression selection and case-insensitive public name lookup.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "eyes_api.h"
#include "eyes_faces.h"
#include "face_filter.h"
#include <ctype.h>

/** @brief Update the base expression without losing an active temporary reaction. @param expr Mood. */
void eyes_set_expression(Expression expr) {
	expr = face_filter().resolve(expr);
	eyes_faces_base(expr);
	EyeState &state = eyes_state();
	if (state.effectExpression == expr) { state.effectClockInit = false; }
	if (state.dizzyUntilMs != 0) { state.savedExpr = expr; return; }
	eyes_config().expression = expr;
}

/** @brief Compare a nullable input against a lowercase name. @param input Input. @param name Name. @return Match. */
static bool match_name(const char *input, const char *name) {
	if (!input || !name) { return false; }
	while (*name) {
		if (tolower((unsigned char)*input) != *name) { return false; }
		++input; ++name;
	}
	return *input == '\0';
}

/** @brief Resolve all existing expression aliases. @param name Input. @param out Output. @return Known name. */
static bool lookup_expression(const char *name, Expression &out) {
	for (uint8_t i = 0; i < EXPRESSION_COUNT; ++i) {
		Expression candidate = static_cast<Expression>(i);
		if (match_name(name, eyes_expression_name(candidate))) { out = candidate; return true; }
	}
	if (match_name(name, "saddown")) { out = SAD_DOWN; return true; }
	if (match_name(name, "sadup")) { out = SAD_UP; return true; }
	return false;
}

/** @brief Select a named expression without changing state on invalid input. @param name Name. @return Found. */
bool eyes_set_expression_by_name(const char *name) {
	Expression expression;
	if (!lookup_expression(name, expression) || !face_filter().allows(expression)) { return false; }
	eyes_set_expression(expression);
	return true;
}
