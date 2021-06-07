/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdafx.h>


#include "HDRData.h"
#include <cie.h>


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

		// TODO: Make dedicated luminance comparators
		fabs(masteringDisplayMaxLuminance-other.masteringDisplayMaxLuminance) < 0.0001 &&
		fabs(masteringDisplayMinLuminance-other.masteringDisplayMinLuminance) < 0.0001 &&

		fabs(maxCll-other.maxCll) < 0.0001 &&
		fabs(maxFall-other.maxFall) < 0.0001;
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
		masteringDisplayMinLuminance > 0 &&
		maxCll > 0 &&
		maxFall > 0;
}
