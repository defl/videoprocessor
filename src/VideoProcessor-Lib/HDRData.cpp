/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once

#include <pch.h>

#include <cie.h>

#include "HDRData.h"


HDRData::HDRData(const HDRData& other)
{
	displayPrimaryRedX = other.displayPrimaryRedX;
	displayPrimaryRedY = other.displayPrimaryRedY;
	displayPrimaryGreenX = other.displayPrimaryGreenX;
	displayPrimaryGreenY = other.displayPrimaryGreenY;
	displayPrimaryBlueX = other.displayPrimaryBlueX;
	displayPrimaryBlueY = other.displayPrimaryBlueY;

	whitePointX = other.whitePointX;
	whitePointY = other.whitePointY;

	masteringDisplayMaxLuminance = other.masteringDisplayMaxLuminance;
	masteringDisplayMinLuminance = other.masteringDisplayMinLuminance;

	maxCll = other.maxCll;
	maxFall = other.maxFall;
}


bool HDRData::operator == (const HDRData& other) const
{
	return
		CieEquals(displayPrimaryRedX, other.displayPrimaryRedX) &&
		CieEquals(displayPrimaryRedY, other.displayPrimaryRedY) &&
		CieEquals(displayPrimaryGreenX, other.displayPrimaryGreenX) &&
		CieEquals(displayPrimaryGreenY, other.displayPrimaryGreenY) &&
		CieEquals(displayPrimaryBlueX, other.displayPrimaryBlueX) &&
		CieEquals(displayPrimaryBlueY, other.displayPrimaryBlueY) &&

		CieEquals(whitePointX, other.whitePointX) &&
		CieEquals(whitePointY, other.whitePointY) &&

		LumenEqual(masteringDisplayMaxLuminance, other.masteringDisplayMaxLuminance) &&
		LumenEqual(masteringDisplayMinLuminance, other.masteringDisplayMinLuminance) &&

		LumenEqual(maxCll, other.maxCll) &&
		LumenEqual(maxFall, other.maxFall);
}


bool HDRData::operator != (const HDRData& other) const
{
	return !(*this == other);
}


bool HDRData::IsValid() const
{
	return
		displayPrimaryRedX > 0 &&
		displayPrimaryRedY > 0 &&
		displayPrimaryGreenX > 0 &&
		displayPrimaryGreenY > 0 &&
		displayPrimaryBlueX > 0 &&
		displayPrimaryBlueY > 0 &&
		whitePointX > 0 &&
		whitePointY > 0 &&
		masteringDisplayMaxLuminance > 0 &&

		// These are soft checks, zero might or might not be appropriate but it helps to accept some edge cases
		// of missing or very enthousiatically set metadata
		masteringDisplayMinLuminance >= 0 &&
		maxCll >= 0 &&
		maxFall >= 0;
}


bool LumenEqual(double a, double b)
{
	return fabs(a - b) < 0.00001;
}
