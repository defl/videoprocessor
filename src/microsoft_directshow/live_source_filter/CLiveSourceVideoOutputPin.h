/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <streams.h>

#include <IVideoFrameFormatter.h>

#include "CLiveSource.h"


class CLiveSourceVideoOutputPin:
	public CBaseOutputPin,
	public IAMPushSource
{
public:

	DECLARE_IUNKNOWN;

	CLiveSourceVideoOutputPin(CLiveSource* filter, CCritSec *pLock, HRESULT *phr);
	virtual ~CLiveSourceVideoOutputPin();

	// CBaseOutputPin overrides
	HRESULT GetMediaType(int iPosition, CMediaType* pmt) override;
	HRESULT CheckMediaType(const CMediaType *pmt) override;
	HRESULT DecideAllocator(IMemInputPin* pPin, IMemAllocator** pAlloc) override;
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *ppropInputRequest) override;

	// IAMPushSource members
	virtual STDMETHODIMP GetMaxStreamOffset(REFERENCE_TIME* prtMaxOffset) override;
	virtual STDMETHODIMP GetPushSourceFlags(ULONG* pFlags) override;
	virtual STDMETHODIMP GetStreamOffset(REFERENCE_TIME* prtOffset) override;
	virtual STDMETHODIMP SetMaxStreamOffset(REFERENCE_TIME rtMaxOffset) override;
	virtual STDMETHODIMP SetPushSourceFlags(ULONG Flags) override;
	virtual STDMETHODIMP SetStreamOffset(REFERENCE_TIME rtOffset) override;
	virtual STDMETHODIMP GetLatency(REFERENCE_TIME* prtLatency) override;

	// IQualityControl
	virtual STDMETHODIMP Notify(IBaseFilter * pSender, Quality q) override;

	void SetFormatter(IVideoFrameFormatter* videoFrameFormatter);

	// Part of ILiveSource interface replicated here
	void OnHDRData(HDRDataSharedPtr&);
	HRESULT OnVideoFrame(VideoFrame&);

private:

	IVideoFrameFormatter* m_videoFrameFormatter = nullptr;

	uint32_t m_frameCounter = 0;

	HDRDataSharedPtr m_hdrData = nullptr;
	bool m_hdrChanged = false;
};
