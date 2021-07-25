/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <video_frame_formatter/IVideoFrameFormatter.h>


 /**
  * Video frame formatter which reads V210 and write to P010
  * (that's YUV422 to YUV420 both in 10 bit, all assuming this is running on little endian hardware)
  */
class CV210toP010VideoFrameFormatter:
	public IVideoFrameFormatter
{
public:

	virtual ~CV210toP010VideoFrameFormatter() {}

	// IVideoFrameFormatter
	void OnVideoState(VideoStateComPtr& videoState) override;
	bool FormatVideoFrame(const VideoFrame& inFrame, BYTE* outBuffer) override;
	LONG GetOutFrameSize() const override;

private:
	uint32_t m_height = 0;
	uint32_t m_width = 0;
};
