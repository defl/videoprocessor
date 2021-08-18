/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <pch.h>

#include "CBufferedLiveSourceVideoOutputPin.h"
#include "CUnbufferedLiveSourceVideoOutputPin.h"


#include "CLiveSource.h"


CLiveSource::CLiveSource(
	LPUNKNOWN pUnk,
	HRESULT* phr):
	CBaseFilter(LIVE_SOURCE_FILTER_NAME, pUnk, &m_critSec, CLSID_CLiveSource)
{
}


CLiveSource::~CLiveSource()
{
	assert(!m_videoOutputPin);  // Didn't call Destroy()
}


CUnknown* WINAPI CLiveSource::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
	CUnknown* liveSource = new CLiveSource(pUnk, phr);

	if (phr)
	{
		if (!liveSource)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}

	return liveSource;
}


STDMETHODIMP CLiveSource::Initialize(
	IVideoFrameFormatter* videoFrameFormatter,
	const AM_MEDIA_TYPE& mediaType,
	timestamp_t frameDuration,
	ITimingClock* timingClock,
	DirectShowStartStopTimeMethod timestamp,
	bool useFrameQueue,
	size_t frameQueueMaxSize)
{
	assert(!m_videoOutputPin);
	assert(videoFrameFormatter);
	assert(mediaType.majortype.Data1 > 0);
	assert(frameDuration > 0);

	HRESULT hr = S_OK;

	if (useFrameQueue)
	{
		m_videoOutputPin = new CBufferedLiveSourceVideoOutputPin(
			this,
			&m_critSec,
			&hr);
	}
	else
	{
		m_videoOutputPin = new CUnbufferedLiveSourceVideoOutputPin(
			this,
			&m_critSec,
			&hr);
	}

	if (!m_videoOutputPin || hr != S_OK)
		throw std::runtime_error("Failed to construct pin");

	if (m_videoOutputPin)
		m_videoOutputPin->AddRef();

	m_videoOutputPin->Initialize(
		videoFrameFormatter,
		frameDuration,
		timingClock,
		timestamp,
		mediaType);

	if (useFrameQueue)
		m_videoOutputPin->SetFrameQueueMaxSize(frameQueueMaxSize);

	return S_OK;
}


STDMETHODIMP CLiveSource::Destroy()
{
	if (m_videoOutputPin)
	{
		ULONG refCount = m_videoOutputPin->Release();
		delete m_videoOutputPin;  // Pin's release() does not delete at last one
		m_videoOutputPin = nullptr;
	}

	return S_OK;
}


STDMETHODIMP CLiveSource::OnHDRData(HDRDataSharedPtr& hdrData)
{
	m_videoOutputPin->OnHDRData(hdrData);
	return S_OK;
}


STDMETHODIMP CLiveSource::OnVideoFrame(VideoFrame& videoFrame)
{
	return m_videoOutputPin->OnVideoFrame(videoFrame);
}


STDMETHODIMP CLiveSource::SetFrameQueueMaxSize(size_t frameQueueMaxSize)
{
	if (frameQueueMaxSize < 0)
		throw std::runtime_error("Queue must be >= 0");

	m_videoOutputPin->SetFrameQueueMaxSize(frameQueueMaxSize);
	return S_OK;
}


STDMETHODIMP CLiveSource::Reset()
{
	m_videoOutputPin->Reset();
	return S_OK;
}


STDMETHODIMP CLiveSource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	if (riid == IID_ILiveSource)
		return GetInterface((ILiveSource*)this, ppv);

	else if (riid == IID_IAMFilterMiscFlags)
		return GetInterface((IAMFilterMiscFlags*)this, ppv);

	else
		return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}


int CLiveSource::GetPinCount()
{
	return 1;
}


CBasePin* CLiveSource::GetPin(int n)
{
	if (n != 0)
		throw std::runtime_error("CLiveSource only has 1 pin");

	return m_videoOutputPin;
}


ULONG CLiveSource::GetMiscFlags()
{
	return AM_FILTER_MISC_FLAGS_IS_SOURCE;
}


int CLiveSource::GetFrameQueueSize()
{
	return (int)m_videoOutputPin->GetFrameQueueSize();
}


double CLiveSource::ExitLatencyMs() const
{
	return m_videoOutputPin->ExitLatencyMs();
}


uint64_t CLiveSource::DroppedFrameCount() const
{
	return m_videoOutputPin->DroppedFrameCount();
}
