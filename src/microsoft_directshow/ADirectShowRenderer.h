/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <dshow.h>
#include <dxva.h>

#include <IRenderer.h>
#include <PixelValueRange.h>
#include <VideoState.h>
#include <IVideoFrameFormatter.h>
#include <ITimingClock.h>
#include <microsoft_directshow/live_source_filter/CLiveSource.h>
#include <microsoft_directshow/DirectShowTimingClock.h>


/**
 * Abstract DirectShow renderer
 */
class ADirectShowRenderer:
	public IRenderer
{
public:

	ADirectShowRenderer(
		GUID rendererCLSID,
		IRendererCallback& callback,
		HWND videoHwnd,
		HWND eventHwnd,
		UINT eventMsg,
		ITimingClock* timingClock,
		VideoStateComPtr& videoState,
		DirectShowStartStopTimeMethod timestamp,
		bool useFrameQueue,
		size_t frameQueueMaxSize,
		bool useHDRDdata);

	virtual ~ADirectShowRenderer();

	// IRenderer
	bool OnVideoState(VideoStateComPtr&) override;
	void OnVideoFrame(VideoFrame& videoFrame) override;
	HRESULT OnWindowsEvent(LONG_PTR param1, LONG_PTR param2) override;
	void Build() override;
	void Start() override;
	void Stop() override;
	void Reset() override;
	void OnSize() override;
	void SetFrameQueueMaxSize(size_t) override;
	size_t GetFrameQueueSize() override;
	double EntryLatencyMs() const override;
	double ExitLatencyMs() const override;
	uint64_t DroppedFrameCount() const override;

protected:

	virtual void MediaTypeGenerate() = 0;
	virtual void Connect() = 0;

	const GUID m_rendererCLSID;
	IRendererCallback& m_callback;
	HWND m_videoHwnd;
	HWND m_eventHwnd;
	UINT m_eventMsg;
	ITimingClock* m_timingClock;
	VideoStateComPtr m_videoState;
	DirectShowStartStopTimeMethod m_timestamp;
	bool m_useFrameQueue;
	size_t m_frameQueueMaxSize;
	bool m_useHDRDdata;
	DXVA_NominalRange m_forceNominalRange;
	DXVA_VideoTransferFunction m_forceVideoTransferFunction;
	DXVA_VideoTransferMatrix m_forceVideoTransferMatrix;
	DXVA_VideoPrimaries m_forceVideoPrimaries;

	LONG m_renderBoxWidth = 0;
	LONG m_renderBoxHeight = 0;

	RendererState m_state = RendererState::RENDERSTATE_UNKNOWN;

	IGraphBuilder* m_pGraph = nullptr;
	IMediaControl* m_pControl = nullptr;
	IMediaEventEx* m_pEvent = nullptr;
	IVideoWindow* m_videoWindow = nullptr;
	IFilterGraph2* m_pGraph2 = nullptr;
	IMediaFilter* m_mediaFilter = nullptr;
	IAMGraphStreams* m_amGraphStreams = nullptr;
	IReferenceClock* m_referenceClock = nullptr;
	IVideoFrameFormatter* m_videoFramFormatter = nullptr;
	AM_MEDIA_TYPE m_pmt;
	CLiveSource* m_liveSource = nullptr;
	IBaseFilter* m_pLav = nullptr;
	IBaseFilter* m_pRenderer = nullptr;

	uint64_t m_frameCounter = 0;
	uint64_t m_missingFrameCounter = 0;
	double m_frameLatencyEntry = 0.0;

	// Handle Directshow graph events
	void OnGraphEvent(long evCode, LONG_PTR param1, LONG_PTR param2);

	// Helper for state setting and callbacks
	void SetState(RendererState state);

	// Graph management functions.
	void GraphBuild();
	void GraphTeardown();
	void GraphRun();
	void GraphStop();
};
