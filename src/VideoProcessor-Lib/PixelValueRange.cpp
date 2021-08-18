/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include "PixelValueRange.h"


const TCHAR* ToString(const PixelValueRange pixelValueRange)
{
	switch (pixelValueRange)
	{
	case PixelValueRange::PIXELVALUERANGE_UNKNOWN:
		return TEXT("UNKNOWN");

	case PixelValueRange::PIXELVALUERANGE_0_255:
		return TEXT("0-255");

	case PixelValueRange::PIXELVALUERANGE_16_235:
		return TEXT("16-235");
	}

	throw std::runtime_error("PixelValueRange ToString() failed, value not recognized");
}
