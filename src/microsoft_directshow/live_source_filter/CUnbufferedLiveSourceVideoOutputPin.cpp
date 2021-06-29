/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#include <stdafx.h>

#include "CUnbufferedLiveSourceVideoOutputPin.h"


CUnbufferedLiveSourceVideoOutputPin::CUnbufferedLiveSourceVideoOutputPin(
	CLiveSource* filter,
	CCritSec* pLock,
	HRESULT* phr):
	ALiveSourceVideoOutputPin(filter, pLock, phr)
{
}


CUnbufferedLiveSourceVideoOutputPin::~CUnbufferedLiveSourceVideoOutputPin()
{

}


HRESULT CUnbufferedLiveSourceVideoOutputPin::OnVideoFrame(VideoFrame& videoFrame)
{
	BYTE* pData = NULL;
	HRESULT hr;

	// Get buffer for sample
	// Note you can fill in start and stop time, but following the code shows that they are unused.
	IMediaSample* pSample = NULL;
	hr = this->GetDeliveryBuffer(&pSample, NULL, NULL, 0);
	if (FAILED(hr))
	{
		return hr;
	}

	// Render
	hr = RenderVideoFrameIntoSample(videoFrame, pSample);
	if (FAILED(hr))
	{
		pSample->Release();
		return hr;
	}

	// Deliver to downstream renderer (this will block)
	hr = this->Deliver(pSample);
	pSample->Release();

	return hr;
}


void CUnbufferedLiveSourceVideoOutputPin::SetFrameQueueMaxSize(size_t frameBufferMaxSize)
{
	if (frameBufferMaxSize != 0)
		throw std::runtime_error("CUnbufferedLiveSourceVideoOutputPin can only accept zero frame buffers ");
}
