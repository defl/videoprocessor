/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

 // https://en.wikipedia.org/wiki/CIE_1931_color_space

#pragma once

#include <atlstr.h>


// Return true if the color is a valid value for a CIE color
bool CieValidColor(double color);


// Returns true if 2 values are indisinguishable in CIE color space
bool CieEquals(double color1, double color2);


// Return the string representation of an XY coordinate
CString CieXYToString(double x, double y);
