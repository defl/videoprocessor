/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include "VideoConversionOverride.h"


const TCHAR* ToString(const VideoConversionOverride videoConversionOverride)
{
	switch (videoConversionOverride)
	{
	case VideoConversionOverride::VIDEOCONVERSION_NONE:
		return TEXT("No override");

	case VideoConversionOverride::VIDEOCONVERSION_V210_TO_P010:
		return TEXT("V210 > P010");
	}

	throw std::runtime_error("VideoConversionOverride ToString() failed, value not recognized");
}
