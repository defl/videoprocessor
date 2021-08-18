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
 * This is used by renderers to make specific choices
 */
enum class VideoConversionOverride
{
	// No override, let renderer decide
	VIDEOCONVERSION_NONE,

	// If the video is v210 (YUV422) convert it to p010 (YUV420)
	VIDEOCONVERSION_V210_TO_P010
};


const TCHAR* ToString(const VideoConversionOverride);
