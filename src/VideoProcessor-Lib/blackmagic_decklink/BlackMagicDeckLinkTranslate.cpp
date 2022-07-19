/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#pragma warning(disable : 26812)  // class enum over class in BM API


#include <map>

#include <DeckLinkAPI_h.h>

#include "BlackMagicDeckLinkTranslate.h"


//
// See "Blackmagic DeckLink SDK.pdf" version 12.0 chapter 3.3 page 207 "Display Modes"
//

struct BMDDisplayModeData
{
	int width;
	int height;
	int fieldsPerFrame;
	BMDTimeScale timeScale;
	BMDTimeScale frameDuration;
};

static const std::map<BMDDisplayMode, BMDDisplayModeData> BD_DISPLAY_MODE_DATA =
{
	{bmdModeNTSCp,    {720, 486, 1, 60000, 1001}},
	{bmdModePALp,     {720, 576, 1, 50000, 1000}},

	{bmdModeNTSC,     {720, 486, 2, 30000, 1001}},
	{bmdModeNTSC2398, {720, 486, 2, 24000, 1001}},
	{bmdModePAL,      {720, 576, 2, 25000, 1000}},

	// 720 modes
	{bmdModeHD720p50,   {1280, 720, 1, 50000, 1000}},
	{bmdModeHD720p5994, {1280, 720, 1, 60000, 1001}},
	{bmdModeHD720p60,   {1280, 720, 1, 60000, 1000}},

	// 1080 modes
	{bmdModeHD1080p2398,  {1920, 1080, 1,  24000, 1001}},
	{bmdModeHD1080p24,    {1920, 1080, 1,  24000, 1000}},
	{bmdModeHD1080p25,    {1920, 1080, 1,  25000, 1000}},
	{bmdModeHD1080p2997,  {1920, 1080, 1,  30000, 1001}},
	{bmdModeHD1080p30,    {1920, 1080, 1,  30000, 1000}},
	{bmdModeHD1080p4795,  {1920, 1080, 1,  48000, 1001}},
	{bmdModeHD1080p48,    {1920, 1080, 1,  48000, 1000}},
	{bmdModeHD1080p50,    {1920, 1080, 1,  50000, 1000}},
	{bmdModeHD1080p5994,  {1920, 1080, 1,  60000, 1001}},
	{bmdModeHD1080p6000,  {1920, 1080, 1,  60000, 1000}},
	{bmdModeHD1080p9590,  {1920, 1080, 1,  96000, 1001}},
	{bmdModeHD1080p96,    {1920, 1080, 1,  96000, 1000}},
	{bmdModeHD1080p100,   {1920, 1080, 1, 100000, 1000}},
	{bmdModeHD1080p11988, {1920, 1080, 1, 120000, 1001}},
	{bmdModeHD1080p120,   {1920, 1080, 1, 120000, 1000}},

	{bmdModeHD1080i50,    {1920, 1080, 2,  25000, 1000}},
	{bmdModeHD1080i5994,  {1920, 1080, 2,  30000, 1001}},
	{bmdModeHD1080i6000,  {1920, 1080, 2,  30000, 1000}},

	// 2k modes
	{bmdMode2k2398,     {2048, 1556, 1,  24000, 1001}},
	{bmdMode2k24,       {2048, 1556, 1,  24000, 1000}},
	{bmdMode2k25,       {2048, 1556, 1,  25000, 1000}},

	{bmdMode2kDCI2398,  {2048, 1080, 1,  24000, 1001}},
	{bmdMode2kDCI24,    {2048, 1080, 1,  24000, 1000}},
	{bmdMode2kDCI25,    {2048, 1080, 1,  25000, 1000}},
	{bmdMode2kDCI2997,  {2048, 1080, 1,  30000, 1001}},
	{bmdMode2kDCI30,    {2048, 1080, 1,  30000, 1000}},
	{bmdMode2kDCI4795,  {2048, 1080, 1,  48000, 1001}},
	{bmdMode2kDCI48,    {2048, 1080, 1,  48000, 1000}},
	{bmdMode2kDCI50,    {2048, 1080, 1,  50000, 1000}},
	{bmdMode2kDCI5994,  {2048, 1080, 1,  60000, 1001}},
	{bmdMode2kDCI60,    {2048, 1080, 1,  60000, 1000}},
	{bmdMode2kDCI9590,  {2048, 1080, 1,  96000, 1001}},
	{bmdMode2kDCI96,    {2048, 1080, 1,  96000, 1000}},
	{bmdMode2kDCI100,   {2048, 1080, 1, 100000, 1000}},
	{bmdMode2kDCI11988, {2048, 1080, 1, 120000, 1001}},
	{bmdMode2kDCI120,   {2048, 1080, 1, 120000, 1000}},

	// 4k modes
	{bmdMode4K2160p2398,  {3840, 2160, 1,  24000, 1001}},
	{bmdMode4K2160p24,    {3840, 2160, 1,  24000, 1000}},
	{bmdMode4K2160p25,    {3840, 2160, 1,  25000, 1000}},
	{bmdMode4K2160p2997,  {3840, 2160, 1,  30000, 1001}},
	{bmdMode4K2160p30,    {3840, 2160, 1,  30000, 1000}},
	{bmdMode4K2160p4795,  {3840, 2160, 1,  48000, 1001}},
	{bmdMode4K2160p48,    {3840, 2160, 1,  48000, 1000}},
	{bmdMode4K2160p50,    {3840, 2160, 1,  50000, 1000}},
	{bmdMode4K2160p5994,  {3840, 2160, 1,  60000, 1001}},
	{bmdMode4K2160p60,    {3840, 2160, 1,  60000, 1000}},
	{bmdMode4K2160p9590,  {3840, 2160, 1,  96000, 1001}},
	{bmdMode4K2160p96,    {3840, 2160, 1,  96000, 1000}},
	{bmdMode4K2160p100,   {3840, 2160, 1, 100000, 1000}},
	{bmdMode4K2160p11988, {3840, 2160, 1, 120000, 1001}},
	{bmdMode4K2160p120,   {3840, 2160, 1, 120000, 1000}},

	{bmdMode4kDCI2398,    {4096, 2160, 1,  24000, 1001}},
	{bmdMode4kDCI24,      {4096, 2160, 1,  24000, 1000}},
	{bmdMode4kDCI25,      {4096, 2160, 1,  25000, 1000}},
	{bmdMode4kDCI2997,    {4096, 2160, 1,  30000, 1000}},
	{bmdMode4kDCI30,      {4096, 2160, 1,  30000, 1000}},
	{bmdMode4kDCI4795,    {4096, 2160, 1,  48000, 1001}},
	{bmdMode4kDCI48,      {4096, 2160, 1,  48000, 1000}},
	{bmdMode4kDCI50,      {4096, 2160, 1,  50000, 1000}},
	{bmdMode4kDCI5994,    {4096, 2160, 1,  60000, 1001}},
	{bmdMode4kDCI9590,    {4096, 2160, 1,  96000, 1001}},
	{bmdMode4kDCI96,      {4096, 2160, 1,  96000, 1000}},
	{bmdMode4kDCI100,     {4096, 2160, 1, 100000, 1000}},
	{bmdMode4kDCI11988,   {4096, 2160, 1, 120000, 1001}},
	{bmdMode4kDCI120,     {4096, 2160, 1, 120000, 1000}},

	// 8k modes
	{bmdMode8K4320p2398, {7680, 4320, 1, 24000, 1001}},
	{bmdMode8K4320p24,   {7680, 4320, 1, 24000, 1000}},
	{bmdMode8K4320p25,   {7680, 4320, 1, 25000, 1000}},
	{bmdMode8K4320p2997, {7680, 4320, 1, 30000, 1001}},
	{bmdMode8K4320p30,   {7680, 4320, 1, 30000, 1000}},
	{bmdMode8K4320p4795, {7680, 4320, 1, 48000, 1001}},
	{bmdMode8K4320p48,   {7680, 4320, 1, 48000, 1000}},
	{bmdMode8K4320p50,   {7680, 4320, 1, 50000, 1000}},
	{bmdMode8K4320p5994, {7680, 4320, 1, 60000, 1001}},
	{bmdMode8K4320p60,   {7680, 4320, 1, 60000, 1000}},

	{bmdMode8kDCI2398,   {8192, 4320, 1, 24000, 1001}},
	{bmdMode8kDCI24,     {8192, 4320, 1, 24000, 1000}},
	{bmdMode8kDCI25,     {8192, 4320, 1, 25000, 1000}},
	{bmdMode8kDCI2997,   {8192, 4320, 1, 30000, 1001}},
	{bmdMode8kDCI30,     {8192, 4320, 1, 30000, 1000}},
	{bmdMode8kDCI4795,   {8192, 4320, 1, 48000, 1001}},
	{bmdMode8kDCI48,     {8192, 4320, 1, 48000, 1000}},
	{bmdMode8kDCI50,     {8192, 4320, 1, 50000, 1000}},
	{bmdMode8kDCI5994,   {8192, 4320, 1, 60000, 1001}},
	{bmdMode8kDCI60,     {8192, 4320, 1, 60000, 1000}},

	// Computer screen modes
	{bmdMode640x480p60,   { 640,  480, 1, 60000, 1000}},
	{bmdMode800x600p60,   { 800,  600, 1, 60000, 1000}},
	{bmdMode1440x900p50,  {1440,  900, 1, 50000, 1000}},
	{bmdMode1440x900p60,  {1440,  900, 1, 60000, 1000}},
	{bmdMode1440x1080p50, {1440, 1080, 1, 50000, 1000}},
	{bmdMode1440x1080p60, {1440, 1080, 1, 60000, 1000}},
	{bmdMode1600x1200p50, {1600, 1200, 1, 50000, 1000}},
	{bmdMode1600x1200p60, {1600, 1200, 1, 60000, 1000}},
	{bmdMode1920x1200p50, {1920, 1200, 1, 50000, 1000}},
	{bmdMode1920x1200p60, {1920, 1200, 1, 60000, 1000}},
	{bmdMode1920x1440p50, {1920, 1440, 1, 50000, 1000}},
	{bmdMode1920x1440p60, {1920, 1440, 1, 60000, 1000}},
	{bmdMode2560x1440p50, {2560, 1440, 1, 50000, 1000}},
	{bmdMode2560x1440p60, {2560, 1440, 1, 60000, 1000}},
	{bmdMode2560x1600p50, {2560, 1600, 1, 50000, 1000}},
	{bmdMode2560x1600p60, {2560, 1600, 1, 60000, 1000}}
};


