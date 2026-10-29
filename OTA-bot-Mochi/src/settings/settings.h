/**
 * @file settings.h
 * @brief Main-task preferences, explicit flash save, and live eye/backlight application.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "settings_model.h"
#include "../eyes/face_filter.h"

/** @brief Load a validated profile or defaults. @return Flash storage available. */
bool settings_init();
/** @brief Read main-task settings. @return Active profile. */
const CompanionSettings &settings_get();
/** @brief Apply selected eye style, expression, palette, and brightness. */
void settings_apply();
/** @brief Change a validated profile without writing flash. @param value Profile. @return Accepted. */
bool settings_update(const CompanionSettings &value);
/** @brief Save only on explicit request. @return Successful flash write or unchanged saved profile. */
bool settings_save();
/** @brief Check pending changes. @return Unsaved profile. */
bool settings_dirty();
/** @brief Read a UI revision. @return Current settings revision. */
uint32_t settings_revision();
/** @brief Restore defaults in RAM, requiring an explicit save. */
void settings_defaults();
/** @brief Replace the face allow-list in RAM, preserving at least one allowed expression. */
bool settings_faces_update(const FaceFilter &value);
/** @brief Save only the face allow-list, leaving other unsaved settings untouched. */
bool settings_faces_save();
/** @brief Detect unsaved allow-list changes. */
bool settings_faces_dirty();
/** @brief Read the allow-list revision for optimistic browser updates. */
uint32_t settings_faces_revision();
