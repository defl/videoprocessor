/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <atlstr.h>


enum class ColorSpace
{
	UNKNOWN,
	REC_601_525,  // These 2 have slightly different primaries, last number is amount of lines
	REC_601_625,  // 625 = PAL/SECAM, 525 = NTSC SMPTE C
	REC_709,
	BT_2020
};


const TCHAR* ToString(const ColorSpace);

// Convert color sapce to Cie1931 coordinates
double ColorSpaceToCie1931RedX(ColorSpace);
double ColorSpaceToCie1931RedY(ColorSpace);
double ColorSpaceToCie1931GreenX(ColorSpace);
double ColorSpaceToCie1931GreenY(ColorSpace);
double ColorSpaceToCie1931BlueX(ColorSpace);
double ColorSpaceToCie1931BlueY(ColorSpace);
