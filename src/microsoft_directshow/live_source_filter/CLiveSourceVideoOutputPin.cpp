/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "CLiveSourceVideoOutputPin.h"

#include <guid.h>
#include <IMediaSideData.h>
#include <microsoft_directshow/DirectShowTranslations.h>


#define TARGET_AV_PIX_FMT AV_PIX_FMT_RGB48LE

CLiveSourceVideoOutputPin::CLiveSourceVideoOutputPin(
	CLiveSource* filter,
	CCritSec* pLock,
	HRESULT* phr) :
	CBaseOutputPin(
		LIVE_SOURCE_FILTER_NAME, filter, pLock, phr,
		LIVE_SOURCE_FILTER_VIDEO_OUPUT_PIN_NAME)
{
}


CLiveSourceVideoOutputPin::~CLiveSourceVideoOutputPin()
{
}


STDMETHODIMP CLiveSourceVideoOutputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IAMPushSource)
	{
		return GetInterface((IAMPushSource*)this, ppv);
	}
	else if (riid == IID_IQualityControl)
	{
		return GetInterface((IQualityControl*)this, ppv);
	}
	else if (riid == IID_IKsPropertySet)
	{
		return GetInterface((IKsPropertySet*)this, ppv);
	}

	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
}


HRESULT CLiveSourceVideoOutputPin::GetMediaType(int iPosition, CMediaType* pmt)
{
	throw std::runtime_error("GetMediaType() is not supported");
}


HRESULT CLiveSourceVideoOutputPin::CheckMediaType(const CMediaType* pmt)
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


HRESULT CLiveSourceVideoOutputPin::DecideAllocator(IMemInputPin* pPin, IMemAllocator** ppAlloc)
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


HRESULT CLiveSourceVideoOutputPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *ppropInputRequest)
{
	CheckPointer(pAlloc,E_POINTER);
	CheckPointer(ppropInputRequest,E_POINTER);

	CAutoLock cAutoLock(m_pLock);
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


STDMETHODIMP CLiveSourceVideoOutputPin::GetMaxStreamOffset(REFERENCE_TIME *prtMaxOffset)
{
	*prtMaxOffset = 0;
	return S_OK;
}


STDMETHODIMP CLiveSourceVideoOutputPin::GetPushSourceFlags(ULONG *pFlags)
{
	// Return (* pFlags) = 0 [if this is a iAMPushSource]
	// https://docs.microsoft.com/en-us/windows/win32/api/strmif/nn-strmif-ireferenceclock
	*pFlags = 0;
	return S_OK;
}


STDMETHODIMP CLiveSourceVideoOutputPin::GetStreamOffset(REFERENCE_TIME *prtOffset)
{
	*prtOffset = 0;
	return S_OK;
}


STDMETHODIMP CLiveSourceVideoOutputPin::SetMaxStreamOffset(REFERENCE_TIME rtMaxOffset)
{
	return E_NOTIMPL;
}


STDMETHODIMP CLiveSourceVideoOutputPin::SetPushSourceFlags(ULONG Flags)
{
	return E_NOTIMPL;
}


STDMETHODIMP CLiveSourceVideoOutputPin::SetStreamOffset(REFERENCE_TIME rtOffset)
{
	return E_NOTIMPL;
}


STDMETHODIMP CLiveSourceVideoOutputPin::GetLatency(REFERENCE_TIME *prtLatency)
{
	// latency in 100-nanosecond units.
	*prtLatency = 0;  // TODO: Just a guess
	return S_OK;
}


//
// IQualityControl
//


STDMETHODIMP CLiveSourceVideoOutputPin::Notify(IBaseFilter* pSender, Quality q)
{
	// TODO
	return S_OK;
}


HRESULT STDMETHODCALLTYPE CLiveSourceVideoOutputPin::SetSink(IQualityControl* piqc)
{
	// TODO
	return S_OK;
}


//
// IKsPropertySet
//


HRESULT STDMETHODCALLTYPE CLiveSourceVideoOutputPin::Set(
	REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData,
	LPVOID pPropData, DWORD cbPropData)
{
	// https://docs.microsoft.com/en-us/windows/win32/directshow/pin-requirements-for-capture-filters
	return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CLiveSourceVideoOutputPin::Get(
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


HRESULT STDMETHODCALLTYPE CLiveSourceVideoOutputPin::QuerySupported(
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


void CLiveSourceVideoOutputPin::SetFormatter(IVideoFrameFormatter* videoFrameFormatter)
{
	if (!videoFrameFormatter)
		throw std::runtime_error("Cannot set null IVideoFrameFormatter");

	m_videoFrameFormatter = videoFrameFormatter;
}


void CLiveSourceVideoOutputPin::SetFrameDuration(timestamp_t duration)
{
	if(duration <= 0)
		throw std::runtime_error("Duration must be > 0");

	assert(duration > 50000LL); // 5ms frame is 200Hz, probably a reasonable upper bound
	assert(duration < 10000000LL);  // 1Hz, reasonable lower bound

	m_frameDuration = duration;
}


void CLiveSourceVideoOutputPin::SetTimestampTicksPerSecond(timingclocktime_t timestampTicksPerSecond)
{
	if (timestampTicksPerSecond < 0)
		throw std::runtime_error("Ticks per second must be >= 0");

	m_timestampTicksPerSecond = timestampTicksPerSecond;
}


void CLiveSourceVideoOutputPin::OnHDRData(HDRDataSharedPtr& hdrData)
{
	if (!hdrData)
		throw std::runtime_error("Setting HDRData to null is not allowed");

	m_hdrData = hdrData;
	m_hdrChanged = true;
}


HRESULT CLiveSourceVideoOutputPin::OnVideoFrame(VideoFrame& videoFrame)
{
	BYTE* pData = NULL;
	HRESULT hr;

	// Get buffer for sample
	// TODO: the 2 null pointers here point to the start and end of the sample, use that in cooperation with the captured time?
	IMediaSample* pSample = NULL;
	hr = this->GetDeliveryBuffer(&pSample, NULL, NULL, 0);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = pSample->GetPointer(&pData);
	if (FAILED(hr))
	{
		pSample->Release();
		return hr;
	}

	// Set time if we have a timestamp
	if (videoFrame.GetTimingTimestamp() > 0)
	{
		assert(m_timestampTicksPerSecond > 0);

		REFERENCE_TIME timeStart = videoFrame.GetTimingTimestamp();

		// TODO: I don't like the floating point math here
		timeStart = timeStart * (10000000.0 / m_timestampTicksPerSecond);

		// Guarantee first frame to start counting at time zero
		if (m_startTimeOffset == 0)
		{
			m_startTimeOffset = timeStart;
		}
		timeStart -= m_startTimeOffset;

		assert(m_frameDuration > 0);
		REFERENCE_TIME timeStop = timeStart + m_frameDuration;

		hr = pSample->SetTime(&timeStart, &timeStop);
		if (FAILED(hr))
		{
			pSample->Release();
			return hr;
		}

#ifdef _DEBUG
		if (m_frameCounter < 10)
		{
			DbgLog((LOG_TRACE, 1, TEXT("::OnVideoFrame(): #%I64u, Start: %I64d Stop: %I64d"),
				m_frameCounter, timeStart, timeStop));
		}

		// In debug keep track of expected timestamps and ensure they're not too far off
		// allow for a whole bunch of startup frames
		if (m_frameCounter > 0)
		{
			assert(m_previousTimeStop < timeStop);
			//assert(abs(m_previousTimeStop - timeStart) < 10000);  // Must be under a ms
		}
		m_previousTimeStop = timeStop;
#endif
	}

	// Media time is frame counter
	LONGLONG mediaTimeStart = m_frameCounter;
	LONGLONG mediaTimeStop = mediaTimeStart + 1;
	hr = pSample->SetMediaTime(&mediaTimeStart, &mediaTimeStop);
	if (FAILED(hr))
	{
		pSample->Release();
		return hr;
	}

	// Format (which can just be a copy) the video frame to the DirectShow buffer
	assert(m_videoFrameFormatter);
	m_videoFrameFormatter->FormatVideoFrame(videoFrame, pData);

	hr = pSample->SetActualDataLength(m_videoFrameFormatter->GetOutFrameSize());
	if (FAILED(hr))
	{
		pSample->Release();
		return hr;
	}

	// First frame is known discontinuity
	if (m_frameCounter == 0)
	{
		hr = pSample->SetDiscontinuity(TRUE);
		if (FAILED(hr))
		{
			pSample->Release();
			return hr;
		}
	}

	// All frames are complete images and hence sync points by definition
	hr = pSample->SetSyncPoint(TRUE);
	if (FAILED(hr))
	{
		pSample->Release();
		return hr;
	}

	// Set HDR data if needed
	if (m_hdrData)
	{
		if ((m_frameCounter % 100) == 0 || m_hdrChanged)
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

	hr = this->Deliver(pSample);
	pSample->Release();

	++m_frameCounter;

	return hr;
}
