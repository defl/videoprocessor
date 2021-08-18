/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */


#include <pch.h>

#include <guid.h>
#include <microsoft_directshow/DirectShowDefines.h>

#include "DirectShowTranslations.h"


const GUID TranslateToMediaSubType(VideoFrameEncoding videoFrameEncoding)
{
	switch (videoFrameEncoding)
	{
	case VideoFrameEncoding::UYVY:
		return MEDIASUBTYPE_UYVY;

	case VideoFrameEncoding::HDYC:
		return MEDIASUBTYPE_HDYC;

	case VideoFrameEncoding::V210:
		return MEDIASUBTYPE_v210;

	case VideoFrameEncoding::R210:
		return MEDIASUBTYPE_r210;

	case VideoFrameEncoding::R10b:
		return MEDIASUBTYPE_R10b;

	case VideoFrameEncoding::R12B:
		return MEDIASUBTYPE_R12B;
	}

	throw std::runtime_error("TranslateToMediaSubType cannot translate");
}


DXVA_NominalRange TranslatePixelValueRange(PixelValueRange pixelValueRange)
{
	switch (pixelValueRange)
	{
	case PixelValueRange::PIXELVALUERANGE_UNKNOWN:
		return DXVA_NominalRange::DXVA_NominalRange_Unknown;

	case PixelValueRange::PIXELVALUERANGE_0_255:
		return DXVA_NominalRange::DXVA_NominalRange_0_255;

	case PixelValueRange::PIXELVALUERANGE_16_235:
		return DXVA_NominalRange::DXVA_NominalRange_16_235;
	}

	throw std::runtime_error("TranslatePixelValueRange cannot translate");
}


DXVA_VideoTransferMatrix TranslateVideoTransferMatrix(ColorSpace colorSpace)
{
	switch (colorSpace)
	{
	case ColorSpace::REC_601_525:
		return DXVA_VideoTransferMatrix_BT601;

	case ColorSpace::REC_601_625:
		return DXVA_VideoTransferMatrix_BT601;

	case ColorSpace::REC_709:
		return DXVA_VideoTransferMatrix_BT709;

	case ColorSpace::BT_2020:
		return DIRECTSHOW_VIDEOTRANSFERMATRIX_BT2020_10;
	}

	throw std::runtime_error("TranslateVideoTransferMatrix cannot translate");
}


DXVA_VideoPrimaries TranslateVideoPrimaries(ColorSpace colorSpace)
{
	switch (colorSpace)
	{
	case ColorSpace::REC_601_525:
		return DXVA_VideoPrimaries_BT470_2_SysM;  // TODO: There is also a SysBG

	case ColorSpace::REC_709:
		return DXVA_VideoPrimaries_BT709;

	case ColorSpace::BT_2020:
		return DIRECTSHOW_VIDEOPRIMARIES_BT2020;
	}

	throw std::runtime_error("TranslateVideoTransferMatrix cannot translate");
}


DXVA_VideoTransferFunction TranslateVideoTranferFunction(EOTF eotf, ColorSpace colorSpace)
{
	switch (eotf)
	{
	case EOTF::SDR:
		if (colorSpace == ColorSpace::REC_709)
			return DXVA_VideoTransFunc_22_709;
		else
			throw std::runtime_error("Don't know video transfer function for SDR outside of REC 709");
		break;

	case EOTF::PQ:
		return DIRECTSHOW_VIDEOTRANSFUNC_2084;
		break;
	}

	throw std::runtime_error("TranslateVideoTranferFunction cannot translate");
}


DWORD TranslatePixelformatToBiCompression(VideoFrameEncoding videoFrameEncoding)
{
	switch (videoFrameEncoding)
	{
	case VideoFrameEncoding::ARGB_8BIT:
	case VideoFrameEncoding::BGRA_8BIT:
	case VideoFrameEncoding::R210:
	case VideoFrameEncoding::R10b:
	case VideoFrameEncoding::R10l:
	case VideoFrameEncoding::R12B:
	case VideoFrameEncoding::R12L:
		return BI_RGB;
	}

	return VideoFrameEncodingFourCC(videoFrameEncoding);
}
