/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include <guid.h>
#include <IMediaSideData.h>

#include "ALiveSourceVideoOutputPin.h"


const static REFERENCE_TIME REFERENCE_TIME_INVALID = -1;


ALiveSourceVideoOutputPin::ALiveSourceVideoOutputPin(
	CLiveSource* filter,
	CCritSec* pLock,
	HRESULT* phr):
	CBaseOutputPin(
		LIVE_SOURCE_FILTER_NAME, filter, pLock, phr,
		LIVE_SOURCE_FILTER_VIDEO_OUPUT_PIN_NAME)
{
}


ALiveSourceVideoOutputPin::~ALiveSourceVideoOutputPin()
{
}


void ALiveSourceVideoOutputPin::Initialize(
	IVideoFrameFormatter* const videoFrameFormatter,
	timestamp_t frameDuration,
	ITimingClock* const timingClock,
	RendererTimestamp timestamp,
	int frameClockOffsetMs,
	GUID mediaSubType)
{
	if (!videoFrameFormatter)
		throw std::runtime_error("Cannot set null IVideoFrameFormatter");

	if (frameDuration <= 0)
		throw std::runtime_error("Duration must be > 0");
	assert(frameDuration > 50000LL); // 5ms frame is 200Hz, probably a reasonable upper bound
	assert(frameDuration < 10000000LL);  // 1Hz, reasonable lower bound

	assert(m_frameClockOffsetMs > -20000);  // 20 sec
	assert(m_frameClockOffsetMs <  20000);

	m_videoFrameFormatter = videoFrameFormatter;
	m_frameDuration = frameDuration;
	m_timingClock = timingClock;
	m_timestamp = timestamp;
	m_frameClockOffsetMs = frameClockOffsetMs;
	m_mediaSubType = mediaSubType;
}


HRESULT ALiveSourceVideoOutputPin::GetMediaType(int iPosition, CMediaType* pmt)
{
	throw std::runtime_error("GetMediaType() is not supported");
}


HRESULT ALiveSourceVideoOutputPin::CheckMediaType(const CMediaType* pmt)
{
	CheckPointer(pmt, E_POINTER);

	if(pmt->majortype != MEDIATYPE_Video)
	{
		return E_INVALIDARG;
	}

	if (!IsEqualGUID(pmt->subtype, static_cast<CLiveSource*>(m_pFilter)->GetMediaSubType()))
	{
		return E_INVALIDARG;
	}

	if (pmt->formattype != FORMAT_VideoInfo2)
	{
		return E_INVALIDARG;
	}

	return S_OK;
}


HRESULT ALiveSourceVideoOutputPin::DecideAllocator(IMemInputPin* pPin, IMemAllocator** ppAlloc)
{
	HRESULT hr = NOERROR;
	*ppAlloc = nullptr;

	// get downstream prop request
	// the derived class may modify this in DecideBufferSize, but
	// we assume that he will consistently modify it the same way,
	// so we only get it once
	ALLOCATOR_PROPERTIES prop;
	ZeroMemory(&prop, sizeof(prop));

	pPin->GetAllocatorRequirements(&prop);

	if (prop.cbAlign == 0)
		prop.cbAlign = 1;

	// We only try the allocator of the input pin, we don't have a suitable allocator
	// for IMediaSideData.
	hr = pPin->GetAllocator(ppAlloc);
	if (SUCCEEDED(hr))
	{
		hr = DecideBufferSize(*ppAlloc, &prop);
		if (SUCCEEDED(hr))
		{
			hr = pPin->NotifyAllocator(*ppAlloc, FALSE);
			if (SUCCEEDED(hr))
			{
				return NOERROR;
			}
		}
	}

	// If the GetAllocator failed we may not have an interface
	if (*ppAlloc)
	{
		(*ppAlloc)->Release();
		*ppAlloc = nullptr;
	}
	return hr;
}


HRESULT ALiveSourceVideoOutputPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *ppropInputRequest)
{
	CheckPointer(pAlloc,E_POINTER);
	CheckPointer(ppropInputRequest,E_POINTER);

	// TODO: Lock this? In the exmaples there things were locked
	//CAutoLock cAutoLock(m_pLock);
	HRESULT hr = NOERROR;

	ppropInputRequest->cBuffers = 1;
	ppropInputRequest->cbBuffer = m_videoFrameFormatter->GetOutFrameSize();

	ASSERT(ppropInputRequest->cbBuffer);

	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(ppropInputRequest,&Actual);
	if(FAILED(hr))
	{
		return hr;
	}

	if(Actual.cbBuffer < ppropInputRequest->cbBuffer)
	{
		return E_FAIL;
	}

	return S_OK;
}


