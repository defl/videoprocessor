/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include "CNoopVideoFrameFormatter.h"


void CNoopVideoFrameFormatter::OnVideoState(VideoStateComPtr& videoState)
{
	if (!videoState)
		throw std::runtime_error("Null video state is not allowed");

	m_bytesPerVideoFrame = videoState->BytesPerFrame();
	assert(m_bytesPerVideoFrame > 0);
}


bool CNoopVideoFrameFormatter::FormatVideoFrame(
	const VideoFrame& inFrame,
	BYTE* outBuffer)
{
	if (m_bytesPerVideoFrame == 0)
		throw std::runtime_error("bytes per frame not known, call OnVideoState() first");

	memcpy(outBuffer, inFrame.GetData(), m_bytesPerVideoFrame);
	return true;
}


LONG CNoopVideoFrameFormatter::GetOutFrameSize() const
{
	assert(m_bytesPerVideoFrame > 0);
	return m_bytesPerVideoFrame;
}
