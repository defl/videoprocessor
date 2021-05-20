/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */


#include <stdafx.h>

#include <guid.h>

#include "DirectShowTranslations.h"


const GUID TranslateToMediaSubType(PixelFormat pixelFormat)
{
	switch (pixelFormat)
	{
	case PixelFormat::YUV_8BIT:
		// TODO: Can also be HDYC if color space is rec709, see https://www.fourcc.org/yuv.php
		return MEDIASUBTYPE_UYVY;

	case PixelFormat::YUV_10BIT:
		return MEDIASUBTYPE_v210;

	case PixelFormat::RGB_10BIT:
		return MEDIASUBTYPE_r210;

	case PixelFormat::RGB_BE_12BIT:
		break;
	}

	throw std::runtime_error("Cannot translate pixelformat");
}