//
// Functions
//

ColorFormat TranslateColorFormat(BMDDetectedVideoInputFormatFlags detectedVideoInputFormatFlagsValue)
{
	if (detectedVideoInputFormatFlagsValue & bmdDetectedVideoInputYCbCr422)
		return ColorFormat::YCbCr422;

	if (detectedVideoInputFormatFlagsValue & bmdDetectedVideoInputRGB444)
		return ColorFormat::RGB444;

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


VideoFrameEncoding Translate(BMDPixelFormat bmdPixelFormat, ColorSpace colorSpace)
{
	switch (bmdPixelFormat)
	{
	case bmdFormat8BitYUV:
		// Note this can also be HDYC depending on colorspace (REC709), not implemented
		return VideoFrameEncoding::UYVY;

	case bmdFormat10BitYUV:
		return VideoFrameEncoding::V210;

	case bmdFormat8BitARGB:
		return VideoFrameEncoding::ARGB_8BIT;

	case bmdFormat8BitBGRA:
		return VideoFrameEncoding::BGRA_8BIT;

	case bmdFormat10BitRGB:
		return VideoFrameEncoding::R210;

	case bmdFormat10BitRGBX:
		return VideoFrameEncoding::R10b;

	case bmdFormat10BitRGBXLE:
		return VideoFrameEncoding::R10l;

	case bmdFormat12BitRGB:
		return VideoFrameEncoding::R12B;

	case bmdFormat12BitRGBLE:
		return VideoFrameEncoding::R12L;

	case bmdFormatH265:
		return VideoFrameEncoding::H265;

	case bmdFormatDNxHR:
		return VideoFrameEncoding::DNxHR;
	}

	throw std::runtime_error("Failed to translate BMDPixelFormat");
}


EOTF TranslateEOTF(LONGLONG electroOpticalTransferFuncValue)
{
	// 3 bit value
	assert(electroOpticalTransferFuncValue >= 0);
	assert(electroOpticalTransferFuncValue <= 7);

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
	}

	throw std::runtime_error("Failed to translate EOTF");
}


