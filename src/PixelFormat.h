/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <atlstr.h>

// TODO: Make this a class and make all the below lookups properties
enum class PixelFormat
{
	UNKNOWN,
	YUV_8BIT,  // UYVY
	YUV_10BIT,  // v210
	ARGB_8BIT,
	BGRA_8BIT,
	RGB_10BIT,  // r210
	RGB_BE_10BIT,  // R10b
	RGB_LE_10BIT,  // R10l
	RGB_BE_12BIT,  // R12B
	RGB_LE_12BIT,  // R12L
	H265,
	DNxHR
};


// Return the PixelFormat as a human-readable string
const TCHAR* ToString(const PixelFormat);


// Return the the bits per pixel in this format
// Spec is similar as https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader:
//   Specifies the number of bits per pixel (bpp). For uncompressed formats, this value is the average
//   number of bits per pixel. For compressed formats, this value is the implied bit depth of the uncompressed
//   image, after the image has been decoded.
uint32_t PixelFormatBitsPerPixel(PixelFormat pixelFormat);


// Return the the FourCC code for the pixel format
// More info: https://docs.microsoft.com/en-us/windows/win32/directshow/fourcc-codes
uint32_t PixelFormatFourCC(PixelFormat pixelFormat);
