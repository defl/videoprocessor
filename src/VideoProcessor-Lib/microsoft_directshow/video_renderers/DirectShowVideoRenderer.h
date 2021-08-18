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
#include <video_frame_formatter/IVideoFrameFormatter.h>
#include <ITimingClock.h>
#include <VideoConversionOverride.h>
#include <microsoft_directshow/live_source_filter/CLiveSource.h>
#include <microsoft_directshow/DirectShowTimingClock.h>


/**
 * Abstract DirectShow video renderer
 */
class DirectShowVideoRenderer:
	public IVideoRenderer
{
public:

	DirectShowVideoRenderer(
		IRendererCallback& callback,
		HWND videoHwnd,
		HWND eventHwnd,
		UINT eventMsg,
		ITimingClock* timingClock,
		DirectShowStartStopTimeMethod timestamp,
		bool useFrameQueue,
		size_t frameQueueMaxSize,
		VideoConversionOverride videoConversionOverride);
	virtual ~DirectShowVideoRenderer();

	// IVideoRenderer
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

	IRendererCallback& m_callback;
	HWND m_videoHwnd;
	HWND m_eventHwnd;
	UINT m_eventMsg;
	ITimingClock* m_timingClock;
	VideoStateComPtr m_videoState;
	DirectShowStartStopTimeMethod m_timestamp;
	bool m_useFrameQueue;
	size_t m_frameQueueMaxSize;
	VideoConversionOverride m_videoConversionOverride;
	DXVA_NominalRange m_forceNominalRange = DXVA_NominalRange::DXVA_NominalRange_Unknown;
	DXVA_VideoTransferFunction m_forceVideoTransferFunction = DXVA_VideoTransferFunction::DXVA_VideoTransFunc_Unknown;
	DXVA_VideoTransferMatrix m_forceVideoTransferMatrix = DXVA_VideoTransferMatrix::DXVA_VideoTransferMatrix_Unknown;
	DXVA_VideoPrimaries m_forceVideoPrimaries = DXVA_VideoPrimaries::DXVA_VideoPrimaries_Unknown;

	LONG m_renderBoxWidth = 0;
	LONG m_renderBoxHeight = 0;

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

	// This is the whole thing, with everything included
	virtual void GraphBuild();
	virtual void GraphTeardown();
	virtual void GraphRun();
	virtual void GraphStop();

	virtual void FilterGraphBuild();
	virtual void FilterGraphDestroy();

	// Window management
	virtual void WindowSetup();
	virtual void WindowTeardown();

	// Live source filter management
	virtual void LiveSourceBuildAndConnect();
	virtual void LiveSourceDisconnect();
	virtual void LiveSourceDestroy();

	// If called the implementation should instantiate a renderer
	// in m_pRenderer.
	// Most probably by calling CoCreateInstance(...).
	// A nullptr return will signal an error
	virtual void RendererBuild() = 0;

	// Add renderer to the graph and connect
	virtual void RendererConnect() = 0;
	virtual void RendererDestroy();

	virtual void MediaTypeGenerate() = 0;


private:

	// Use SetState()
	RendererState m_state = RendererState::RENDERSTATE_UNKNOWN;
};
