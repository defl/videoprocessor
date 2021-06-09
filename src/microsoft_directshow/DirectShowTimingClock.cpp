/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */


#include <stdafx.h>

#include "DirectShowTimingClock.h"


DirectShowTimingClock::DirectShowTimingClock(ITimingClock& timingClock):
	CBaseReferenceClock(DIRECTSHOW_TIMING_CLOCK_NAME, nullptr, nullptr, nullptr),
	m_timingClock(timingClock)
{
	DbgLog((LOG_TRACE, 1, TEXT("DirectShowTimingClock::DirectShowTimingClock()")));
}


DirectShowTimingClock::~DirectShowTimingClock()
{
	DbgLog((LOG_TRACE, 1, TEXT("DirectShowTimingClock::~DirectShowTimingClock()")));
}


REFERENCE_TIME DirectShowTimingClock::GetPrivateTime()
{
	// Should return a 100ns/tick time format

	const double ticksPer100ns = m_timingClock.GetTimingClockTicksPerSecond() / 10000000.0;
	assert(ticksPer100ns != 0.0);

	const REFERENCE_TIME rt = (REFERENCE_TIME)round(m_timingClock.GetTimingClockTime() / ticksPer100ns);
	assert(rt > 0);

	return rt;
}
