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
	public IAMPushSource,
	public IKsPropertySet
{
public:

	CLiveSourceVideoOutputPin(CLiveSource* filter, CCritSec *pLock, HRESULT *phr);
	virtual ~CLiveSourceVideoOutputPin();

	// TODO Units of frameduration?
	void Initialize(
		IVideoFrameFormatter * videoFrameFormatter,
		timestamp_t frameDuration,
		timingclocktime_t timestampTicksPerSecond);

	// IUnknown
	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) override;

	// CBaseOutputPin overrides
	HRESULT GetMediaType(int iPosition, CMediaType* pmt) override;
	HRESULT CheckMediaType(const CMediaType *pmt) override;
	HRESULT DecideAllocator(IMemInputPin* pPin, IMemAllocator** pAlloc) override;
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *ppropInputRequest) override;

	// IAMPushSource
	STDMETHODIMP GetMaxStreamOffset(REFERENCE_TIME* prtMaxOffset) override;
	STDMETHODIMP GetPushSourceFlags(ULONG* pFlags) override;
	STDMETHODIMP GetStreamOffset(REFERENCE_TIME* prtOffset) override;
	STDMETHODIMP SetMaxStreamOffset(REFERENCE_TIME rtMaxOffset) override;
	STDMETHODIMP SetPushSourceFlags(ULONG Flags) override;
	STDMETHODIMP SetStreamOffset(REFERENCE_TIME rtOffset) override;
	STDMETHODIMP GetLatency(REFERENCE_TIME* prtLatency) override;

	// IQualityControl
	STDMETHODIMP Notify(IBaseFilter * pSender, Quality q) override;
	HRESULT STDMETHODCALLTYPE SetSink(IQualityControl* piqc) override;

	// IKsPropertySet
	HRESULT STDMETHODCALLTYPE Set(
		REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData,
		LPVOID pPropData, DWORD cbPropData) override;
	HRESULT STDMETHODCALLTYPE Get(
		REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData,
		DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData,
		DWORD* pcbReturned) override;
	HRESULT STDMETHODCALLTYPE QuerySupported(
		REFGUID guidPropSet, DWORD dwPropID, DWORD* pTypeSupport) override;

	// Part of ILiveSource interface replicated here
	void OnHDRData(HDRDataSharedPtr&);
	HRESULT OnVideoFrame(VideoFrame&);

private:

	IVideoFrameFormatter* m_videoFrameFormatter = nullptr;
	timestamp_t m_frameDuration = 0;
	timingclocktime_t m_timestampTicksPerSecond = 0;

	uint32_t m_frameCounter = 0;
#ifdef _DEBUG
	REFERENCE_TIME m_previousTimeStop = 0;
#endif
	timestamp_t m_startTimeOffset = 0;

	HDRDataSharedPtr m_hdrData = nullptr;
	bool m_hdrChanged = false;
};
