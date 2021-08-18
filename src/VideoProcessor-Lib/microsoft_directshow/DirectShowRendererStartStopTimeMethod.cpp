/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include "DirectShowRendererStartStopTimeMethod.h"


const TCHAR* ToString(const DirectShowStartStopTimeMethod rendererTimestamp)
{
	switch (rendererTimestamp)
	{
	case DirectShowStartStopTimeMethod::DS_SSTM_CLOCK_SMART:
		return TEXT("Clock-Smart");

	case DirectShowStartStopTimeMethod::DS_SSTM_CLOCK_THEO:
		return TEXT("Clock-Theo");

	case DirectShowStartStopTimeMethod::DS_SSTM_CLOCK_CLOCK:
		return TEXT("Clock-Clock");

	case DirectShowStartStopTimeMethod::DS_SSTM_THEO_THEO:
		return TEXT("Theo-Theo");

	case DirectShowStartStopTimeMethod::DS_SSTM_CLOCK_NONE:
		return TEXT("Clock-None");

	case DirectShowStartStopTimeMethod::DS_SSTM_THEO_NONE:
		return TEXT("Theo-None");

	case DirectShowStartStopTimeMethod::DS_SSTM_NONE:
		return TEXT("None");
	}

	throw std::runtime_error("DirectShowStartStopTimeMethod ToString() failed, value not recognized");
}
