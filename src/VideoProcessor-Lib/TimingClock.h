/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <stdint.h>
#include <limits>


typedef int64_t timingclocktime_t;


const static timingclocktime_t TIMING_CLOCK_TIME_INVALID = std::numeric_limits<timingclocktime_t>::lowest();


// Return the dif in various units of time from a to b and expressed in ms.
// If stop > start then the result will be positive
double TimingClockDiffMs(timingclocktime_t start, timingclocktime_t stop, timingclocktime_t ticksPerSecond);
