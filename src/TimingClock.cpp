/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "TimingClock.h"


const TCHAR* ToString(const TimingClockType timingClock)
{
	switch (timingClock)
	{
	case TimingClockType::TIMING_CLOCK_NONE:
		return TEXT("None");

	case TimingClockType::TIMING_CLOCK_OS:
		return TEXT("OS");

	case TimingClockType::TIMING_CLOCK_VIDEO_STREAM:
		return TEXT("Video stream");
	}

	throw std::runtime_error("UNSPECIFIED TimingClock");
}
