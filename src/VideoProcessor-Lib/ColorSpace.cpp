/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include "ColorSpace.h"


const TCHAR* ToString(const ColorSpace colorspace)
{
	switch (colorspace)
	{
	case ColorSpace::UNKNOWN:
		return TEXT("UNKNOWN");

	case ColorSpace::REC_601_525:
		return TEXT("REC.601 (NTSC)");

	case ColorSpace::REC_601_576:
	case ColorSpace::REC_601_625:
		return TEXT("REC.601 (PAL/SECAM)");

	case ColorSpace::REC_709:
		return TEXT("REC.709");

	case ColorSpace::P3_D65:
		return TEXT("P3-D65 (Display)");

	case ColorSpace::P3_DCI:
		return TEXT("P3-DCI (Theater)");

	case ColorSpace::P3_D60:
		return TEXT("P3-D60 (ACES)");

	case ColorSpace::BT_2020:
		return TEXT("BT.2020");
	}

	throw std::runtime_error("ColorSpace ToString() failed, value not recognized");
}


//
//Coordinates from:
// - https://en.wikipedia.org/wiki/Rec._2020
// - https://en.wikipedia.org/wiki/DCI-P3
// - https://en.wikipedia.org/wiki/Rec._709
// - https://en.wikipedia.org/wiki/Rec._601
//


double ColorSpaceToCie1931RedX(ColorSpace colorspace)
{
	switch (colorspace)
	{
	case ColorSpace::BT_2020:
		return 0.708;
	case ColorSpace::P3_D65:
	case ColorSpace::P3_DCI:
	case ColorSpace::P3_D60:
		return 0.68;
	case ColorSpace::REC_709:
		return 0.64;
	case ColorSpace::REC_601_525:
		return 0.63;
	case ColorSpace::REC_601_576:
	case ColorSpace::REC_601_625:
		return 0.64;
	}

	throw std::runtime_error("Cannot convert colorspace to CIE1931 coordinate");
}


double ColorSpaceToCie1931RedY(ColorSpace colorspace)
{
	switch (colorspace)
	{
	case ColorSpace::BT_2020:
		return 0.292;
	case ColorSpace::P3_D65:
	case ColorSpace::P3_DCI:
	case ColorSpace::P3_D60:
		return 0.32;
	case ColorSpace::REC_709:
		return 0.33;
	case ColorSpace::REC_601_525:
		return 0.34;
	case ColorSpace::REC_601_576:
	case ColorSpace::REC_601_625:
		return 0.33;
	}

	throw std::runtime_error("Cannot convert colorspace to CIE1931 coordinate");
}


double ColorSpaceToCie1931GreenX(ColorSpace colorspace)
{
	switch (colorspace)
	{
	case ColorSpace::BT_2020:
		return  0.17;
	case ColorSpace::P3_D65:
	case ColorSpace::P3_DCI:
	case ColorSpace::P3_D60:
		return 0.265;
	case ColorSpace::REC_709:
		return 0.30;
	case ColorSpace::REC_601_525:
		return 0.310;
	case ColorSpace::REC_601_576:
	case ColorSpace::REC_601_625:
		return 0.290;
	}

	throw std::runtime_error("Cannot convert colorspace to CIE1931 coordinate");
}


double ColorSpaceToCie1931GreenY(ColorSpace colorspace)
{
	switch (colorspace)
	{
	case ColorSpace::BT_2020:
		return 0.797;
	case ColorSpace::P3_D65:
	case ColorSpace::P3_DCI:
	case ColorSpace::P3_D60:
		return 0.69;
	case ColorSpace::REC_709:
		return 0.60;
	case ColorSpace::REC_601_525:
		return 0.595;
	case ColorSpace::REC_601_576:
	case ColorSpace::REC_601_625:
		return 0.600;
	}

	throw std::runtime_error("Cannot convert colorspace to CIE1931 coordinate");
}


double ColorSpaceToCie1931BlueX(ColorSpace colorspace)
{
	switch (colorspace)
	{
	case ColorSpace::BT_2020:
		return 0.131;
	case ColorSpace::P3_D65:
	case ColorSpace::P3_DCI:
	case ColorSpace::P3_D60:
		return 0.15;
	case ColorSpace::REC_709:
		return 0.15;
	case ColorSpace::REC_601_525:
		return 0.155;
	case ColorSpace::REC_601_576:
	case ColorSpace::REC_601_625:
		return 0.150;
	}

	throw std::runtime_error("Cannot convert colorspace to CIE1931 coordinate");
}


double ColorSpaceToCie1931BlueY(ColorSpace colorspace)
{
	switch (colorspace)
	{
	case ColorSpace::BT_2020:
		return 0.046;
	case ColorSpace::P3_D65:
	case ColorSpace::P3_DCI:
	case ColorSpace::P3_D60:
		return 0.06;
	case ColorSpace::REC_709:
		return 0.06;
	case ColorSpace::REC_601_525:
		return 0.07;
	case ColorSpace::REC_601_576:
	case ColorSpace::REC_601_625:
		return 0.06;
	}

	throw std::runtime_error("Cannot convert colorspace to CIE1931 coordinate");
}


double ColorSpaceToCie1931WpX(ColorSpace colorspace)
{
	switch (colorspace)
	{
	case ColorSpace::BT_2020:
	case ColorSpace::P3_D65:
	case ColorSpace::REC_709:
	case ColorSpace::REC_601_525:
	case ColorSpace::REC_601_576:
	case ColorSpace::REC_601_625:
		return 0.3127;
	case ColorSpace::P3_DCI:
		return 0.314;
	case ColorSpace::P3_D60:
		return 0.32168;
	}

	throw std::runtime_error("Cannot convert colorspace to CIE1931 coordinate");
}


double ColorSpaceToCie1931WpY(ColorSpace colorspace)
{
	switch (colorspace)
	{
	case ColorSpace::BT_2020:
	case ColorSpace::P3_D65:
	case ColorSpace::REC_709:
	case ColorSpace::REC_601_525:
	case ColorSpace::REC_601_576:
	case ColorSpace::REC_601_625:
		return 0.329;
	case ColorSpace::P3_DCI:
		return 0.351;
	case ColorSpace::P3_D60:
		return 0.33767;
	}

	throw std::runtime_error("Cannot convert colorspace to CIE1931 coordinate");
}
