/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include "cie.h"


bool CieValidColor(double color)
{
	// In some exotic modes this can be just over 1.08
	return (color >= 0) && (color <= 1.1);
}


bool CieEquals(double color1, double color2)
{
	assert(CieValidColor(color1));
	assert(CieValidColor(color2));

	return fabs(color1 - color2) < 0.0001;
}


CString CieXYToString(double x, double y)
{
	// Whitepoints
	// https://en.wikipedia.org/wiki/Standard_illuminant
	if (CieEquals(0.34567, x) && CieEquals(0.35850, y))
		return TEXT("D50");
	if (CieEquals(0.33242, x) && CieEquals(0.34743, y))
		return TEXT("D55");
	if (CieEquals(0.31271, x) && CieEquals(0.32902, y))
		return TEXT("D65");
	if (CieEquals(0.29902, x) && CieEquals(0.31485, y))
		return TEXT("D75");

	// Shared ones
	if (CieEquals(0.640, x) && CieEquals(0.330, y))
		return TEXT("709/601 R");
	if (CieEquals(0.150, x) && CieEquals(0.060, y))
		return TEXT("P3/601/709 B");

	// BT2020
	// https://en.wikipedia.org/wiki/Rec._2020
	if (CieEquals(0.708, x) && CieEquals(0.292, y))
		return TEXT("BT2020 R");
	if (CieEquals(0.17, x) && CieEquals(0.797, y))
		return TEXT("BT2020 G");
	if (CieEquals(0.131, x) && CieEquals(0.046, y))
		return TEXT("BT2020 B");

	// DCI-P3
	// https://en.wikipedia.org/wiki/DCI-P3
	if (CieEquals(0.680, x) && CieEquals(0.320, y))
		return TEXT("D.P3 R");
	if (CieEquals(0.265, x) && CieEquals(0.690, y))
		return TEXT("D.P3 G");
	// blue is a shared one

	// Rec 709
	// https://en.wikipedia.org/wiki/Rec._709
	// Red is a shared one
	if (CieEquals(0.300, x) && CieEquals(0.600, y))
		return TEXT("R.709 G");
	// blue is a shared one

	// Rec 601
	// https://en.wikipedia.org/wiki/Rec._601
	// Red is a shared one
	if (CieEquals(0.290, x) && CieEquals(0.600, y))
		return TEXT("R.601-625 G");
	// blue is a shared one

	if (CieEquals(0.630, x) && CieEquals(0.340, y))
		return TEXT("R.601-525 R");
	if (CieEquals(0.310, x) && CieEquals(0.595, y))
		return TEXT("R.601-525 G");
	if (CieEquals(0.155, x) && CieEquals(0.070, y))
		return TEXT("R.601-525 B");

	// Unknown, just give coordinates
	CString str;
	str.Format(_T("%.03f,%.03f"), x, y);
	return str;
}
