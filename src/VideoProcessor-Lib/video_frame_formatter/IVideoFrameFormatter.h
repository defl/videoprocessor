/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <VideoFrame.h>
#include <VideoState.h>


/**
 * Interface for a video frame formatter, this can render from one
 * format to another.
 */
class IVideoFrameFormatter
{
public:

	virtual ~IVideoFrameFormatter() {}

	// New video state, must be called before FormatVideoFrame()
	virtual void OnVideoState(VideoStateComPtr& videoState) = 0;

	// Handle video frame
	// Can only be called after OnVideoState()
	// Returns true if something was converted, false if not
	virtual bool FormatVideoFrame(const VideoFrame& inFrame, BYTE* outBuffer) = 0;

	// Get size of frame that will be put in FormatVideoFrame()'s outBuffer, in bytes
	// Can only be called after OnVideoState()
	virtual LONG GetOutFrameSize() const = 0;
};
