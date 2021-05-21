/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#pragma warning(disable : 26812)  // class enum over class in BM API

#include "BlackMagicDeckLinkTranslate.h"


Encoding TranslateEncoding(BMDDetectedVideoInputFormatFlags detectedVideoInputFormatFlagsValue)
{
	if (detectedVideoInputFormatFlagsValue & bmdDetectedVideoInputYCbCr422)
		return Encoding::YCbCr422;

	if (detectedVideoInputFormatFlagsValue & bmdDetectedVideoInputRGB444)
		return Encoding::RGB444;

	throw std::runtime_error("Failed to translate BMDDetectedVideoInputFormatFlags to encoding");
}


BitDepth TranslateBithDepth(BMDDetectedVideoInputFormatFlags detectedVideoInputFormatFlagsValue)
{
	if (detectedVideoInputFormatFlagsValue & bmdDetectedVideoInput8BitDepth)
		return BitDepth::BITDEPTH_8BIT;

	if (detectedVideoInputFormatFlagsValue & bmdDetectedVideoInput10BitDepth)
		return BitDepth::BITDEPTH_10BIT;

	if (detectedVideoInputFormatFlagsValue & bmdDetectedVideoInput12BitDepth)
		return BitDepth::BITDEPTH_12BIT;

	throw std::runtime_error("Failed to translate BMDDetectedVideoInputFormatFlags to bit depth");
}


PixelFormat Translate(BMDPixelFormat pixelFormat)
{
	switch (pixelFormat)
	{
	case bmdFormat8BitYUV:
		return PixelFormat::YUV_8BIT;

	case bmdFormat10BitYUV:
		return PixelFormat::YUV_10BIT;

	case bmdFormat8BitARGB:
		return PixelFormat::ARGB_8BIT;

	case bmdFormat8BitBGRA:
		return PixelFormat::BGRA_8BIT;

	case bmdFormat10BitRGB:
		return PixelFormat::RGB_10BIT;

	case bmdFormat10BitRGBX:
		return PixelFormat::RGB_BE_10BIT;

	case bmdFormat10BitRGBXLE:
		return PixelFormat::RGB_LE_10BIT;

	case bmdFormat12BitRGB:
		return PixelFormat::RGB_BE_12BIT;

	case bmdFormat12BitRGBLE:
		return PixelFormat::RGB_LE_12BIT;

	case bmdFormatH265:
		return PixelFormat::H265;

	case bmdFormatDNxHR:
		return PixelFormat::DNxHR;
	}

	throw std::runtime_error("Failed to translate BMDPixelFormat");
}


EOTF TranslateEOTF(LONGLONG electroOpticalTransferFuncValue)
{
	// Comments in the SDK 12 docs: "EOTF in range 0-7 as per CEA 861.3",
	// which is "A2016 HDR STATIC METADATA EXTENSIONS".

	switch (electroOpticalTransferFuncValue)
	{
	case 0:
		return EOTF::SDR;

	case 1:
		return EOTF::HDR;

	case 2:
		return EOTF::PQ;

	case 3:
		return EOTF::HLG;

	// 4-7 are reserved for future use
	// Higher values not possible, spec only has 3 bits
	}

	throw std::runtime_error("Failed to translate EOTF");
}


ColorSpace Translate(BMDColorspace colorSpace, uint32_t verticalLines)
{
	switch (colorSpace)
	{
	case bmdColorspaceRec601:
		if (verticalLines == 525)
			return ColorSpace::REC_601_525;
		if (verticalLines == 625)
			return ColorSpace::REC_601_625;

		throw std::runtime_error("Unknown amount of lines for REC 601 color space");

	case bmdColorspaceRec709:
		return ColorSpace::REC_709;

	case bmdColorspaceRec2020:
		return ColorSpace::BT_2020;
	}

	throw std::runtime_error("Failed to translate ColorSpace");
}


