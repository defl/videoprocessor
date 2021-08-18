/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <atomic>

#include <EOTF.h>
#include <ColorSpace.h>
#include <DisplayMode.h>
#include <VideoFrameEncoding.h>
#include <HDRData.h>


/**
 * This describes the video stream
 */
class VideoState:
	public IUnknown
{
public:
	VideoState() {}
	VideoState(const VideoState&);

	// True if valid, else do not interpret other fields
	bool valid = false;

	DisplayModeSharedPtr displayMode = nullptr;
	VideoFrameEncoding videoFrameEncoding = VideoFrameEncoding::UNKNOWN;
	EOTF eotf = EOTF::UNKNOWN;
	ColorSpace colorspace = ColorSpace::UNKNOWN;
	bool invertedVertical = false;

	// Will be non-null if valid
	HDRDataSharedPtr hdrData = nullptr;

	// IUnknown
	HRESULT	QueryInterface(REFIID iid, LPVOID* ppv) override;
	ULONG AddRef() override;
	ULONG Release() override;

	// Return the the amount of bytes needed to store a row/line of pixels in this format
	uint32_t BytesPerRow() const;

	// Return the the amount of bytes needed to store a full frame of pixels in this format
	uint32_t BytesPerFrame() const;

private:

	std::atomic<ULONG> m_refCount;
};


typedef CComPtr<VideoState> VideoStateComPtr;