ColorSpace Translate(BMDColorspace colorSpace, uint32_t verticalLines)
{
	switch (colorSpace)
	{
	case bmdColorspaceRec601:

		if (verticalLines == 486)
			return ColorSpace::UNKNOWN;  // The card will once in a while throw an odd (invalid?) mode with 486 vertical lines for a very brief period, we simply ignore it.
		if (verticalLines == 525)
			return ColorSpace::REC_601_525;
		if (verticalLines == 576)
			return ColorSpace::REC_601_576;
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
	const auto& it = BD_DISPLAY_MODE_DATA.find(displayMode);
	if(it == BD_DISPLAY_MODE_DATA.end())
		throw std::runtime_error("Unknown BMDDisplayMode");

	assert(
		it->second.fieldsPerFrame == 1 ||
		it->second.fieldsPerFrame == 2);

	return std::make_shared<DisplayMode>(
		it->second.width,
		it->second.height,
		(it->second.fieldsPerFrame == 2),  // Interlaced?
		(unsigned int)it->second.timeScale,
		(unsigned int)it->second.frameDuration);
}


double FPS(BMDDisplayMode displayMode)
{
	const auto& it = BD_DISPLAY_MODE_DATA.find(displayMode);
	if (it == BD_DISPLAY_MODE_DATA.end())
		throw std::runtime_error("Unknown BMDDisplayMode");

	return (double)(it->second.timeScale) / (double)(it->second.frameDuration);
}
