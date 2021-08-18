/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <atlstr.h>

/**
 * We define this as how the frame data is encoded.
 * (This is a union between pixelformat and decoder in ffmpeg terms.)
 */
enum class VideoFrameEncoding
{
	UNKNOWN,

	// UYVY & HDYC 4:2:2
	// There are 2 similar types with the main difference being which color space they're used in
	UYVY,  // BT470 SD video color space
	HDYC,  // BT.709

	// v210 4:2:2
	// Twelve 10-bit unsigned components are packed into four 32-bit little-endian words
	V210,

	// ARGB (or ARGB32) 4:4:4:4 raw
	// Four 8-bit unsigned components are packed into one 32-bit little-endian word. Alpha channel is valid.
	ARGB_8BIT,

	// BGRA (or RGB32) 4:4:4:x raw
	// Four 8-bit unsigned components are packed into one 32-bit little-endian word. The alpha channel may be valid.
	BGRA_8BIT,

	// r210: 4:4:4 raw
	// Three 10-bit unsigned components are packed into one 32-bit big-endian word.
	R210,

	// R10b: 4:4:4 raw
	// Three 10-bit unsigned components are packed into one 32-bit big-endian word.
	R10b,

	// R10l: 4:4:4 raw
	// Three 10-bit unsigned components are packed into one 32-bit little-endian word.
	R10l,

	// R12B: Big-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component.
	// (SMPTE 268M Digital Moving-Picture Exchange version 1, Annex C, Method C4 packing.)
	R12B,

	// R12L: Little-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component.
	// (SMPTE 268M Digital Moving-Picture Exchange version 1, Annex C, Method C4 packing.)
	R12L,

	H265,
	DNxHR
};


// Return the VideoFrameEncoding as a human-readable string
const TCHAR* ToString(const VideoFrameEncoding);


// Return the the bits per pixel in this format
// Spec is similar as https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader:
//   Specifies the number of bits per pixel (bpp). For uncompressed formats, this value is the average
//   number of bits per pixel. For compressed formats, this value is the implied bit depth of the uncompressed
//   image, after the image has been decoded.
uint32_t VideoFrameEncodingBitsPerPixel(VideoFrameEncoding);


// Return the the FourCC code for the pixel format
// More info: https://docs.microsoft.com/en-us/windows/win32/directshow/fourcc-codes
uint32_t VideoFrameEncodingFourCC(VideoFrameEncoding);
