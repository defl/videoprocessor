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

#include <IVideoFrameFormatter.h>


 /**
  * This formatter can conver using an ffmpeg decoder and scaler for a target pixel format
  */
class CFFMpegDecoderVideoFrameFormatter:
	public IVideoFrameFormatter
{
public:

	CFFMpegDecoderVideoFrameFormatter(
		AVCodecID inputCodecId,
		AVPixelFormat targetPixelFormat);
	~CFFMpegDecoderVideoFrameFormatter();

	// IVideoFrameFormatter
	void OnVideoState(VideoStateComPtr& videoState) override;
	void FormatVideoFrame(const VideoFrame& inFrame, BYTE* outBuffer) override;
	LONG GetOutFrameSize() const override;

private:

	const AVPixelFormat m_targetPixelFormat;
	const AVCodec* m_avCodecDecoder;
	AVCodecContext* m_avCodecContext;

	int m_inputBytesPerVideoFrame = 0;
	int m_height = 0;
	int m_width = 0;
	LONG m_outFrameSize = 0;

	struct SwsContext* m_sws = nullptr;
	AVFrame* inputFrame = nullptr;
	AVFrame* outputFrame = nullptr;

	AVPacket* pkt;

	void Cleanup();
};