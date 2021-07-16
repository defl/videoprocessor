/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include "DirectShowGenericVideoRenderer.h"


/**
 * Enhanced Video Renderer
 * TODO: This does not work as is, it's incomplete, see https://docs.microsoft.com/en-us/windows/win32/medfound/how-to-write-an-evr-presenter
 */
class DirectShowEnhancedVideoRenderer:
	public DirectShowGenericVideoRenderer
{
public:

	DirectShowEnhancedVideoRenderer(
		IRendererCallback& callback,
		HWND videoHwnd,
		HWND eventHwnd,
		UINT eventMsg,
		ITimingClock* timingClock,
		DirectShowStartStopTimeMethod directShowStartStopTimeMethod,
		bool useFrameQueue,
		size_t frameQueueMaxSize,
		VideoConversionOverride videoConversionOverride);

	virtual ~DirectShowEnhancedVideoRenderer() {}
};
