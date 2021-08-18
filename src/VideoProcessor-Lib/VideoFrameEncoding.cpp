/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include <Aviriff.h>

#include "VideoFrameEncoding.h"


const TCHAR* ToString(const VideoFrameEncoding videoFrameEncoding)
{
	switch (videoFrameEncoding)
	{
	case VideoFrameEncoding::UNKNOWN:
		return TEXT("UNKNOWN");

	case VideoFrameEncoding::UYVY:
		return TEXT("YUV 8-bit (UYVY)");

	case VideoFrameEncoding::HDYC:
		return TEXT("YUV 8-bit (HDYC)");

	case VideoFrameEncoding::V210:
		return TEXT("YUV 10-bit (v210)");

	case VideoFrameEncoding::ARGB_8BIT:
		return TEXT("ARGB 8-bit");

	case VideoFrameEncoding::BGRA_8BIT:
		return TEXT("BGRA 8-bit");

	case VideoFrameEncoding::R210:
		return TEXT("RGB 10-bit (r210)");

	case VideoFrameEncoding::R10b:
		return TEXT("RGB Big-Endian 10-bit (R10b)");

	case VideoFrameEncoding::R10l:
		return TEXT("RGB Little-Endian 10-bit (R10l)");

	case VideoFrameEncoding::R12B:
		return TEXT("RGB Big-Endian 12-bit (R12B)");

	case VideoFrameEncoding::R12L:
		return TEXT("RGB Little-Endian 12-bit (R12L)");

	case VideoFrameEncoding::H265:
		return TEXT("H.265 Encoded");

	case VideoFrameEncoding::DNxHR:
		return TEXT("DNxHR Encoded");
	}

	throw std::runtime_error("VideoFrameEncoding ToString() failed, value not recognized");
}


uint32_t VideoFrameEncodingBitsPerPixel(const VideoFrameEncoding videoFrameEncoding)
{
	switch (videoFrameEncoding)
	{
	case VideoFrameEncoding::UYVY:
	case VideoFrameEncoding::HDYC:
		return 16;

	case VideoFrameEncoding::V210:
		return 10;

	case VideoFrameEncoding::ARGB_8BIT:
	case VideoFrameEncoding::BGRA_8BIT:
		return 32;

	case VideoFrameEncoding::R210:
		return 30;

	case VideoFrameEncoding::R12B:
		return 36/8;  // Guess
	}

	throw std::runtime_error("Don't know how to bits per pixel for VideoFrameEncoding");
}


uint32_t VideoFrameEncodingFourCC(const VideoFrameEncoding videoFrameEncoding)
{
	// ffmpeg riff.c is great for this

	switch (videoFrameEncoding)
	{
	case VideoFrameEncoding::UYVY:
		return FCC('UYVY');

	case VideoFrameEncoding::HDYC:
		return FCC('HDYC');

	case VideoFrameEncoding::V210:
		return FCC('v210');
	}

	throw std::runtime_error("Don't know fourCC for VideoFrameEncoding");
}
