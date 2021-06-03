/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "CFFMpegScalerVideoFrameFormatter.h"


static const int OUTPUT_LINESIZE_ALIGNMENT = 1;


CFFMpegScalerVideoFrameFormatter::CFFMpegScalerVideoFrameFormatter(
	AVPixelFormat sourcePixelFormat,
	AVPixelFormat targetPixelFormat):
	m_sourcePixelFormat(sourcePixelFormat),
	m_targetPixelFormat(targetPixelFormat)
{
	// Check params

	if (sourcePixelFormat == AV_PIX_FMT_NONE)
		throw std::runtime_error("Need valid source AV_PIX_FMT_*");

	if (targetPixelFormat == AV_PIX_FMT_NONE)
		throw std::runtime_error("Need valid target AV_PIX_FMT_*");

	if (!sws_isSupportedInput(sourcePixelFormat))
		throw std::runtime_error("Source pixel format not supported by swscale");

	if(!sws_isSupportedOutput(m_targetPixelFormat))
		throw std::runtime_error("Target pixel format not supported by swscale");

	// Alloc used buffers

	inputFrame = av_frame_alloc();
	if (!inputFrame)
		throw std::runtime_error("Failed to alloc input frame");

	outputFrame = av_frame_alloc();
	if (!outputFrame)
		throw std::runtime_error("Failed to alloc output frame");
}


CFFMpegScalerVideoFrameFormatter::~CFFMpegScalerVideoFrameFormatter()
{
	Cleanup();

	if (inputFrame)
		av_frame_free(&inputFrame);

	if (outputFrame)
		av_frame_free(&outputFrame);
}


void CFFMpegScalerVideoFrameFormatter::OnVideoState(VideoStateComPtr& videoState)
{
	if (!videoState)
		throw std::runtime_error("Null video state is not allowed");

	Cleanup();

	m_inputBytesPerVideoFrame = videoState->BytesPerFrame();
	assert(m_inputBytesPerVideoFrame > 0);

	m_height = videoState->displayMode->FrameHeight();
	assert(m_height > 0);

	m_width = videoState->displayMode->FrameWidth();
	assert(m_width > 0);

	// Build context
	m_sws = sws_getContext(
		m_width, m_height,
		m_sourcePixelFormat,
		m_width, m_height,
		m_targetPixelFormat,
		SWS_FAST_BILINEAR,
		NULL,
		NULL,
		NULL);

	if (!m_sws)
		throw std::runtime_error("Failed to get context");

	if (!av_image_alloc(
		inputFrame->data, inputFrame->linesize,
		m_width, m_height,
		m_sourcePixelFormat,
		OUTPUT_LINESIZE_ALIGNMENT))
		throw std::runtime_error("Failed to allocate inputFrame image");

	if(!av_image_alloc(
		outputFrame->data, outputFrame->linesize,
		m_width, m_height,
		m_targetPixelFormat,
		OUTPUT_LINESIZE_ALIGNMENT))
		throw std::runtime_error("Failed to allocate outputFrame image");

	m_outFrameSize = av_image_get_buffer_size(
		m_targetPixelFormat,
		m_width, m_height,
		OUTPUT_LINESIZE_ALIGNMENT);

	if(m_outFrameSize <= 0)
		throw std::runtime_error("Failed to get output frame size");
}


void CFFMpegScalerVideoFrameFormatter::FormatVideoFrame(
	const VideoFrame& inFrame,
	BYTE* outBuffer)
{
	if (m_width == 0 || m_height == 0 || m_inputBytesPerVideoFrame == 0)
		throw std::runtime_error("Width, height or bytes per frame not known, call OnVideoState() first");

	int readSize = av_image_fill_arrays(
		inputFrame->data, inputFrame->linesize,
		(uint8_t*)inFrame.GetData(),
		m_sourcePixelFormat,
		m_width, m_height,
		OUTPUT_LINESIZE_ALIGNMENT);

	if (readSize != m_inputBytesPerVideoFrame)
		throw std::runtime_error("Failed to av_image_fill_arrays");

	int scaled_lines = sws_scale(
		m_sws,
		inputFrame->data, inputFrame->linesize,
		0, m_height,
		outputFrame->data, outputFrame->linesize);

	if (scaled_lines != m_height)
		throw std::runtime_error("Failed to sws_scale all lines");

	int copiedSize = av_image_copy_to_buffer(
		(uint8_t*)outBuffer,
		m_outFrameSize,
		outputFrame->data, outputFrame->linesize,
		m_targetPixelFormat,
		m_width, m_height,
		OUTPUT_LINESIZE_ALIGNMENT);

	if (copiedSize != m_outFrameSize)
		throw std::runtime_error("Failed to av_image_copy_to_buffer");
}


LONG CFFMpegScalerVideoFrameFormatter::GetOutFrameSize() const
{
	assert(m_outFrameSize > 0);
	return m_outFrameSize;
}


void CFFMpegScalerVideoFrameFormatter::Cleanup()
{
	if (m_sws)
		sws_freeContext(m_sws);

	if (inputFrame)
		av_freep(&inputFrame[0]);

	if (outputFrame)
		av_freep(&outputFrame[0]);
}
