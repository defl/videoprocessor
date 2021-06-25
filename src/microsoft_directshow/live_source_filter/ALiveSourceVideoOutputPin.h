/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <IVideoFrameFormatter.h>
#include <IRenderer.h>    // TODO: Pull out timestamp?

#include "CLiveSource.h"


/**
 * Abstract implementation of the video pin
 */
class ALiveSourceVideoOutputPin:
	public CBaseOutputPin,
	public IAMPushSource,
	public IQualityControl,
	public IKsPropertySet
{
public:

	ALiveSourceVideoOutputPin(
		CLiveSource* filter,
		CCritSec* pLock,
		HRESULT* phr);
	virtual ~ALiveSourceVideoOutputPin();

	DECLARE_IUNKNOWN;

	void Initialize(
		IVideoFrameFormatter* const videoFrameFormatter,
		timestamp_t frameDuration,
		ITimingClock* const timingClock,
		RendererTimestamp timestamp,
		int frameClockOffsetMs,
		GUID mediaSubType);

	// CBaseOutputPin overrides
	HRESULT GetMediaType(int iPosition, CMediaType* pmt);
	HRESULT CheckMediaType(const CMediaType *pmt);
	HRESULT DecideAllocator(IMemInputPin* pPin, IMemAllocator** pAlloc);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *ppropInputRequest);

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
	virtual HRESULT OnVideoFrame(VideoFrame&) = 0;

	// Set the size of the queue.
	// Zero means no queueing, might not be legal
	virtual void SetFrameQueueMaxSize(size_t) = 0;

	// Get the size of the queue.
	// Zero means no queueing going on.
	virtual size_t GetFrameQueueSize() = 0;

	// Get the current "video lead" in milliseconds
	// Video lead is how many ms the last frame start is ahead of the clock.
	double GetFrameVideoLeadMs() const { return m_lastFrameVideoLeadMs; }

protected:

	// Render function to render a videoFrame onto a IMediaSample.
	// Will not release the sample or dec videoframe nor do the Deliver()
	HRESULT RenderVideoFrameIntoSample(VideoFrame&, IMediaSample* const);

private:

	// Constructor
	IVideoFrameFormatter* m_videoFrameFormatter;
	timestamp_t m_frameDuration;
	ITimingClock* m_timingClock;
	RendererTimestamp m_timestamp;
	int m_frameClockOffsetMs;
	GUID m_mediaSubType;

#ifdef _DEBUG
	REFERENCE_TIME m_previousTimeStop = 0;
#endif
	timestamp_t m_startTimeOffset = 0;
	uint64_t m_frameCounter = 0;
	uint64_t m_previousFrameCounter = 0;

	HDRDataSharedPtr m_hdrData = nullptr;
	bool m_hdrChanged = false;

	double m_lastFrameVideoLeadMs = 0.0;  // how many ms the last frame start is ahead of the clock.
};
