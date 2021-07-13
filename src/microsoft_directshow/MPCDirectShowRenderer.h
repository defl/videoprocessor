/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include "CVideoInfo1DirectShowRenderer.h"


/**
 * MPC Video Renderer
 * https://github.com/Aleksoid1978/VideoRenderer/
 */
class MPCDirectShowRenderer:
	public CVideoInfo1DirectShowRenderer
{
public:

	MPCDirectShowRenderer(
		GUID rendererCLSID,
		IRendererCallback& callback,
		HWND videoHwnd,
		HWND eventHwnd,
		UINT eventMsg,
		ITimingClock* timingClock,
		VideoStateComPtr& videoState,
		DirectShowStartStopTimeMethod timestamp,
		bool useFrameQueue,
		size_t frameQueueMaxSize);

	virtual ~MPCDirectShowRenderer() {}
};
