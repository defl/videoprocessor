/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <stdint.h>


typedef int64_t timingclocktime_t;


/**
 * Interface for objects which are a timing clock
 */
class ITimingClock
{
public:

	/**
	 * Get the timestamp of the timing clock
	 */
	virtual timingclocktime_t GetTimingClockTime() = 0;

	/**
	 * Get ticks/second from the timing clock
	 */
	virtual timingclocktime_t GetTimingClockTicksPerSecond() const = 0;
};
