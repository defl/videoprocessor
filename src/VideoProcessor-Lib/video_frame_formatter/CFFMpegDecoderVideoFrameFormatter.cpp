/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include <libavutil/error.h>

#include "CFFMpegDecoderVideoFrameFormatter.h"


static const int OUTPUT_LINESIZE_ALIGNMENT = 1;


CFFMpegDecoderVideoFrameFormatter::CFFMpegDecoderVideoFrameFormatter(
	AVCodecID inputCodecId,
	AVPixelFormat targetPixelFormat):
	m_targetPixelFormat(targetPixelFormat)
{
	// Check params

	if (inputCodecId == AV_CODEC_ID_NONE)
		throw std::runtime_error("Need valid AV_CODEC_ID_*");

	if (targetPixelFormat == AV_PIX_FMT_NONE)
		throw std::runtime_error("Need valid AV_PIX_FMT_*");

	if(!sws_isSupportedOutput(m_targetPixelFormat))
		throw std::runtime_error("Target pixel format not supported by swscale");

	// Build decoder and context

	const AVCodec* avCodecDecoder = avcodec_find_decoder(inputCodecId);
	if (!avCodecDecoder)
		throw std::runtime_error("Codec not found");

	m_avCodecContext = avcodec_alloc_context3(avCodecDecoder);
	if (!m_avCodecContext)
		throw std::runtime_error("Could not allocate video codec context");

	// This is a non-standard ffmpeg extension signalling no use of other threads
	m_avCodecContext->thread_count = -1;

	if (avcodec_open2(m_avCodecContext, avCodecDecoder, nullptr) < 0)
		throw std::runtime_error("Could not open codec");

	if(!sws_isSupportedInput(m_avCodecContext->pix_fmt))
		throw std::runtime_error("Source decoder pixel format not supported");

	// Alloc used buffers

	inputFrame = av_frame_alloc();
	if (!inputFrame)
		throw std::runtime_error("Failed to alloc input frame");

	outputFrame = av_frame_alloc();
	if (!outputFrame)
		throw std::runtime_error("Failed to alloc output frame");

	pkt = av_packet_alloc();
	if (!pkt)
		throw std::runtime_error("Failed to alloc pkt");
}


CFFMpegDecoderVideoFrameFormatter::~CFFMpegDecoderVideoFrameFormatter()
{
	Cleanup();

	if (m_avCodecContext)
	{
		avcodec_close(m_avCodecContext);
		avcodec_free_context(&m_avCodecContext);
	}

	if (pkt)
		av_packet_free(&pkt);

	if(inputFrame)
		av_frame_free(&inputFrame);

	if (outputFrame)
		av_frame_free(&outputFrame);
}


void CFFMpegDecoderVideoFrameFormatter::OnVideoState(VideoStateComPtr& videoState)
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

	// Update codec context with width and height
	m_avCodecContext->height = m_height;
	m_avCodecContext->width = m_width;

	// Build context
	m_sws = sws_getContext(
		m_width, m_height,
		m_avCodecContext->pix_fmt,
		m_width, m_height,
		m_targetPixelFormat,
		SWS_FAST_BILINEAR,
		nullptr,
		nullptr,
		nullptr);

	if (!m_sws)
		throw std::runtime_error("Failed to get context");

	if (!av_image_alloc(
		inputFrame->data, inputFrame->linesize,
		m_width, m_height,
		m_avCodecContext->pix_fmt,
		1 /* alignment */))
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


bool CFFMpegDecoderVideoFrameFormatter::FormatVideoFrame(
	const VideoFrame& inFrame,
	BYTE* outBuffer)
{
	if (m_width == 0 || m_height == 0 || m_inputBytesPerVideoFrame == 0)
		throw std::runtime_error("Width, height or bytes per frame not known, call OnVideoState() first");

	pkt->data = (uint8_t*)inFrame.GetData();
	pkt->size = m_inputBytesPerVideoFrame;

	// Decode
	int ret = avcodec_send_packet(m_avCodecContext, pkt);
	if (ret < 0)
		throw std::runtime_error("Failed to send packet for decoding");

	ret = avcodec_receive_frame(m_avCodecContext, inputFrame);
	if (ret == AVERROR(EAGAIN))
		return false;
	if (ret == AVERROR_EOF)
		throw std::runtime_error("Unexpected return for avcodec_receive_frame");
	if (ret < 0)
		throw std::runtime_error("avcodec_receive_frame errored");

	// Convert
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

	return true;
}


LONG CFFMpegDecoderVideoFrameFormatter::GetOutFrameSize() const
{
	assert(m_outFrameSize > 0);
	return m_outFrameSize;
}


void CFFMpegDecoderVideoFrameFormatter::Cleanup()
{
	if (m_outFrameSize > 0)
	{
		sws_freeContext(m_sws);

		av_freep(&inputFrame->data[0]);
		av_freep(&outputFrame->data[0]);
	}
}
