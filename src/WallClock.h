/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>

// Time in 100ns since epoch.
typedef int64_t timestamp_t;

const static timestamp_t MIN_TIMESTAMP = 16228512000000000LL;  // 2021-06-05 0Z
const static timestamp_t TICKS_PER_SECOND = 10000000LL;  // The smallest unit of representable time(100ns) is called a tick.

/**
 * Current time in 100ns since epoch
 */
timestamp_t GetWallClockTime();
