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
#include <IVideoFrameFormatter.h>

#include "ILiveSource.h"


class CLiveSourceVideoOutputPin;


#define LIVE_SOURCE_FILTER_NAME TEXT("LiveSourceFilter")
#define LIVE_SOURCE_FILTER_VIDEO_OUPUT_PIN_NAME TEXT("Video")


/**
 * This is a source filter with a single output pin which you can use to push
 * frames yourself through the ILiveSource interface.
 *
 * It's based on "DirectShow Filters Development Part2: Live Source filter" by Roman Ginzburg
 * https://www.codeproject.com/Articles/158053/DirectShow-Filters-Development-Part-2-Live-Source
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
	STDMETHODIMP Setup(
		IVideoFrameFormatter* videoFrameFormatter,
		GUID mediaSubType,
		timestamp_t frameDuration,
		timingclocktime_t timestampTicksPerSecond) override;
	STDMETHODIMP OnHDRData(HDRDataSharedPtr&) override;
	STDMETHODIMP OnVideoFrame(VideoFrame&) override;

	// CBaseFilter
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) override;
	int GetPinCount() override;
	CBasePin* GetPin(int n) override;

	// IAMFilterMiscFlags
	ULONG STDMETHODCALLTYPE GetMiscFlags(void) override;

	// Get the MediaSubType the data will represent
	GUID GetMediaSubType();

private:
	CLiveSourceVideoOutputPin* m_videoOutputPin;

	GUID m_mediaSubType;

	CCritSec m_critSec;
};
