/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include "VideoFrame.h"


VideoFrame::VideoFrame(
	const void* data, uint64_t counter,
	timingclocktime_t timingTimestamp, IUnknown* sourceBuffer):
	m_data(data),
	m_counter(counter),
	m_timingTimestamp(timingTimestamp),
	m_sourceBuffer(sourceBuffer)
{
	assert(data);
}

VideoFrame::VideoFrame(const VideoFrame& videoFrame) :
	m_data(videoFrame.m_data),
	m_counter(videoFrame.m_counter),
	m_timingTimestamp(videoFrame.m_timingTimestamp),
	m_sourceBuffer(videoFrame.m_sourceBuffer)
{
}


VideoFrame::~VideoFrame()
{
}


void VideoFrame::SourceBufferAddRef()
{
	m_sourceBuffer->AddRef();
}


void VideoFrame::SourceBufferRelease()
{
	const ULONG refCount = m_sourceBuffer->Release();
	assert(refCount == 0);
}


VideoFrame& VideoFrame::operator= (const VideoFrame& videoFrame)
{
	assert(this != &videoFrame);

	m_data = videoFrame.m_data;
	m_counter = videoFrame.m_counter;
	m_timingTimestamp = videoFrame.m_timingTimestamp;
	m_sourceBuffer = videoFrame.m_sourceBuffer;

	return *this;
}
