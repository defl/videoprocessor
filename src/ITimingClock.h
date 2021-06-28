/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <stdint.h>

#include <TimingClock.h>


/**
 * Interface for objects which are a timing clock. Timing clocks are used for
 * frame and data timestamping and rendering.
 *
 * They need to be constant rate and convertable to a second,
 * there is no requirement for conversion to wall-clock time.
 */
class ITimingClock
{
public:

	/**
	 * Get the current time of the timing clock.
	 */
	virtual timingclocktime_t TimingClockNow() = 0;

	/**
	 * Get ticks/second from the timing clock
	 */
	virtual timingclocktime_t TimingClockTicksPerSecond() const = 0;

	/**
	 * Get a description of what the clock represents.
	 */
	virtual const TCHAR* TimingClockDescription() = 0;
};