//
// IAMPushSource
//


STDMETHODIMP ALiveSourceVideoOutputPin::GetMaxStreamOffset(REFERENCE_TIME *prtMaxOffset)
{
	*prtMaxOffset = 0;
	return S_OK;
}


STDMETHODIMP ALiveSourceVideoOutputPin::GetPushSourceFlags(ULONG *pFlags)
{
	// Return (* pFlags) = 0 [if this is a iAMPushSource]
	// https://docs.microsoft.com/en-us/windows/win32/api/strmif/nn-strmif-ireferenceclock
	*pFlags = 0;
	return S_OK;
}


STDMETHODIMP ALiveSourceVideoOutputPin::GetStreamOffset(REFERENCE_TIME *prtOffset)
{
	*prtOffset = 0;
	return S_OK;
}


STDMETHODIMP ALiveSourceVideoOutputPin::SetMaxStreamOffset(REFERENCE_TIME rtMaxOffset)
{
	return E_NOTIMPL;
}


STDMETHODIMP ALiveSourceVideoOutputPin::SetPushSourceFlags(ULONG Flags)
{
	return E_NOTIMPL;
}


STDMETHODIMP ALiveSourceVideoOutputPin::SetStreamOffset(REFERENCE_TIME rtOffset)
{
	return E_NOTIMPL;
}


STDMETHODIMP ALiveSourceVideoOutputPin::GetLatency(REFERENCE_TIME *prtLatency)
{
	// latency in 100-nanosecond units.
	*prtLatency = 0;  // TODO: Just a guess
	return S_OK;
}


//
// IQualityControl
//


STDMETHODIMP ALiveSourceVideoOutputPin::Notify(IBaseFilter* pSender, Quality q)
{
	// TODO
	return S_OK;
}


HRESULT STDMETHODCALLTYPE ALiveSourceVideoOutputPin::SetSink(IQualityControl* piqc)
{
	// TODO
	return S_OK;
}


//
// IKsPropertySet
//


