/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <atlstr.h>


/**
 * This enum is used to determin which clock will be used for
 * timing.
 */
enum TimingClockType
{
	// Nothing set
	TIMING_CLOCK_UNKNOWN = -1,

	// No timing clock (freerunning output)
	TIMING_CLOCK_NONE = 0,

	// Time comes from OS. Taken at first possible opportunity
	TIMING_CLOCK_OS = 1,

	// Time comes from capture device video stream
	TIMING_CLOCK_VIDEO_STREAM = 2
};


// All from the enum above in descending order of preference
static const TimingClockType TimingClockTypes[] = {
	TIMING_CLOCK_VIDEO_STREAM,
	TIMING_CLOCK_OS,
	TIMING_CLOCK_NONE
};


const TCHAR* ToString(const TimingClockType timingClock);
