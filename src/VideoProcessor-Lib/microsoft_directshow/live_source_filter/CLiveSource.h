/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <streams.h>

#include <VideoState.h>
#include <video_frame_formatter/IVideoFrameFormatter.h>
#include <microsoft_directshow/DirectShowRendererStartStopTimeMethod.h>

#include "ILiveSource.h"


class ALiveSourceVideoOutputPin;


#define LIVE_SOURCE_FILTER_NAME TEXT("LiveSourceFilter")
#define LIVE_SOURCE_FILTER_VIDEO_OUPUT_PIN_NAME TEXT("Video")


/**
 * This is a source filter with a single video output pin which you can use to push
 * frames yourself through the ILiveSource interface.
 */
class CLiveSource:
	public CBaseFilter,
	public ILiveSource,
	public IAMFilterMiscFlags
{
public:

	CLiveSource(LPUNKNOWN pUnk,	HRESULT* phr);
	virtual ~CLiveSource();

	static CUnknown* WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT* phr);

	// IUnknown
	DECLARE_IUNKNOWN;

	// ILiveSource
	STDMETHODIMP Initialize(
		IVideoFrameFormatter* videoFrameFormatter,
		const AM_MEDIA_TYPE& mediaType,
		timestamp_t frameDuration,
		ITimingClock* timingClock,
		DirectShowStartStopTimeMethod timestamp,
		bool useFrameQueue,
		size_t frameQueueMaxSize) override;
	STDMETHODIMP Destroy() override;
	STDMETHODIMP OnHDRData(HDRDataSharedPtr&) override;
	STDMETHODIMP OnVideoFrame(VideoFrame&) override;
	STDMETHODIMP SetFrameQueueMaxSize(size_t) override;
	STDMETHODIMP Reset() override;

	// CBaseFilter
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) override;
	int GetPinCount() override;
	CBasePin* GetPin(int n) override;

	// Get the video output pin
	// Can only be called after Initialize()
	ALiveSourceVideoOutputPin* GetVideoOutputPin() const { return m_videoOutputPin; }

	// IAMFilterMiscFlags
	ULONG STDMETHODCALLTYPE GetMiscFlags(void) override;

	//
	// Queue
	//

	// Get the current frame queue size, negative means no queue
	// Can only be called after Initialize()
	int GetFrameQueueSize();

	//
	// Metrics
	//

	// Get the exit latency in ms, which the amount of time between the frame timestamp
	// and when the frame is delivered to the DirectShow renderer.
	// This is sampled.
	double ExitLatencyMs() const;


	// Get the amount of dropped frames due to queue actions
	uint64_t DroppedFrameCount() const;

private:
	ALiveSourceVideoOutputPin* m_videoOutputPin = nullptr;

	CCritSec m_critSec;
};
