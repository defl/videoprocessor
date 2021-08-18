/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include "BitDepth.h"


const TCHAR* ToString(const BitDepth bitDepth)
{
	switch (bitDepth)
	{
	case BitDepth::UNKNOWN:
		return TEXT("UNKNOWN");

	case BitDepth::BITDEPTH_8BIT:
		return TEXT("8-bit");

	case BitDepth::BITDEPTH_10BIT:
		return TEXT("10-bit");

	case BitDepth::BITDEPTH_12BIT:
		return TEXT("12-bit");
	}

	throw std::runtime_error("BitDepth ToString() failed, value not recognized");
}
