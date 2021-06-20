/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "CLiveSourceVideoOutputPin.h"

#include "CLiveSource.h"


CLiveSource::CLiveSource(
	LPUNKNOWN pUnk,
	HRESULT* phr):
	CBaseFilter(LIVE_SOURCE_FILTER_NAME, pUnk, &m_critSec, CLSID_CLiveSource)
{
	m_videoOutputPin = new CLiveSourceVideoOutputPin(
		this,
		&m_critSec,
		phr);

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
		//TODO: Because we cannot properly release everything we manually delete here
		//m_videoOutputPin->Release();
		delete m_videoOutputPin;
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
	timingclocktime_t timestampTicksPerSecond)
{
	assert(videoFrameFormatter);
	assert(mediaSubType.Data1 > 0);
	assert(frameDuration > 0);

	m_videoOutputPin->Initialize(
		videoFrameFormatter,
		frameDuration,
		timestampTicksPerSecond);

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
	{
		return GetInterface((ILiveSource*)this, ppv);
	}
	else if (riid == IID_IAMPushSource)
	{
		// This should be on the pin itself
		throw std::runtime_error("NonDelegatingQueryInterface IID_IAMPushSource called on CLiveSource");
	}
	else if (riid == IID_IQualityControl)
	{
		// This should be on the pin itself
		throw std::runtime_error("NonDelegatingQueryInterface IID_IQualityControl called on CLiveSource");
	}
	else if (riid == IID_IKsPropertySet)
	{
		// This should be on the pin itself
		throw std::runtime_error("NonDelegatingQueryInterface IID_IKsPropertySet called on CLiveSource");
	}
	else if (riid == IID_IAMFilterMiscFlags)
	{
		return GetInterface((IAMFilterMiscFlags*)this, ppv);
	}
	else
	{
		return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
	}
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