DisplayModeSharedPtr Translate(BMDDisplayMode displayMode)
{
	// See "Blackmagic DeckLink SDK.pdf" chater "Display Modes"
	switch (displayMode)
	{
	case bmdModeHD720p50:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_720p_50);
	case bmdModeHD720p5994:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_720p_59_94);
	case bmdModeHD720p60:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_720p_60);

	case bmdModeHD1080p2398:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_1080p_23_976);
	case bmdModeHD1080p24:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_1080p_24);
	case bmdModeHD1080p25:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_1080p_25);
	case bmdModeHD1080p2997:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_1080p_29_97);
	case bmdModeHD1080p30:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_1080p_30);
	case bmdModeHD1080p4795:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_1080p_47_95);
	case bmdModeHD1080p48:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_1080p_48);
	case bmdModeHD1080p50:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_1080p_50);
	case bmdModeHD1080p5994:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_1080p_59_94);
	case bmdModeHD1080p6000:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_1080p_60);

	case bmdMode2k2398:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_2KFULLFRAME_23_976);
	case bmdMode2k24:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_2KFULLFRAME_24);
	case bmdMode2k25:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_2KFULLFRAME_25);

	case bmdMode2kDCI2398:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_2KDCI_23_976);
	case bmdMode2kDCI24:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_2KDCI_24);
	case bmdMode2kDCI25:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_2KDCI_25);
	case bmdMode2kDCI2997:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_2KDCI_29_97);
	case bmdMode2kDCI30:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_2KDCI_30);
	case bmdMode2kDCI4795:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_2KDCI_47_95);
	case bmdMode2kDCI48:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_2KDCI_48);
	case bmdMode2kDCI50:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_2KDCI_50);
	case bmdMode2kDCI5994:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_2KDCI_59_94);
	case bmdMode2kDCI60:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_2KDCI_60);

	case bmdMode4K2160p2398:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4K_23_976);
	case bmdMode4K2160p24:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4K_24);
	case bmdMode4K2160p25:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4K_25);
	case bmdMode4K2160p2997:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4K_29_97);
	case bmdMode4K2160p30:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4K_30);
	case bmdMode4K2160p4795:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4K_47_95);
	case bmdMode4K2160p48:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4K_48);
	case bmdMode4K2160p50:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4K_50);
	case bmdMode4K2160p5994:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4K_59_94);
	case bmdMode4K2160p60:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4K_60);

	case bmdMode4kDCI2398:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4KDCI_23_976);
	case bmdMode4kDCI24:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4KDCI_24);
	case bmdMode4kDCI25:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4KDCI_25);
	case bmdMode4kDCI2997:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4KDCI_29_97);
	case bmdMode4kDCI30:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4KDCI_30);
	case bmdMode4kDCI4795:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4KDCI_47_95);
	case bmdMode4kDCI48:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4KDCI_48);
	case bmdMode4kDCI50:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4KDCI_50);
	case bmdMode4kDCI5994:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4KDCI_59_94);
	case bmdMode4kDCI60:
		return std::shared_ptr< DisplayMode>(new DISPLAYMODE_4KDCI_60);

	// Few screen modes
	case bmdMode640x480p60:
		return std::make_shared<DisplayMode>(640, 480, 60000);
	case bmdMode800x600p60:
		return std::make_shared<DisplayMode>(800, 600, 60000);
	case bmdMode1440x900p50:
		return std::make_shared<DisplayMode>(1440, 900, 50000);
	case bmdMode1440x900p60:
		return std::make_shared<DisplayMode>(1440, 900, 60000);
	case bmdMode1440x1080p50:
		return std::make_shared<DisplayMode>(1440, 1080, 50000);
	case bmdMode1440x1080p60:
		return std::make_shared<DisplayMode>(1440, 1080, 60000);
	case bmdMode1600x1200p50:
		return std::make_shared<DisplayMode>(1600, 1200, 50000);
	case bmdMode1600x1200p60:
		return std::make_shared<DisplayMode>(1600, 1200, 60000);
	case bmdMode1920x1200p50:
		return std::make_shared<DisplayMode>(1920, 1200, 50000);
	case bmdMode1920x1200p60:
		return std::make_shared<DisplayMode>(1920, 1200, 60000);
	case bmdMode1920x1440p50:
		return std::make_shared<DisplayMode>(1920, 1440, 50000);
	case bmdMode1920x1440p60:
		return std::make_shared<DisplayMode>(1920, 1440, 60000);
	case bmdMode2560x1440p50:
		return std::make_shared<DisplayMode>(2560, 1440, 50000);
	case bmdMode2560x1440p60:
		return std::make_shared<DisplayMode>(2560, 1440, 60000);
	case bmdMode2560x1600p50:
		return std::make_shared<DisplayMode>(2560, 1600, 50000);
	case bmdMode2560x1600p60:
		return std::make_shared<DisplayMode>(2560, 1600, 60000);

	// Unhandled modes (past and future formats + interlaced)
	case bmdModeNTSC:
	case bmdModeNTSC2398:
	case bmdModePAL:
	case bmdModeNTSCp:
	case bmdModePALp:
	case bmdModeHD1080i50:
	case bmdModeHD1080i5994:
	case bmdModeHD1080i6000:
	case bmdMode2kDCI9590:
	case bmdMode2kDCI96:
	case bmdMode2kDCI100:
	case bmdMode2kDCI11988:
	case bmdMode2kDCI120:
	case bmdMode4K2160p9590:
	case bmdMode4K2160p96:
	case bmdMode4K2160p100:
	case bmdMode4K2160p11988:
	case bmdMode4K2160p120:
	case bmdMode4kDCI9590:
	case bmdMode4kDCI96:
	case bmdMode4kDCI100:
	case bmdMode4kDCI11988:
	case bmdMode4kDCI120:
	case bmdMode8K4320p2398:
	case bmdMode8K4320p24:
	case bmdMode8K4320p25:
	case bmdMode8K4320p2997:
	case bmdMode8K4320p30:
	case bmdMode8K4320p4795:
	case bmdMode8K4320p48:
	case bmdMode8K4320p50:
	case bmdMode8K4320p5994:
	case bmdMode8K4320p60:
	case bmdMode8kDCI2398:
	case bmdMode8kDCI24:
	case bmdMode8kDCI25:
	case bmdMode8kDCI2997:
	case bmdMode8kDCI30:
	case bmdMode8kDCI4795:
	case bmdMode8kDCI48:
	case bmdMode8kDCI50:
	case bmdMode8kDCI5994:
	case bmdMode8kDCI60:
		throw std::runtime_error("Known but unhandled display mode");
	}

	throw std::runtime_error("Failed to translate DisplayMode");
}
