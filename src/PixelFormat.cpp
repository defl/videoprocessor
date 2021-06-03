/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include <Aviriff.h>

#include "PixelFormat.h"


const TCHAR* ToString(const PixelFormat pixelFormat)
{
	switch (pixelFormat)
	{
	case PixelFormat::UNKNOWN:
		return TEXT("UNKNOWN");

	case PixelFormat::YUV_8BIT:
		return TEXT("YUV 8-bit (UYVY)");

	case PixelFormat::YUV_10BIT:
		return TEXT("YUV 10-bit (v210)");

	case PixelFormat::ARGB_8BIT:
		return TEXT("ARGB 8-bit");

	case PixelFormat::BGRA_8BIT:
		return TEXT("BGRA 8-bit");

	case PixelFormat::R210:
		return TEXT("RGB 10-bit (r210)");

	case PixelFormat::RGB_BE_10BIT:
		return TEXT("RGB Big-Endian 10-bit (R10b)");

	case PixelFormat::RGB_LE_10BIT:
		return TEXT("RGB Little-Endian 10-bit (R10l)");

	case PixelFormat::RGB_BE_12BIT:
		return TEXT("RGB Big-Endian 12-bit (R12B)");

	case PixelFormat::RGB_LE_12BIT:
		return TEXT("RGB Little-Endian 12-bit (R12L)");

	case PixelFormat::H265:
		return TEXT("H.265 Encoded");

	case PixelFormat::DNxHR:
		return TEXT("DNxHR Encoded");
	}

	throw std::runtime_error("UNSPECIFIED pixelMode");
}


uint32_t PixelFormatBitsPerPixel(PixelFormat pixelFormat)
{
	switch (pixelFormat)
	{
	case PixelFormat::YUV_8BIT:
		return 16;

	case PixelFormat::YUV_10BIT:
		return 20;

	case PixelFormat::ARGB_8BIT:
	case PixelFormat::BGRA_8BIT:
		return 32;

	case PixelFormat::R210:
		return 30;

	case PixelFormat::RGB_BE_12BIT:
		return 36/8;  // Guess
	}

	throw std::runtime_error("Don't know how to bits per pixel for given format");
}


// ffmpeg riff.c is great for this
uint32_t PixelFormatFourCC(PixelFormat pixelFormat)
{
	switch (pixelFormat)
	{
	case PixelFormat::YUV_8BIT:
		return FCC('UYVY');

	case PixelFormat::YUV_10BIT:
		return FCC('v210');
	}

	throw std::runtime_error("Don't know fourCC for given format");
}
