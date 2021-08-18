/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */


#include <pch.h>

#include "DirectShowTimingClock.h"


DirectShowTimingClock::DirectShowTimingClock(ITimingClock& timingClock):
	CBaseReferenceClock(DIRECTSHOW_TIMING_CLOCK_NAME, nullptr, nullptr, nullptr),
	m_timingClock(timingClock),
	m_ticksPer100ns(m_timingClock.TimingClockTicksPerSecond() / 10000000.0)
{
	DbgLog((LOG_TRACE, 1, TEXT("DirectShowTimingClock::DirectShowTimingClock()")));

	assert(m_ticksPer100ns > 0);
}


DirectShowTimingClock::~DirectShowTimingClock()
{
	DbgLog((LOG_TRACE, 1, TEXT("DirectShowTimingClock::~DirectShowTimingClock()")));
}


REFERENCE_TIME DirectShowTimingClock::GetPrivateTime()
{
	const REFERENCE_TIME rt = (REFERENCE_TIME)(m_timingClock.TimingClockNow() / m_ticksPer100ns);
	assert(rt > 0);

	return rt;
}
