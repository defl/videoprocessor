/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include "CV210toP210VideoFrameFormatter.h"

//
// Parts of this are copied from ffmpeg v210dec.c, see /3rdparty/ffmpeg/README.txt for license and attribution
//


#define V210_READ_PACK_BLOCK(a, b, c) \
    do {                              \
        val  = *src++;                \
        a = val & 0x3FF;              \
        b = (val >> 10) & 0x3FF;      \
        c = (val >> 20) & 0x3FF;      \
    } while (0)


#define P010_WRITE_VALUE(d, v) (*d++ = (v << 6))


#define PIXELS_PER_PACK 6
#define BYTES_PER_PACK (4 * sizeof(uint32_t))


void CV210toP210VideoFrameFormatter::OnVideoState(VideoStateComPtr& videoState)
{
	if (!videoState)
		throw std::runtime_error("Null video state is not allowed");

    if (videoState->videoFrameEncoding != VideoFrameEncoding::V210)
        throw std::runtime_error("Can only handle V210 input");

    m_height = videoState->displayMode->FrameHeight();
    if (m_height % 2 != 0)
        throw std::runtime_error("P010 output needs an even amount of input lines");

    m_width = videoState->displayMode->FrameWidth();
	if (m_width % 6 != 0)
		throw std::runtime_error("Can only handle conversions which align with V210 boundry (6 pixels)");

    const uint32_t bytes = videoState->BytesPerFrame();
    const uint32_t expectedBytes =
        videoState->displayMode->FrameHeight() *
        (videoState->displayMode->FrameWidth() / PIXELS_PER_PACK * BYTES_PER_PACK);

    if(bytes != expectedBytes)
        throw std::runtime_error("Unexpected amount of bytes for frame");
}


bool CV210toP210VideoFrameFormatter::FormatVideoFrame(
	const VideoFrame& inFrame,
	BYTE* outBuffer)
{
	// Read V210
	// https://wiki.multimedia.cx/index.php/V210

	// Write P210
    // 10bpp per component, data in the high bits, zeros in the low bits (we assume little-endian native)
	// https://docs.microsoft.com/en-us/windows/win32/medfound/10-bit-and-16-bit-yuv-video-formats

    const uint32_t pixels = m_height * m_width;
    const uint32_t aligned_width = ((m_width + 47) / 48) * 48;
    const uint32_t stride = aligned_width * 8 / 3;

    uint16_t* dstY = (uint16_t *)outBuffer;
    uint16_t* dstUV = (uint16_t*)(outBuffer + ((ptrdiff_t)pixels * sizeof(uint16_t)));

    const uint32_t packsPerLine = m_width / PIXELS_PER_PACK;

    for (uint32_t line = 0; line < m_height; line++)
    {
        const uint32_t* src = (const uint32_t*)((const BYTE *)inFrame.GetData() + (ptrdiff_t)(line * stride));  // Lines start at 128 byte alignment

        for (uint32_t pack = 0; pack < packsPerLine; pack++)
        {
            uint32_t val;
            uint16_t u, y1, y2, v;

            V210_READ_PACK_BLOCK(u, y1, v);
            P010_WRITE_VALUE(dstUV, u);
            P010_WRITE_VALUE(dstY, y1);
            P010_WRITE_VALUE(dstUV, v);

            V210_READ_PACK_BLOCK(y1, u, y2);
            P010_WRITE_VALUE(dstY, y1);
            P010_WRITE_VALUE(dstUV, u);
            P010_WRITE_VALUE(dstY, y2);

            V210_READ_PACK_BLOCK(v, y1, u);
            P010_WRITE_VALUE(dstUV, v);
            P010_WRITE_VALUE(dstY, y1);
            P010_WRITE_VALUE(dstUV, u);

            V210_READ_PACK_BLOCK(y1, v, y2);
            P010_WRITE_VALUE(dstY, y1);
            P010_WRITE_VALUE(dstUV, v);
            P010_WRITE_VALUE(dstY, y2);
        }
    }

	return true;
}


LONG CV210toP210VideoFrameFormatter::GetOutFrameSize() const
{
    const LONG pixels = m_height * m_width;

    return
        (pixels * sizeof(uint16_t)) +  // Every pixel 1 y
        (pixels / 2 * (2 * sizeof(uint16_t)));  // Every 2 pixels 2 16-bit numbers
}
