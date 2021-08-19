/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <DeckLinkAPI_h.h>

#include <ColorFormat.h>
#include <BitDepth.h>
#include <VideoFrameEncoding.h>
#include <EOTF.h>
#include <ColorSpace.h>
#include <DisplayMode.h>


ColorFormat TranslateColorFormat(BMDDetectedVideoInputFormatFlags detectedVideoInputFormatFlagsValue);

BitDepth TranslateBithDepth(BMDDetectedVideoInputFormatFlags detectedVideoInputFormatFlagsValue);

VideoFrameEncoding Translate(BMDPixelFormat, ColorSpace);

EOTF TranslateEOTF(LONGLONG electroOpticalTransferFuncValue);

ColorSpace Translate(BMDColorspace, uint32_t verticalLines);

DisplayModeSharedPtr Translate(BMDDisplayMode);

double FPS(BMDDisplayMode displayMode);
