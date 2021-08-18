/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <guiddef.h>
#include <dxva.h>

#include <VideoFrameEncoding.h>
#include <ColorSpace.h>
#include <EOTF.h>
#include <PixelValueRange.h>


const GUID TranslateToMediaSubType(VideoFrameEncoding);

DXVA_NominalRange TranslatePixelValueRange(PixelValueRange);

DXVA_VideoTransferMatrix TranslateVideoTransferMatrix(ColorSpace);

DXVA_VideoPrimaries TranslateVideoPrimaries(ColorSpace);

DXVA_VideoTransferFunction TranslateVideoTranferFunction(EOTF, ColorSpace);

DWORD TranslateVideoFrameEncodingToBiCompression(VideoFrameEncoding);
