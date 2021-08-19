/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


extern "C"
{
	#include <libswscale/swscale.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/frame.h>
	#include <libavcodec/codec.h>
	#include <libavcodec/avcodec.h>
}

#include <video_frame_formatter/IVideoFrameFormatter.h>


 /**
  * This formatter can convert using an ffmpeg decoder and scaler for a target pixel format
  */
class CFFMpegDecoderVideoFrameFormatter:
	public IVideoFrameFormatter
{
public:

	CFFMpegDecoderVideoFrameFormatter(
		AVCodecID inputCodecId,
		AVPixelFormat targetPixelFormat);
	virtual ~CFFMpegDecoderVideoFrameFormatter();

	// IVideoFrameFormatter
	void OnVideoState(VideoStateComPtr& videoState) override;
	bool FormatVideoFrame(const VideoFrame& inFrame, BYTE* outBuffer) override;
	LONG GetOutFrameSize() const override;

private:

	const AVPixelFormat mTargetPixelFormat;
	const AVCodec* mAVCodecDecoder;
	AVCodecContext* mAVCodecContext;

	int mInputBytesPerVideoFrame = 0;
	int mHeight = 0;
	int mWidth = 0;
	LONG mOutFrameSize = 0;

	struct SwsContext* mSws = nullptr;
	AVFrame* mInputFrame = nullptr;
	AVFrame* mOutputFrame = nullptr;

	AVPacket* mPkt;

	void Cleanup();
};
