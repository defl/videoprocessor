/*
 * Copyright(C) 2021 Dennis Fleurbaaij <mail@dennisfleurbaaij.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see < https://www.gnu.org/licenses/>.
 */

#pragma once


#include <dshow.h>

#include <IRenderer.h>
#include <PixelValueRange.h>
#include <VideoState.h>
#include <IVideoFrameFormatter.h>
#include <ITimingClock.h>
#include <microsoft_directshow/live_source_filter/CLiveSource.h>
#include <microsoft_directshow/DirectShowTimingClock.h>


/**
 * This renderer renders using madVR.
 * For that to work it has to create a DirectShow graph.
 */
class DirectShowMadVRRenderer:
	public IRenderer
{
public:

	// For the force_* params, the _unknown define means don't force but auto-select
	DirectShowMadVRRenderer(
		IRendererCallback& callback,
		HWND videoHwnd,
		HWND eventHwnd,
		UINT eventMsg,
		ITimingClock* timingClock,
		VideoStateComPtr& videoState,
		RendererTimestamp timestamp,
		size_t frameQueueMaxSize,
		int frameClockOffsetMs,
		DXVA_NominalRange forceNominalRange,
		DXVA_VideoTransferFunction forceVideoTransferFunction,
		DXVA_VideoTransferMatrix forceVideoTransferMatrix,
		DXVA_VideoPrimaries forceVideoPrimaries);

	virtual ~DirectShowMadVRRenderer();

	// IRenderer
	bool OnVideoState(VideoStateComPtr&) override;
	void OnVideoFrame(VideoFrame& videoFrame) override;
	HRESULT OnWindowsEvent(LONG_PTR param1, LONG_PTR param2) override;
	void Build() override;
	void Start() override;
	void Stop() override;
	void OnPaint() override;
	void OnSize() override;
	int GetFrameQueueSize() override;
	double GetFrameVideoLeadMs() override;

private:
	IRendererCallback& m_callback;
	HWND m_videoHwnd;
	HWND m_eventHwnd;
	UINT m_eventMsg;
	ITimingClock* m_timingClock;
	VideoStateComPtr m_videoState;
	RendererTimestamp m_timestamp;
	size_t m_frameQueueMaxSize;
	int m_frameClockOffsetMs;
	DXVA_NominalRange m_forceNominalRange = DXVA_NominalRange_Unknown;  // Unknown means not force
	DXVA_VideoTransferFunction m_forceVideoTransferFunction = DXVA_VideoTransFunc_Unknown;  // Unknown means not force
	DXVA_VideoTransferMatrix m_forceVideoTransferMatrix = DXVA_VideoTransferMatrix_Unknown;  // Unknown means not force
	DXVA_VideoPrimaries m_forceVideoPrimaries = DXVA_VideoPrimaries_Unknown;  // Unknown means not force

	LONG m_renderBoxWidth = 0;
	LONG m_renderBoxHeight = 0;

	RendererState m_state = RendererState::RENDERSTATE_UNKNOWN;

	IGraphBuilder* m_pGraph = NULL;
	IMediaControl* m_pControl = NULL;
	IMediaEventEx* m_pEvent = NULL;
	IVideoWindow* m_videoWindow = NULL;
	IFilterGraph2* m_pGraph2 = NULL;
	IMediaFilter* m_mediaFilter = NULL;
	IAMGraphStreams* m_amGraphStreams = NULL;
	IReferenceClock* m_referenceClock = nullptr;
	IVideoFrameFormatter* m_videoFramFormatter = nullptr;
	CLiveSource* m_liveSource = nullptr;
	IBaseFilter* m_pMVR = nullptr;

#ifdef _DEBUG
	timingclocktime_t m_previousFrameTime = 0;
#endif

	uint64_t m_frameCounter = 0;

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
