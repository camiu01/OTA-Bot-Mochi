/**
 * @file ancs_service.h
 * @brief iPhone notification transport over the existing NimBLE server link.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

/** @brief Start an iPhone-only NimBLE peripheral and ANCS worker. @return True when ready. */
bool ancs_begin();
