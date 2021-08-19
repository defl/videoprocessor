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
	mTargetPixelFormat(targetPixelFormat)
{
	// Check params

	if (inputCodecId == AV_CODEC_ID_NONE)
		throw std::runtime_error("Need valid AV_CODEC_ID_*");

	if (targetPixelFormat == AV_PIX_FMT_NONE)
		throw std::runtime_error("Need valid AV_PIX_FMT_*");

	if(!sws_isSupportedOutput(mTargetPixelFormat))
		throw std::runtime_error("Target pixel format not supported by swscale");

	// Build decoder and context

	const AVCodec* avCodecDecoder = avcodec_find_decoder(inputCodecId);
	if (!avCodecDecoder)
		throw std::runtime_error("Codec not found");

	mAVCodecContext = avcodec_alloc_context3(avCodecDecoder);
	if (!mAVCodecContext)
		throw std::runtime_error("Could not allocate video codec context");

	// This is a non-standard ffmpeg extension signalling no use of other threads
	mAVCodecContext->thread_count = -1;

	if (avcodec_open2(mAVCodecContext, avCodecDecoder, nullptr) < 0)
		throw std::runtime_error("Could not open codec");

	if(!sws_isSupportedInput(mAVCodecContext->pix_fmt))
		throw std::runtime_error("Source decoder pixel format not supported");

	// Alloc used buffers

	mInputFrame = av_frame_alloc();
	if (!mInputFrame)
		throw std::runtime_error("Failed to alloc input frame");

	mOutputFrame = av_frame_alloc();
	if (!mOutputFrame)
		throw std::runtime_error("Failed to alloc output frame");

	mPkt = av_packet_alloc();
	if (!mPkt)
		throw std::runtime_error("Failed to alloc mPkt");
}


CFFMpegDecoderVideoFrameFormatter::~CFFMpegDecoderVideoFrameFormatter()
{
	Cleanup();

	if (mAVCodecContext)
	{
		avcodec_close(mAVCodecContext);
		avcodec_free_context(&mAVCodecContext);
	}

	if(mInputFrame)
		av_frame_free(&mInputFrame);

	if (mOutputFrame)
		av_frame_free(&mOutputFrame);

	if (mPkt)
		av_packet_free(&mPkt);
}


void CFFMpegDecoderVideoFrameFormatter::OnVideoState(VideoStateComPtr& videoState)
{
	if (!videoState)
		throw std::runtime_error("Null video state is not allowed");

	Cleanup();

	mInputBytesPerVideoFrame = videoState->BytesPerFrame();
	assert(mInputBytesPerVideoFrame > 0);

	mHeight = videoState->displayMode->FrameHeight();
	assert(mHeight > 0);

	mWidth = videoState->displayMode->FrameWidth();
	assert(mWidth > 0);

	// Update codec context with width and height
	mAVCodecContext->height = mHeight;
	mAVCodecContext->width = mWidth;

	// Build context
	mSws = sws_getContext(
		mWidth, mHeight,
		mAVCodecContext->pix_fmt,
		mWidth, mHeight,
		mTargetPixelFormat,
		SWS_FAST_BILINEAR,
		nullptr,
		nullptr,
		nullptr);

	if (!mSws)
		throw std::runtime_error("Failed to get context");

	if(!av_image_alloc(
		mOutputFrame->data, mOutputFrame->linesize,
		mWidth, mHeight,
		mTargetPixelFormat,
		OUTPUT_LINESIZE_ALIGNMENT))
		throw std::runtime_error("Failed to allocate mOutputFrame image");

	mOutFrameSize = av_image_get_buffer_size(
		mTargetPixelFormat,
		mWidth, mHeight,
		OUTPUT_LINESIZE_ALIGNMENT);

	if(mOutFrameSize <= 0)
		throw std::runtime_error("Failed to get output frame size");
}


bool CFFMpegDecoderVideoFrameFormatter::FormatVideoFrame(
	const VideoFrame& inFrame,
	BYTE* outBuffer)
{
	assert(mOutFrameSize > 0);  // Means it's set up

	if (mWidth == 0 || mHeight == 0 || mInputBytesPerVideoFrame == 0)
		throw std::runtime_error("Width, height or bytes per frame not known, call OnVideoState() first");

	mPkt->data = (uint8_t*)inFrame.GetData();
	mPkt->size = mInputBytesPerVideoFrame;

	// Decode
	int ret = avcodec_send_packet(mAVCodecContext, mPkt);
	if (ret < 0)
		throw std::runtime_error("Failed to send packet for decoding");

	ret = avcodec_receive_frame(mAVCodecContext, mInputFrame);
	if (ret == AVERROR(EAGAIN))
		return false;
	if (ret == AVERROR_EOF)
		throw std::runtime_error("Unexpected return for avcodec_receive_frame");
	if (ret < 0)
		throw std::runtime_error("avcodec_receive_frame errored");

	// Convert
	int scaled_lines = sws_scale(
		mSws,
		mInputFrame->data, mInputFrame->linesize,
		0, mHeight,
		mOutputFrame->data, mOutputFrame->linesize);
	if (scaled_lines != mHeight)
		throw std::runtime_error("Failed to sws_scale all lines");

	int copiedSize = av_image_copy_to_buffer(
		(uint8_t*)outBuffer,
		mOutFrameSize,
		mOutputFrame->data, mOutputFrame->linesize,
		mTargetPixelFormat,
		mWidth, mHeight,
		OUTPUT_LINESIZE_ALIGNMENT);

	if (copiedSize != mOutFrameSize)
		throw std::runtime_error("Failed to av_image_copy_to_buffer");

	return true;
}


LONG CFFMpegDecoderVideoFrameFormatter::GetOutFrameSize() const
{
	assert(mOutFrameSize > 0);
	return mOutFrameSize;
}


void CFFMpegDecoderVideoFrameFormatter::Cleanup()
{
	if (mOutFrameSize > 0)
	{
		sws_freeContext(mSws);

		av_freep(&mOutputFrame->data[0]);
	}
}