HRESULT STDMETHODCALLTYPE ALiveSourceVideoOutputPin::Set(
	REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData,
	LPVOID pPropData, DWORD cbPropData)
{
	// https://docs.microsoft.com/en-us/windows/win32/directshow/pin-requirements-for-capture-filters
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE ALiveSourceVideoOutputPin::Get(
	REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData,
	DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData,
	DWORD* pcbReturned)
{
	// https://docs.microsoft.com/en-us/windows/win32/directshow/pin-requirements-for-capture-filters

	if (guidPropSet != AMPROPSETID_Pin)
		return E_PROP_SET_UNSUPPORTED;

	if (dwPropID != AMPROPERTY_PIN_CATEGORY)
		return E_PROP_ID_UNSUPPORTED;

	if (pPropData == NULL && pcbReturned == NULL)
		return E_POINTER;

	if (pcbReturned)
		*pcbReturned = sizeof(GUID);

	if (pPropData == NULL)  // Caller just wants to know the size.
		return S_OK;

	if (cbPropData < sizeof(GUID))  // The buffer is too small.
		return E_UNEXPECTED;

	*(GUID*)pPropData = PIN_CATEGORY_CAPTURE;
	return S_OK;
}


HRESULT STDMETHODCALLTYPE ALiveSourceVideoOutputPin::QuerySupported(
	REFGUID guidPropSet, DWORD dwPropID, DWORD* pTypeSupport)
{
	// https://docs.microsoft.com/en-us/windows/win32/directshow/pin-requirements-for-capture-filters

	if (guidPropSet != AMPROPSETID_Pin)
		return E_PROP_SET_UNSUPPORTED;

	if (dwPropID != AMPROPERTY_PIN_CATEGORY)
		return E_PROP_ID_UNSUPPORTED;

	// We support getting this property, but not setting it.
	if (pTypeSupport)
		*pTypeSupport = KSPROPERTY_SUPPORT_GET;

	return S_OK;
}


void ALiveSourceVideoOutputPin::OnHDRData(HDRDataSharedPtr& hdrData)
{
	if (!hdrData)
		throw std::runtime_error("Setting HDRData to null is not allowed");

	m_hdrData = hdrData;
	m_hdrChanged = true;
}


HRESULT ALiveSourceVideoOutputPin::RenderVideoFrameIntoSample(VideoFrame& videoFrame, IMediaSample* const pSample)
{
	HRESULT hr;

	//
	// Media time and related checked
	// (= frame counter)
	//

	//const LONGLONG frameCounter = videoFrame.GetCounter();  // From capture (will jump forward on drops)
	const LONGLONG frameCounter = m_frameCounter++;  // Internal (guaranteed ++)

	// Set frame counter
	LONGLONG mediaTimeStart = frameCounter;
	LONGLONG mediaTimeStop = mediaTimeStart + 1;
	hr = pSample->SetMediaTime(&mediaTimeStart, &mediaTimeStop);
	if (FAILED(hr))
		return hr;

	// Discontinuity check
	const bool isDiscontinuity =
		frameCounter != (m_previousFrameCounter + 1) ||
		frameCounter == 0;
	if (isDiscontinuity)
	{
		DbgLog((LOG_TRACE, 1, TEXT("::FillBuffer(): Frame counter jumped from %I64u to %I64u, discontinuity detected"),
			m_previousFrameCounter, frameCounter));

		hr = pSample->SetDiscontinuity(TRUE);
		if (FAILED(hr))
			return hr;
	}

	m_previousFrameCounter = frameCounter;

	//
	// Setting the time
	//

	switch (m_timestamp)
	{
	case RendererTimestamp::RENDERER_TIMESTAMP_NONE:
		break;

	case RendererTimestamp::RENDERER_TIMESTAMP_THEORETICAL:
	{
		assert(m_startTimeOffset == 0);
		REFERENCE_TIME timeStart = (frameCounter * m_frameDuration);
		REFERENCE_TIME timeStop = timeStart + m_frameDuration;

		hr = pSample->SetTime(&timeStart, &timeStop);
		if (FAILED(hr))
			return hr;

		break;
	}

	case RendererTimestamp::RENDERER_TIMESTAMP_CLOCK:
	{
		if (videoFrame.GetTimingTimestamp() > 0)
		{
			assert(m_timingClock->GetTimingClockTicksPerSecond() > 0);

			// Get frame timestamp as reference time
			// TODO: I don't like the floating point math here
			REFERENCE_TIME timeStart =
				videoFrame.GetTimingTimestamp() *
				(10000000.0 / m_timingClock->GetTimingClockTicksPerSecond());

			// Guarantee first frame to start counting at time zero
			// Note that this is against the recommendations of microsoft for directshow but otherwise
			// madVR doesn't start rendering as it's designed for file based video which starts at 0
			if (m_startTimeOffset == 0)
			{
				m_startTimeOffset = timeStart;
			}

			// If this is not the first frame, add the m_frameClockOffsetMs as reference time (=100ns)
			else
			{
				timeStart += (REFERENCE_TIME)m_frameClockOffsetMs * 10000LL;
			}

			timeStart -= m_startTimeOffset;

			assert(m_frameDuration > 0);
			REFERENCE_TIME timeStop = timeStart + m_frameDuration;

			hr = pSample->SetTime(&timeStart, &timeStop);
			if (FAILED(hr))
				return hr;

#ifdef _DEBUG
			// Every n frames output a bunch of consecutive frames to check start/stop
			if (frameCounter % 200 < 5)
			{
				DbgLog((LOG_TRACE, 1, TEXT("::FillBuffer(#%I64u): , Start: %I64d Stop: %I64d"),
					m_frameCounter, timeStart, timeStop));
			}

			// Check how far we're off
			if (frameCounter > 0 &&
				frameCounter % 100 == 0)
			{
				assert(m_previousTimeStop < timeStop);

				double previousStopStartDiffMs = (m_previousTimeStop - timeStart) / (double)m_timingClock->GetTimingClockTicksPerSecond() * 1000;

				DbgLog((LOG_TRACE, 1, TEXT("::FillBuffer(#%I64u): Diff previous stop to start: %.3f ms"),
					frameCounter, previousStopStartDiffMs));

				//assert(abs(m_previousTimeStop - timeStart) < 10000);  // Must be under a ms
			}

			m_previousTimeStop = timeStop;
#endif // _DEBUG
		}
		break;
	}
	default:
		assert(false);
	}

	//
	// Data copy/formatting
	//

	// Get target data buffer
	BYTE* pData = NULL;
	hr = pSample->GetPointer(&pData);
	if (FAILED(hr))
		return hr;

	assert(pData);

	// Format (which can just be a copy or a full decode) the video frame to the
	// DirectShow buffer
	// A simple memcpy runs in the 2-4ms range for a decent frame size
#ifdef _DEBUG
	timestamp_t startTime = ::GetWallClockTime();
#endif

	m_videoFrameFormatter->FormatVideoFrame(videoFrame, pData);

#ifdef _DEBUG
	if (videoFrame.GetCounter() % 100 == 0)
	{
		DbgLog((LOG_TRACE, 1,
			TEXT("::FillBuffer(#%I64u): Formatter took %.1f us"),
			videoFrame.GetCounter(),
			((::GetWallClockTime() - startTime) / 10.0)));
	}
#endif

	hr = pSample->SetActualDataLength(m_videoFrameFormatter->GetOutFrameSize());
	if (FAILED(hr))
		return hr;

	//
	// Sync
	//

	// All frames are complete images and hence sync points by definition
	hr = pSample->SetSyncPoint(TRUE);
	if (FAILED(hr))
		return hr;

	//
	// HDR metadata
	//

	// TODO: This is now called from a different thread, warning!
	if (m_hdrData)
	{
		if ((videoFrame.GetCounter() % 100) == 1 || m_hdrChanged)
		{
			IMediaSideData* pMediaSideData = nullptr;
			if (FAILED(pSample->QueryInterface(&pMediaSideData)))
				throw std::runtime_error("Failed to get IMediaSideData");

			MediaSideDataHDRContentLightLevel hdrLightLevel;
			ZeroMemory(&hdrLightLevel, sizeof(hdrLightLevel));
			hdrLightLevel.MaxCLL = (unsigned int)round(m_hdrData->maxCll);
			hdrLightLevel.MaxFALL = (unsigned int)round(m_hdrData->maxFall);
			pMediaSideData->SetSideData(IID_MediaSideDataHDRContentLightLevel, (const BYTE*)&hdrLightLevel, sizeof(hdrLightLevel));

			MediaSideDataHDR hdr;
			ZeroMemory(&hdr, sizeof(hdr));
			hdr.display_primaries_x[0] = m_hdrData->displayPrimaryGreenX;
			hdr.display_primaries_x[1] = m_hdrData->displayPrimaryBlueX;
			hdr.display_primaries_x[2] = m_hdrData->displayPrimaryRedX;
			hdr.display_primaries_y[0] = m_hdrData->displayPrimaryGreenY;
			hdr.display_primaries_y[1] = m_hdrData->displayPrimaryBlueY;
			hdr.display_primaries_y[2] = m_hdrData->displayPrimaryRedY;
			hdr.white_point_x = m_hdrData->whitePointX;
			hdr.white_point_y = m_hdrData->whitePointY;
			hdr.max_display_mastering_luminance = m_hdrData->masteringDisplayMaxLuminance;
			hdr.min_display_mastering_luminance = m_hdrData->masteringDisplayMinLuminance;
			pMediaSideData->SetSideData(IID_MediaSideDataHDR, (const BYTE*)&hdr, sizeof(hdr));

			pMediaSideData->Release();

			m_hdrChanged = false;
		}
	}

	//
	// The very last thing we need to do (as close to the renderer as possible) is to
	// calculate how many ms the last frame start is ahead of the clock. This is called
	// Video lead time.
	// - postive means frame to be rendered in the future, which is what we need
	// - negative means the frame is late, will be rendered immediately
	//

	if (m_timestamp == RendererTimestamp::RENDERER_TIMESTAMP_CLOCK)
	{
		REFERENCE_TIME timeStart, timeStop;

		hr = pSample->GetTime(&timeStart, &timeStop);
		if (FAILED(hr))
			throw std::runtime_error("Failed to get start time");

		double timeStartMs = (timeStart + m_startTimeOffset) / 10000.0;
		double clockMs = m_timingClock->GetTimingClockTime() / (double)m_timingClock->GetTimingClockTicksPerSecond() * 1000.0;

		m_lastFrameVideoLeadMs = timeStartMs - clockMs;
	}

	return hr;
}

