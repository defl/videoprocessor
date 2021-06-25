/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "CUnbufferedLiveSourceVideoOutputPin.h"
#include "CBufferedLiveSourceVideoOutputPin.h"

#include "CLiveSource.h"


CLiveSource::CLiveSource(
	LPUNKNOWN pUnk,
	HRESULT* phr):
	CBaseFilter(LIVE_SOURCE_FILTER_NAME, pUnk, &m_critSec, CLSID_CLiveSource)
{
	HRESULT hr = S_OK;

	//m_videoOutputPin = new CUnbufferedLiveSourceVideoOutputPin(
	//	this,
	//	&m_critSec,
	//	&hr);

	m_videoOutputPin = new CBufferedLiveSourceVideoOutputPin(
		this,
		&m_critSec,
		&hr);


	if (hr != S_OK)
		throw std::runtime_error("Failed to construct pin");

	if (phr)
	{
		if (m_videoOutputPin == nullptr)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}

	if(m_videoOutputPin)
		m_videoOutputPin->AddRef();
}


CLiveSource::~CLiveSource()
{
	if (m_videoOutputPin)
	{
		ULONG refCount = m_videoOutputPin->Release();
		assert(refCount == 0);
		m_videoOutputPin = nullptr;
	}
}


CUnknown* WINAPI CLiveSource::CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
	CUnknown* liveSource = new CLiveSource(pUnk, phr);

	if (phr)
	{
		if (liveSource == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}

	return liveSource;
}


STDMETHODIMP CLiveSource::Initialize(
	IVideoFrameFormatter* videoFrameFormatter,
	GUID mediaSubType,
	timestamp_t frameDuration,
	ITimingClock* timingClock,
	RendererTimestamp timestamp,
	size_t frameQueueMaxSize,
	int frameClockOffsetMs)
{
	assert(videoFrameFormatter);
	assert(mediaSubType.Data1 > 0);
	assert(frameDuration > 0);
	assert(-5000 < frameClockOffsetMs < 5000);

	m_videoOutputPin->Initialize(
		videoFrameFormatter,
		frameDuration,
		timingClock,
		timestamp,
		frameClockOffsetMs,
		m_mediaSubType);

	m_videoOutputPin->SetFrameQueueMaxSize(frameQueueMaxSize);

	m_mediaSubType = mediaSubType;
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


GUID CLiveSource::GetMediaSubType()
{
	return m_mediaSubType;
}


int CLiveSource::GetFrameQueueSize()
{
	return m_videoOutputPin->GetFrameQueueSize();
}


double CLiveSource::GetFrameVideoLeadMs()
{
	return m_videoOutputPin->GetFrameVideoLeadMs();
}
