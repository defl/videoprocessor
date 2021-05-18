/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "ColorSpace.h"


const TCHAR* ToString(const ColorSpace colorspace)
{
	switch (colorspace)
	{
	case ColorSpace::UNKNOWN:
		return TEXT("UNKNOWN");

	case ColorSpace::REC_601_525:
		return TEXT("REC.601 (NTSC)");

	case ColorSpace::REC_601_625:
		return TEXT("REC.601 (PAL/SECAM)");

	case ColorSpace::REC_709:
		return TEXT("REC.709");

	case ColorSpace::BT_2020:
		return TEXT("BT.2020");
	}

	throw std::runtime_error("Unspecified ColorSpace");
}
