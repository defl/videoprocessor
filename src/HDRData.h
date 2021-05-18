/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once

#include <memory>

/**
 * HDR primaries, whitepoint and masterting luminance
 */
struct HDRData
{
	double displayPrimaryRedX = 0;
	double displayPrimaryRedY = 0;
	double displayPrimaryGreenX = 0;
	double displayPrimaryGreenY = 0;
	double displayPrimaryBlueX = 0;
	double displayPrimaryBlueY = 0;

	double whitePointX = 0;
	double whitePointY = 0;

	double masteringDisplayMaxLuminance = 0;
	double masteringDisplayMinLuminance = 0;

	double maxCll = 0;
	double maxFall = 0;

	bool operator == (const HDRData& other) const;
	bool operator != (const HDRData& other) const;
};


typedef std::shared_ptr<HDRData> HDRDataSharedPtr;
