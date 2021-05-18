/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "CLiveSourceVideoOutputPin.h"

#include <IMediaSideData.h>
#include <microsoft_directshow/DirectShowTranslations.h>


CLiveSourceVideoOutputPin::CLiveSourceVideoOutputPin(
	CBaseFilter* filter,
	CCritSec *pLock,
	HRESULT *phr):
	CBaseOutputPin(LIVE_SOURCE_FILTER_NAME, filter, pLock, phr, LIVE_SOURCE_FILTER_VIDEO_OUPUT_PIN_NAME)
{
}


CLiveSourceVideoOutputPin::~CLiveSourceVideoOutputPin()
{
}


HRESULT CLiveSourceVideoOutputPin::CheckMediaType(const CMediaType* pmt)
{
	CheckPointer(pmt, E_POINTER);

	if(pmt->majortype != MEDIATYPE_Video)
	{
		return E_INVALIDARG;
	}

	if (pmt->subtype != m_mediaSubType)
	{
		return E_INVALIDARG;
	}

	if (pmt->formattype != FORMAT_VideoInfo2)
	{
		return E_INVALIDARG;
	}

	return S_OK;
}


HRESULT CLiveSourceVideoOutputPin::GetMediaType(int iPosition, CMediaType* pmt)
{
	throw std::runtime_error("GetMediaType() is not supported");
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

	if (!m_bytesPerFrame)
		throw std::runtime_error("No known video state, call OnVideoState() before starting");

	CAutoLock cAutoLock(m_pLock);
	HRESULT hr = NOERROR;

	ppropInputRequest->cBuffers = 1;
	ppropInputRequest->cbBuffer = m_bytesPerFrame;

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


STDMETHODIMP CLiveSourceVideoOutputPin::GetMaxStreamOffset(REFERENCE_TIME *prtMaxOffset)
{
	*prtMaxOffset = 0;
	return S_OK;
}


STDMETHODIMP CLiveSourceVideoOutputPin::GetPushSourceFlags(ULONG *pFlags)
{
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
	// TODO: *prtLatency = m_rtFrameRate;
	return S_OK;
}


STDMETHODIMP CLiveSourceVideoOutputPin::Notify(IBaseFilter* pSender, Quality q)
{
	// TODO
	return S_OK;
}


void CLiveSourceVideoOutputPin::OnVideoState(VideoStateComPtr& videoState)
{
	if (!videoState)
		throw std::runtime_error("Null video state not allowed");

	if (m_bytesPerFrame)
		throw std::runtime_error("Setting a new video state is not allowed");

	m_mediaSubType = TranslateToMediaSubType(videoState->pixelFormat);
	m_bytesPerFrame = videoState->BytesPerFrame();
	OnHDRData(videoState->hdrData);
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

	memcpy(pData, videoFrame.GetData(), m_bytesPerFrame);

	hr = pSample->SetActualDataLength(m_bytesPerFrame);
	if (FAILED(hr))
	{
		pSample->Release();
		return hr;
	}

	hr = pSample->SetSyncPoint(TRUE);
	if (FAILED(hr))
	{
		pSample->Release();
		return hr;
	}

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
